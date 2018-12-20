
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "sys_clock.hpp"
#include <WiFiUdp.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

enum state_machine {IDLE, WATERING};
short current_state = IDLE;

short water_hour = 0;
short water_minute = 14;
short water_second = 0;

short water_duration = 3;
bool just_watered = false;
long water_start = 0;

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

const char* wifi_ssid = "Ape CLT";
const char* wifi_passwd = "5enh@d0w1f1";

bool led = false;
long _old = millis();

int high = 500;
int low = 500;
bool status_led = true;

Sys_clock cookoo;




// NTPPPPPPPPPPPPPP
WiFiUDP UDP;                     // Create an instance of the WiFiUDP class to send and receive
unsigned int timeUNIX = 0;

IPAddress timeServerIP[5] = {IPAddress(200, 160, 7, 186), IPAddress(201, 49, 148, 135), IPAddress(200, 186, 125, 195), IPAddress(200, 20, 186, 76), IPAddress(200, 160, 0, 8)};

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets
// NTPPPPPPPPPPPPPP



void get_leds() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    jsonObj["status"] = led;
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}

void get_system_time(){
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[200];

    jsonObj["day"] = cookoo.get_day();
    jsonObj["hour"] = cookoo.get_hour();
    jsonObj["minute"] = cookoo.get_minute();
    jsonObj["second"] = cookoo.get_second();

    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}

void set_system_time(){

    StaticJsonBuffer<500> jsonBuffer;
    String post_body = http_rest_server.arg("plain");
    JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    short aux;

    if (http_rest_server.method() == HTTP_POST) {
        if (!jsonBody.success()) {
            http_rest_server.send(400);
        }else{
            if(jsonBody.containsKey("second")){
                aux = jsonBody["second"];
                cookoo.set_second(aux);
            }
            if(jsonBody.containsKey("minute")){
                aux = jsonBody["minute"];
                cookoo.set_minute(aux);
            }
            if(jsonBody.containsKey("hour")){
                aux = jsonBody["hour"];
                cookoo.set_hour(aux);
            }
            http_rest_server.send(200);
        }

    }else{
        http_rest_server.send(400);
    }
}

void set_water_time(){

    StaticJsonBuffer<500> jsonBuffer;
    String post_body = http_rest_server.arg("plain");
    JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    short aux;

    if (http_rest_server.method() == HTTP_POST) {
        if (!jsonBody.success()) {
            http_rest_server.send(400);
        }else{
            if(jsonBody.containsKey("second")){
                aux = jsonBody["second"];
                water_second = aux;
            }
            if(jsonBody.containsKey("minute")){
                aux = jsonBody["minute"];
                water_minute = aux;
            }
            if(jsonBody.containsKey("hour")){
                aux = jsonBody["hour"];
                water_hour = aux;
            }
            if(jsonBody.containsKey("duration")){
                aux = jsonBody["duration"];
                water_duration = aux;
            }
            http_rest_server.send(200);
        }

    }else{
        http_rest_server.send(400);
    }
}

void reset_water(){
    just_watered = false;
    http_rest_server.send(200);
}

void post_leds() {

    StaticJsonBuffer<500> jsonBuffer;
    String post_body = http_rest_server.arg("plain");
    Serial.println(post_body);
    JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));
    Serial.print("HTTP Method: ");
    Serial.println(http_rest_server.method());
    int a;

    if (!jsonBody.success()) {
        Serial.println("error in parsin json body");
        http_rest_server.send(400);
    }
    else {
        if (http_rest_server.method() == HTTP_POST) {

          if(jsonBody.containsKey("status")){
            a = jsonBody["status"];
            if (a == 0){
              status_led = false;
            }else{
              status_led = true;
            }
          }

          if(jsonBody.containsKey("high")){
            a = jsonBody["high"];
            high = a;
           EEPROM.put(0, a);
           EEPROM.commit();
         }


          if(jsonBody.containsKey("low")){
            a = jsonBody["low"];
            low = a;
            EEPROM.put(5, a);
            EEPROM.commit();
          }

            http_rest_server.send(201);

          }else{
              http_rest_server.send(404);}
    }
}

void config_rest_server_routing() {

    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "Welcome to the ESP8266 REST Web Server");
    });

    http_rest_server.on("/leds", HTTP_GET, get_leds);
    http_rest_server.on("/leds", HTTP_POST, post_leds);
    http_rest_server.on("/time", HTTP_GET, get_system_time);
    http_rest_server.on("/time", HTTP_POST, set_system_time);

    http_rest_server.on("/water", HTTP_POST, set_water_time);
    http_rest_server.on("/water", HTTP_GET, reset_water);

}

int init_wifi() {
    int retries = 0;

    Serial.println("Connecting to WiFi AP..........");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_passwd);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print("#");
    }
    return WiFi.status(); // return the WiFi connection status
}

void run_blink(){
  if(status_led){
    int now = millis();
    if(led)
    {
      if(now > _old + low)
      {
        led = false;
        _old = millis();
      }
    }else
    {
      if(now > _old + high)
      {
        led = true;
        _old = millis();
      }
    }
  }else
  {
    led = true;
  }
}

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

void ICACHE_RAM_ATTR onTimerISR(){
    // digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));  //Toggle LED Pin
    cookoo.run_on_interrupt();
    timer1_write(312500);
}

void setup()
{
  Serial.begin(115200);

  if (init_wifi() == WL_CONNECTED) {
       Serial.print("Connetted to ");
       Serial.print(wifi_ssid);
       Serial.print("--- IP: ");
       Serial.println(WiFi.localIP());
   }
   else {
       Serial.print("Error connecting to: ");
       Serial.println(wifi_ssid);
}

config_rest_server_routing();

http_rest_server.begin();
Serial.println("HTTP REST Server Started");

pinMode(LED_BUILTIN, OUTPUT);

 EEPROM.begin(10);
 EEPROM.get(0,high);
 EEPROM.get(5,low);

 cookoo.start();
 UDP.begin(123);

 short server_count = 0;

 while (!timeUNIX) {
     sendNTPpacket(timeServerIP[server_count]);
     delay(200);
     uint32_t time = getTime();
     if (time) {                                  // If a new timestamp has been received
         timeUNIX = time;
         Serial.print("NTP response:\t");
         Serial.println(timeUNIX);
     }else{
         server_count++;
         if (server_count == 5){
             ESP.reset();
         }
     }
 }

cookoo.set_second(timeUNIX % 60);
cookoo.set_minute((timeUNIX / 60) % 60);

short hour = ((timeUNIX / 3600) % 24)-2 ;
if (hour < 0){
    hour +=24;
}

cookoo.set_hour(hour);


timer1_attachInterrupt(onTimerISR);
timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
timer1_write(312500);

pinMode(D6, OUTPUT);

}




void check_time(){

    if(cookoo.get_hour() >= water_hour){
        if(cookoo.get_minute() >= water_minute){
            if(cookoo.get_second() >= water_second){
                if(!just_watered){
                    water_start = cookoo.get_second();
                    digitalWrite(D6, HIGH);
                    current_state = WATERING;
                }
            }
        }
    }


}

void water(){

    if(cookoo.get_second() > water_start + water_duration)
    {
        just_watered = true;
        digitalWrite(D6, LOW);
        current_state = IDLE;
    }

}

void loop() {

    // cookoo.run();

    if(WiFi.status()== WL_CONNECTED)
    {
        http_rest_server.handleClient();
    }
    else
    {
        ESP.reset();
    }


    switch(current_state)
    {
        case IDLE  :  check_time();  break;
        case WATERING : water(); break;
    }


    run_blink();
    digitalWrite(LED_BUILTIN, led);


}

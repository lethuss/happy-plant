
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

const char* wifi_ssid = "Ape CLT";
const char* wifi_passwd = "5enh@d0w1f1";

bool led = false;
long _old = millis();

int high = 500;
int low = 500;
bool status_led = true;

u_int64_t clock;

void get_leds() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
    jsonObj["status"] = led;
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
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
}


void loop() {

    if(WiFi.status()== WL_CONNECTED)
    {
      http_rest_server.handleClient();
    }
    else
    {
      ESP.reset();
    }
    run_blink();
    digitalWrite(LED_BUILTIN, led);
}

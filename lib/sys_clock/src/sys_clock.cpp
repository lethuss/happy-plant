#include "sys_clock.hpp"
#include "Arduino.h"


Sys_clock::Sys_clock():
    year(0),
    month(0),
    day(0),
    hour(0),
    minute(0),
    second(0)
    {}

Sys_timestamp::Sys_timestamp():
    hour(0),
    minute(0),
    second(0)
    {}

Sys_timestamp::Sys_timestamp(short h, short m, short s):
    hour(h),
    minute(m),
    second(s)
    {}


// Sys_timestamp & Sys_clock::get_timestamp(){
//     return Sys_timestamp(hour, minute, second);
// }

int Sys_clock::get_year()
{
    return year;
}

int Sys_clock::get_month()
{
    return month;
}

int Sys_clock::get_day()
{
    return day;
}

int Sys_clock::get_hour()
{
    return hour;
}

int Sys_clock::get_minute()
{
    return minute;
}

int Sys_clock::get_second()
{
    return second;
}

void Sys_clock::start()
{
    old = millis();
}

void Sys_clock::resolve_seconds()
{
    unsigned long now  = millis();
    if(now < old){ //check for rollover
        unsigned long up_delta = 0xFFFF - old;
        if (up_delta + now > 1000) {
            second++;
            old = now;
        }
    }else{
        if( now > old + 1000){
            second++;
            old = now;
        }
    }
}

void Sys_clock::run()
{
    resolve_seconds();

    if (second == 60){
        second = 0;
        minute++;
    }
    if (minute == 60){
        minute = 0;
        hour++;
    }
    if (hour == 24){
        hour = 0;
        day++;
    }
}

void Sys_clock::run_on_interrupt()
{

    second++;
    if (second == 60){
        second = 0;
        minute++;
    }
    if (minute == 60){
        minute = 0;
        hour++;
    }
    if (hour == 24){
        hour = 0;
        day++;
    }
}

void Sys_clock::set_time(short year_p, short month_p, short day_p, short hour_p, short minute_p, short second_p){

    if (year_p >=0)
    {
        year = year_p;
    }
    if (month_p >=0 && month_p <=12)
    {
        month = month_p;
    }
    if (day_p >=0 && day_p <=30)
    {
        day = day_p;
    }
    if (hour_p >=0 && hour_p <=24)
    {
        hour = hour_p;
    }
    if (minute_p >=0 && minute_p <=60)
    {
        minute = minute_p;
    }
    if (second_p >=0 && second_p <=60)
    {
        second = second_p;
    }
}

void Sys_clock::set_second(short second_p){
    if (second_p >= 0 && second_p <= 60)
    {
        second = second_p;
    }
}

void Sys_clock::set_minute(short minute_p){
    if (minute_p >=0 && minute_p <=60)
    {
        minute = minute_p;
    }
}

void Sys_clock::set_hour(short hour_p){
    if (hour_p >=0 && hour_p <=24)
    {
        hour = hour_p;
    }
}

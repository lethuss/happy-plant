#include "sys_clock.hpp"


Sys_clock::Sys_clock():
    year(0),
    month(0),
    day(0),
    hour(0),
    minute(0),
    second(0)
    {}

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
    old = millis()
}

void Sys_clock::start()
{
    if(millis() < old){ //check for rollover

    }else{

    }
}

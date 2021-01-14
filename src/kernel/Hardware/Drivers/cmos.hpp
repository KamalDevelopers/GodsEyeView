#ifndef CMOS_HPP
#define CMOS_HPP

#include "../port.hpp"
#include "stdio.hpp"
#include "stdlib.hpp"
#include "string.hpp"

#define CURRENT_YEAR 2020
#define UNIX_TIME 62168472000
#define SECONDS_YEAR 31557600
#define SECONDS_MONTH 2629800
#define SECONDS_WEEK 604800
#define SECONDS_DAY 86400
#define SECONDS_HOUR 3600
#define SECONDS_MIN 60

class TimeDriver {
public:
    TimeDriver();
    ~TimeDriver();

    virtual void read_rtc();
    virtual void SetTimezoneOffset(uint16_t t_offset);

    virtual unsigned char GetSecond();
    virtual unsigned char GetMinute();
    virtual unsigned char GetHour(uint16_t t_offset = 0);

    virtual unsigned char GetDay();
    virtual unsigned char GetMonth();
    virtual unsigned int GetYear();
    virtual unsigned int GetTime(); // Unix
    virtual char* GetFullTime();

protected:
    int century_register = 0x00;
    int timezone_offset = 2;

    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;

    Port8Bit cmos_address;
    Port8Bit cmos_data;

    unsigned char get_RTC_register(int reg);
    int get_update_in_progress_flag();
};
#endif

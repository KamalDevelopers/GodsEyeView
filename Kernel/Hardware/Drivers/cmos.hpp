#ifndef CMOS_HPP
#define CMOS_HPP

#include "../../Mem/mm.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"

#define CURRENT_YEAR 2020
#define UNIX_TIME 62168472000
#define SECONDS_YEAR 31557600
#define SECONDS_MONTH 2629800
#define SECONDS_WEEK 604800
#define SECONDS_DAY 86400
#define SECONDS_HOUR 3600
#define SECONDS_MIN 60

class TimeDriver {
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

    unsigned char get_rtc_register(int reg);
    int get_update_in_progress_flag();

public:
    TimeDriver();
    ~TimeDriver();

    static TimeDriver* time;
    virtual void read_rtc();
    virtual void set_timezone_offset(uint16_t t_offset);

    virtual unsigned char get_second();
    virtual unsigned char get_minute();
    virtual unsigned char get_hour(uint16_t t_offset = 0);

    virtual unsigned char get_day();
    virtual unsigned char get_month();
    virtual unsigned int get_year();
    virtual unsigned int get_time();
    virtual void get_full_time(char seperator, char* time);
};

#endif

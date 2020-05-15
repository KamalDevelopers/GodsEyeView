#ifndef CMOS_HPP
#define CMOS_HPP

#include "../port.hpp"
#include "itoa.hpp"
#include "stdio.hpp"
#include "string.hpp"

#define CURRENT_YEAR 2020

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
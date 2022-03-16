#ifndef CMOS_HPP
#define CMOS_HPP

#include "../../Mem/mm.hpp"
#include "../port.hpp"
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

#define CURRENT_YEAR 2020
#define UNIX_TIME 62168472000
#define SECONDS_YEAR 31557600
#define SECONDS_MONTH 2629800
#define SECONDS_WEEK 604800
#define SECONDS_DAY 86400
#define SECONDS_HOUR 3600
#define SECONDS_MIN 60
#define Time CMOS::active

class CMOS {
protected:
    int century_register = 0x00;
    int timezone_offset = 0;

    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;

    Port8Bit cmos_address;
    Port8Bit cmos_data;

    uint8_t get_rtc_register(int reg);
    int get_update_in_progress_flag();

public:
    CMOS();
    ~CMOS();

    static CMOS* active;
    void read_rtc();
    void set_timezone_offset(int time_offset);
    uint32_t timestamp();
};

#endif

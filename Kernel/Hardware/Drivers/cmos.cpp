#include "cmos.hpp"

CMOS* CMOS::active = 0;
CMOS::CMOS()
    : cmos_address(0x70)
    , cmos_data(0x71)
{
    active = this;
    set_timezone_offset(2);
    uptime = timestamp();
}

void CMOS::set_timezone_offset(int time_offset)
{
    timezone_offset = time_offset;
}

uint32_t CMOS::timestamp()
{
    read_rtc();
    uint32_t t;
    uint32_t y = year;
    uint32_t m = month;
    uint32_t d = day;

    if (m <= 2) {
        m += 12;
        y -= 1;
    }

    t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    t += (30 * m) + (3 * (m + 1) / 5) + d;
    t -= 719561;
    t *= 86400;
    t += (3600 * hour) + (60 * minute) + second;
    return t;
}

int CMOS::get_update_in_progress_flag()
{
    cmos_address.write(0x0A);
    return (cmos_data.read() & 0x80);
}

uint8_t CMOS::get_rtc_register(int reg)
{
    cmos_address.write(reg);
    return cmos_data.read();
}

void CMOS::read_rtc()
{
    uint8_t century;
    uint8_t last_second;
    uint8_t last_minute;
    uint8_t last_hour;
    uint8_t last_day;
    uint8_t last_month;
    uint8_t last_year;
    uint8_t last_century;
    uint8_t register_b;

    while (get_update_in_progress_flag())
        ;
    second = get_rtc_register(0x00);
    minute = get_rtc_register(0x02);
    hour = get_rtc_register(0x04);
    day = get_rtc_register(0x07);
    month = get_rtc_register(0x08);
    year = get_rtc_register(0x09);

    if (century_register != 0)
        century = get_rtc_register(century_register);

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag())
            ;
        second = get_rtc_register(0x00);
        minute = get_rtc_register(0x02);
        hour = get_rtc_register(0x04);
        day = get_rtc_register(0x07);
        month = get_rtc_register(0x08);
        year = get_rtc_register(0x09);
        if (century_register != 0) {
            century = get_rtc_register(century_register);
        }
    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) || (last_day != day) || (last_month != month) || (last_year != year) || (last_century != century));

    register_b = get_rtc_register(0x0B);

    if (!(register_b & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if (century_register != 0) {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    if (!(register_b & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    if (century_register != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if (year < CURRENT_YEAR)
            year += 100;
    }

    hour = hour + timezone_offset;
}

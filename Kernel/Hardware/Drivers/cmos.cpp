#include "cmos.hpp"

CMOS* CMOS::active = 0;
CMOS::CMOS()
    : cmos_address(0x70)
    , cmos_data(0x71)
{
    active = this;
    set_timezone_offset(2);
}

void CMOS::set_timezone_offset(int time_offset)
{
    timezone_offset = time_offset;
}

void CMOS::get_full_time(char seperator, char* time)
{
    char format[10];
    char sec[3], min[3], hour[3], middle[3];

    itoa(get_second(), sec);
    itoa(get_minute(), min);
    itoa(get_hour(), hour);

    memset(format, '\0', 9);
    format[2] = seperator;
    format[5] = seperator;

    if (strlen(hour) == 1) {
        format[0] = '0';
        format[1] = hour[0];
    } else {
        format[0] = hour[0];
        format[1] = hour[1];
    }
    if (strlen(min) == 1) {
        format[3] = '0';
        format[4] = min[0];
    } else {
        format[3] = min[0];
        format[4] = min[1];
    }
    if (strlen(sec) == 1) {
        format[6] = '0';
        format[7] = sec[0];
    } else {
        format[6] = sec[0];
        format[7] = sec[1];
    }

    format[9] = '\0';
    strcpy(time, format);
}

unsigned int CMOS::get_time()
{
    unsigned int yeardata = ((get_year() - 1970)) * SECONDS_YEAR;
    unsigned int monthdata = get_month() * SECONDS_MONTH;
    unsigned int daydata = get_day() * SECONDS_DAY;
    unsigned int hourdata = get_hour() * SECONDS_HOUR;
    unsigned int mindata = get_minute() * SECONDS_MIN;

    return yeardata + monthdata + daydata + hourdata + mindata + get_second();
}

unsigned char CMOS::get_second()
{
    read_rtc();
    return second;
}

unsigned char CMOS::get_minute()
{
    read_rtc();
    return minute;
}

unsigned char CMOS::get_hour()
{
    read_rtc();
    return hour;
}

unsigned char CMOS::get_day()
{
    read_rtc();
    return day;
}

unsigned char CMOS::get_month()
{
    read_rtc();
    return month;
}

unsigned int CMOS::get_year()
{
    read_rtc();
    return year;
}

int CMOS::get_update_in_progress_flag()
{
    cmos_address.write(0x0A);
    return (cmos_data.read() & 0x80);
}

unsigned char CMOS::get_rtc_register(int reg)
{
    cmos_address.write(reg);
    return cmos_data.read();
}

void CMOS::read_rtc()
{
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char register_b;

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

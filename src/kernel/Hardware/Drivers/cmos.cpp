#include "cmos.hpp"

TimeDriver::TimeDriver()
    : cmos_address(0x70)
    , cmos_data(0x71)
{
}

void TimeDriver::SetTimezoneOffset(uint16_t t_offset) { timezone_offset = t_offset; }

char* TimeDriver::GetFullTime()
{
    char *sec, *min, *hour, *middle;

    itoa(GetSecond(), sec);
    itoa(GetMinute(), min);
    itoa(GetHour(), hour);

    if (str_len(sec) == 1) {
        sec[1] = sec[0];
        sec[0] = '0';
    } else if (str_len(min) == 1) {
        min[1] = min[0];
        min[0] = '0';
    } else if (str_len(hour) == 1) {
        hour[1] = hour[0];
        hour[0] = '0';
    }

    strcat(hour, (char*)":\0");
    strcat(min, (char*)":\0");

    char* hourmin = strcat(hour, min);
    char* clck = strcat(hourmin, sec);

    return clck;
}
unsigned char TimeDriver::GetSecond()
{
    read_rtc();
    return second;
}
unsigned char TimeDriver::GetMinute()
{
    read_rtc();
    return minute;
}
unsigned char TimeDriver::GetHour(uint16_t t_offset)
{
    read_rtc();
    return hour + t_offset;
}

unsigned char TimeDriver::GetDay()
{
    read_rtc();
    return day;
}
unsigned char TimeDriver::GetMonth()
{
    read_rtc();
    return month;
}
unsigned int TimeDriver::GetYear()
{
    read_rtc();
    return year;
}

int TimeDriver::get_update_in_progress_flag()
{
    cmos_address.Write(0x0A);
    return (cmos_data.Read() & 0x80);
}

unsigned char TimeDriver::get_RTC_register(int reg)
{
    cmos_address.Write(reg);
    return cmos_data.Read();
}

void TimeDriver::read_rtc()
{
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    while (get_update_in_progress_flag())
        ;
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if (century_register != 0) {
        century = get_RTC_register(century_register);
    }

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
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if (century_register != 0) {
            century = get_RTC_register(century_register);
        }
    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) || (last_day != day) || (last_month != month) || (last_year != year) || (last_century != century));

    registerB = get_RTC_register(0x0B);

    if (!(registerB & 0x04)) {
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

    if (!(registerB & 0x02) && (hour & 0x80)) {
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
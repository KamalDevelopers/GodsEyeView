#include "stdio.hpp"
#include "../port.hpp"

#define CURRENT_YEAR 2020
static int century_register = 0x00;
static int timezone_offset = 2;

static unsigned char second;
static unsigned char minute;
static unsigned char hour;
static unsigned char day;
static unsigned char month;
static unsigned int year;

Port8Bit cmos_address(0x70);
Port8Bit cmos_data(0x71);
static void read_rtc();

namespace SysTime
{
      static void set_timezone_offset(uint16_t t_offset);
      static unsigned char get_second();
      static unsigned char get_minute();
      static unsigned char get_hour();
};

static void SysTime::set_timezone_offset(uint16_t t_offset) { timezone_offset = t_offset; }
static unsigned char SysTime::get_second() { read_rtc(); return second; }
static unsigned char SysTime::get_minute() { read_rtc(); return minute; }
static unsigned char SysTime::get_hour()   { read_rtc(); return hour; }

static int get_update_in_progress_flag() {
      cmos_address.Write(0x0A);
      return (cmos_data.Read() & 0x80);
}
 
static unsigned char get_RTC_register(int reg) {
      cmos_address.Write(reg);
      return cmos_data.Read();
}
 
static void read_rtc() {
      unsigned char century;
      unsigned char last_second;
      unsigned char last_minute;
      unsigned char last_hour;
      unsigned char last_day;
      unsigned char last_month;
      unsigned char last_year;
      unsigned char last_century;
      unsigned char registerB;
 
      while (get_update_in_progress_flag());
      second = get_RTC_register(0x00);
      minute = get_RTC_register(0x02);
      hour = get_RTC_register(0x04);
      day = get_RTC_register(0x07);
      month = get_RTC_register(0x08);
      year = get_RTC_register(0x09);
      if(century_register != 0) {
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
 
            while (get_update_in_progress_flag());
            second = get_RTC_register(0x00);
            minute = get_RTC_register(0x02);
            hour = get_RTC_register(0x04);
            day = get_RTC_register(0x07);
            month = get_RTC_register(0x08);
            year = get_RTC_register(0x09);
            if(century_register != 0) {
                  century = get_RTC_register(century_register);
            }
      } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
               (last_day != day) || (last_month != month) || (last_year != year) ||
               (last_century != century) );
 
      registerB = get_RTC_register(0x0B);
  
      if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(century_register != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
  
      if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      }
  
      if(century_register != 0) {
            year += century * 100;
      } else {
            year += (CURRENT_YEAR / 100) * 100;
            if(year < CURRENT_YEAR) year += 100;
      }
      hour = hour + timezone_offset;
}
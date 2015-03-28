/*!
 * \file
 *
 * \brief DS1307 RTC Library
 *
 * \details Can use either software bit banged or hardware TWI / I2C
 *
 *  Datasheets:  ac:ds1307_rtc
 *
 * \copyright AlphaLoewe
 *
 * \author Richard Lowe
 *
 * \date 06/12/2013
 *
 * \version 1.0 - Initial
 *
 * Status: 99% completed.
 *
 * \note
 * Test configuration:
 *   MCU:             %DEVICE%
 *   Dev.Board:       x
 *   Oscillator:      %DEVICE_CLOCK%
 *   Ext. Modules:    x
 *   SW:              %COMPILER%
 *
 * \par
 *   <all that matters>
 */

/*
  The rules for DST changed in 2007 for the first time in more than 20 years. The new changes were enacted by the Energy Policy Act of 2005, which extended the length of DST in the interest of reducing energy consumption. The new rules increased the duration of DST by about one month. DST will now be in effect for 238 days, or about 65% of the year, although Congress retained the right to revert to the prior law should the change prove unpopular or if energy savings are not significant. At present, Daylight Saving Time in the United States

  begins at 2:00 a.m. on the second Sunday of March and
  ends at 2:00 a.m. on the first Sunday of November
  
  A lithium battery with 48mAh or greater will back up the DS1307 for more than 10 years in the absence of power at +25°C

*/

#ifndef _DS1307_H
#define _DS1307_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/*!
 * \brief 24 or 12 hour modes
 *
 */ 
#define MODE12HOUR 1
#define MODE24HOUR 2

#define JANUARY   0x01
#define FEBUARY   0x02
#define MARCH     0x03
#define APRIL     0x04
#define MAY       0x05
#define JUNE      0x06
#define JULY      0x07
#define AUGUST    0x08
#define SEPTEMBER 0x09
#define OCTOBER   0x0A
#define NOVEMBER  0x0B
#define DECEMBER  0x0C

#define MONDAY    0x01
#define TUESDAY   0x02
#define WEDNESDAY 0x03
#define THURSDAY  0x04
#define FRIDAY    0x05
#define SATURDAY  0x06
#define SUNDAY    0x07

/*!
 * \brief RTC Modes
 *
 */
#define DS1307_SQW_1HZ        0x10
#define DS1307_SQW_4KHZ       0x11
#define DS1307_SQW_8KHZ       0x12
#define DS1307_SQW_32.768KHZ  0x13
#define DS1307_TGL_OUT        0x80

/*!
 *  \struct holds rtc config
 *
 */
typedef struct
{
    int8_t time_zone;         // Local time zone
    bool dst_enable;          // Enable or disable daylight savings time
    uint8_t output_config;    // Configuration for output pin
    bool mode_24hour;
} ds1307_config_t;


//*********  Prototypes  ********
/*!
 *  \brief Initializes RTC
 *
 *  \param ds1307_config_t : configuration of rtc
 *
 *  \code:
 *   ds1307_config_t rtc_config;
 *   rtc_config.time_zone = -8;
 *   rtc_config.dst_enable = true;
 *   rtc_config.config = DS1307_SQW_1HZ;
 *   rtc_config.mode_24hour = true;
 *
 *   ds1307_init( &rtc_config );
 */
void ds1307_init( ds1307_config_t* rtc_config );

/*!
 *  \brief Setting Time with GMT unix timestamp
 *
 *  Sets time of RTC based on GMT Time passed to it.
 *
 *  \param uint32_t GMTTime
 *
 */
void ds1307_set_time_GMT( uint32_t GMTTime );

/*!
 *  \brief Setting Time with individual hour, min, etc.
 *
 *  Sets time of RTC based on TimeStruct passed to it.
 *
 *  \param TimeStruct* set_time
 *
 */
void ds1307_set_time( TimeStruct* set_time );

/*!
 *  \brief Get GMT time
 *
 *  \returns TimeStruct* : Pointer to timestruct
 *
 */
TimeStruct* ds1307_get_GMT_time( void );

/*!
 *  \brief Get Local time
 *
 *  Returns a TimeStruct that represents gmt time with adjustment to timezone.
 *
 *  \returns TimeStruct* : Pointer to timestruct
 */
TimeStruct* ds1307_get_local_time( void );

/*!
 *  \brief Get GMT Time in unix timestamp form
 *
 */
int32_t ds1307_get_GMT_unix_time( void );

/*!
 *  \brief Get Local Time in unix timestamp form
 *
 */
int32_t ds1307_get_local_unix_time( void );

/*!
 *  \brief Get local Time in string format, choice of 12 or 24 hour format
 *
 */
char* ds1307_get_local_time_str( int mode );

/*!
 *  \brief Get GMT time in string format
 *
 */
char* ds1307_get_GMT_time_str( int mode );

/*!
 *  \brief Get default system time in string format
 *
 */
char* ds1307_get_system_time_str( void );

/*!
 *  \brief Writes scratch ram area of ds1307
 *
 */
void ds1307_write_ram( uint8_t addr, uint8_t datam );

/*!
 *  \brief Bulk write
 *
 */
void ds1307_write_ram_bulk( void* write_data, uint8_t size );

/*!
 *  \brief Reads scratch ram area of ds1307
 *
 */
uint8_t ds1307_read_ram( uint8_t addr );

/*!
 *  \brief Bulk read
 *
 */
void ds1307_read_ram_bulk( void* read_data, uint8_t size );

/*!
 *  \brief Read address
 *
 */
uint8_t ds1307_read_address( uint8_t address );

/*!
 *  \brief Get http get http gmt string
 *
 */
char* ds1307_get_http_gmt_str( void );

#endif
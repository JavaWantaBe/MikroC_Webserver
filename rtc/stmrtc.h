/*!
 * \file
 *
 * \brief Realtime Clock for STM32
 *
 * \details
 *
 * \copyright AlphaLoewe
 *
 * \author Richard Lowe
 *
 * \date
 *
 * \version <versionNo> - <change_description>
 *
 * Status: <XX% completed.>
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

#ifndef _STMRTC_H
#define _STMRTC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

/*
  The rules for DST changed in 2007 for the first time in more than 20 years. The new changes were enacted by the Energy Policy Act of 2005, which extended the length of DST in the interest of reducing energy consumption. The new rules increased the duration of DST by about one month. DST will now be in effect for 238 days, or about 65% of the year, although Congress retained the right to revert to the prior law should the change prove unpopular or if energy savings are not significant. At present, Daylight Saving Time in the United States
  
  begins at 2:00 a.m. on the second Sunday of March and
  ends at 2:00 a.m. on the first Sunday of November

*/

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

typedef struct
{
    int8_t time_zone;     // Local time zone
    bool dst_enable;      // Enable or disable daylight savings time 
    TimeStruct dst_begin; // Rule for when to start daylight savings time
    TimeStruct dst_end;   // Rule for when to end daylight savings time
    
} stmrtc_config_t;

//*******  Prototypes  ********
// Initialize rtc with local Time offset and enable or disable dst mode
void stmrtc_init                  ( stmrtc_config_t* rtc_config );
// Setting Time with GMT unix timestamp
void stmrtc_set_time_GMT          ( int32_t GMTTime );
// Setting Time with individual hour, min, sec
void stmrtc_set_time              ( TimeStruct* set_time );
// Get GMT time
TimeStruct* stmrtc_get_GMT_time    ( void );
// Get Local time
TimeStruct* stmrtc_get_local_time  ( void );
// Get GMT Time in unix timestamp form
int32_t stmrtc_get_GMT_unix_time  ( void );
// Get Local Time in unix timestamp form
int32_t stmrtc_get_local_unix_time( void );
// Get local Time in string format, choice of 12 or 24 hour formats
char* stmrtc_get_local_time_str   ( int mode );
// Get GMT Time in string format
char* stmrtc_get_GMT_time_str     ( int mode );
// Get default system Time in string format
char* stmrtc_get_system_time_str  ( void );
// Get HTTP formatted GMT string
char* stmrtc_get_http_gmt_str          ( void );

#endif
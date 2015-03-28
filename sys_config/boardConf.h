#ifndef _BOARDCONF_H
#define _BOARDCONF_H


#include "__Lib_FAT32.h"

/*****************************
 *  Configuration Functions
 ****************************/
/*!
 *  \brief Board Initialization
 *
 *  Configures board memory management, two wire, and settings manager.
 *
 */
void configure_board( void );

// Configure timers
void configure_timers( void );

/*!
 *  \brief Configure the RTC
 *
 *  Configure the RTC settings as well as the input pin for
 *  INT0 used by the square wave generated from the RTC.
 *  Falling edge is required as per datasheet  ac:ds1307_rtc
 *
 */
void configure_rtc( void );

// Start file system
int8 configure_file_system( void );

/*!
 *  \brief Configure the webserver
 *
 *   Mac address is set by a literal.  All other settings are read from
 *   EEPROM and used to initialize the webserver.
 *
 */
void configure_webserver( void );

#ifdef DEBUG
// Debug terminal
void configure_terminal( void );
#endif

#endif
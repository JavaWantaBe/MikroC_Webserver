/*!
 * \file
 *
 * \brief Saves and reads system Settings to and from EEPROM
 *
 * Copyright:
 *   (c) Richard, 12/23/2012
 * Revision History:
 *   .01 - Working
 *   .02 - Added starting define of 3000 block for default settings
 * Status:
 *   50% completed.
 *
 * Test configuration:
 *   MCU:             ATMega1284p
 *   Dev.Board:       AVRPLC16
 *   Oscillator:      8Mhz
 *   Ext. Modules:    x
 *   SW:              MikroC 5.8
 * NOTES:
 *
 */

#ifndef _SETTINGS_MANAGER_H
#define _SETTINGS_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define SETTINGS_NETWORK   0x01
#define SETTINGS_TIDES     0x02
#define SETTINGS_PUMP      0x04
#define SETTINGS_RTC       0x08


typedef struct
{
    // IP Address
    uint8_t myIP[4];            //0x0000 - 0x0003
    // Sub-net Address
    uint8_t subIP[4];           //0x0004 - 0x0007
    // Gateway address
    uint8_t gwIP[4];            //0x0008 - 0x000B
    // Domain name server address
    uint8_t dnsIP[4];           //0x000C - 0x000F
    // DHCP Mode
    bool DHCP_enable;           //0x0010 - 0x0011
    // HTTP Server port
    uint8_t port;               //0x0012 - 0x0012
    // NTP server used
    char time_server[29];       //0x0013 - 0x002F

} network_settings_t;           //0x0000 - 0x002F

// Tide description
typedef struct
{
    // Time of last update
    uint32_t last_update;       //0x0030 - 0x0033
    // Alarm desc
    uint32_t tide_alarms[20];   //0x0034 - 0x0083
    // Station ID
    uint32_t station_id;        //0x0084 - 0x0088

} tide_settings_t;              //0x0030 - 0x0087

// Pump saved settings
typedef  struct
{
    // Last pump number to be cycled
    uint8_t last_pump_cycled;   //0x0088 - 0x0088
    // Last AVG gpm for pump 1
    uint16_t last_pumping1_gpm; //0x0089 - 0x008A
    // Last AVG gpm for pump 2
    uint16_t last_pumping2_gpm; //0x008B - 0x008C
    // Total volume
    uint32_t month_volume_pump1[12];  //0x008D - 0x00BC
    uint32_t month_volume_pump2[12];  //0x00BD - 0x00EC
} pump_settings_t;              //0x0088 - 0x00EC

typedef struct
{
    // Timezone
    int8_t rtc_time_zone;       //0x00ED - 0x00ED
    // Time server
    bool dst_enable;            //0x00EE - 0x00EF

} rtc_settings_t;               //0x00ED - 0x00EF


/*!
 *  \struct Holds all system settings
 *
 *
 *  NOTES:
 *   Do not change order or add without hex modification
 */
typedef struct
{
    //0x0000 - 0x002F
    network_settings_t network_settings;
    //0x0004 - 0x0007
    tide_settings_t tide_settings;
    //0x0088 - 0x00BC
    pump_settings_t pump_settings;
    //0x00ED - 0x00EF
    rtc_settings_t rtc_settings;

} settings_t;   //0x0000 - 0x00BF

/*!
 *  \brief Restores all system settings to default
 *
 *  \param settings_desc struct that holds all system settings
 *
 *  NOTES:
 *   <notes>
 */
void settings_restore_default( settings_t* settings );

/*!
 *  \brief Holds all pointers to functions.
 *
 *  NOTES:
 *   <notes>
 */
void settings_init( void );

void settings_check_data( void );

/*!
 *  \brief Returns webserver settings
 *
 *  \return webserver_settings_t *
 *
 *  \note
 *   <notes>
 */
network_settings_t* settings_get_network( void );

/*!
 *  \brief Returns tide settings
 *
 *  \return tide_settings_t*
 *
 *  \note
 *   <notes>
 */
tide_settings_t* settings_get_tide( void );

/*!
 *  \brief Returns settings for pump
 *
 *  \return pump_settings_t*
 *
 *  \note
 *   <notes>
 */
pump_settings_t* settings_get_pump( void );

/*!
 *  \brief Returns rtc settings
 *
 *  \return rtc_settings_t*
 *
 *  \note
 *   <notes>
 */
rtc_settings_t* settings_get_rtc( void );

/*!
 *  \brief Saves any changes to settings
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void settings_commit( int module_to_save );


char* settings_get_JSdata( void );
void settings_generate_data( void );

#endif
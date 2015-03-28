/*
 * NOTES:
 *      External EEPROM used =  ac:eeprom_datasheet
 */

#include "settings_manager.h"
#include <stddef.h>

#define EEPROM_ENDING_ADDRESS     0x0FFF
#define EEPROM_START_ADDRESS      0x0000

/*
 * It is recommended to have the Global Interrupt Flag cleared
 * during all the steps to avoid these problems.  Datasheet page 23
 *
 *  Table 6-2. EEPROM Programming Time
 *  Symbol Number of Calibrated RC Oscillator Cycles Typ Programming Time
 *  EEPROM write(from CPU)
 *  26,368 3.3 ms
 *
 */
 
/******************************
 *  Private
 *****************************/
static void settings_read_from_eeprom( uint16_t source_address, void* destination_address, size_t data_ );
static void settings_save_to_eeprom( void* source_address, uint16_t destination_address, size_t data_ );
static void settings_commit_new_settings( void );

/******************************
 *   Globals
 *****************************/
static char set_js[] = 
"var settings = {username:'root',ip:[192,168,001,150],sm:[255,255,255,000],gw:\
[192,168,001,001],dns1:[8,8,8,8],dns2:[8,8,4,4],hp:80,dchp:true,ts:\
'ntp.pool.org',station:1,systemtime:[2013, 09, 19, 15, 38, 37, 0],emaillist:\
['ocdrichard@gmail.com','muncy.ryan@gmail.com'],smslist:['5412971063']};";
// Current configuration
static settings_t current_settings;
// Flag toggled when new data is written to the memory model
static uint8_t settings_flags = 0;

/*!
 *  \brief Loads settings into master current_settings
 *
 *  \note
 *   <notes>
 */
static void settings_read_from_eeprom( uint16_t source_address, void* destination_address, size_t data_size )
{
    uint8_t* p_dest = ( uint8_t* )destination_address;
    
    asm cli;

    while( data_size-- > 0 )
    {
        *p_dest++ = EEPROM_Read( source_address++ );
    }
    
    asm sei;

}

/*!
 *  \brief <Basic Description>
 *
 *  \note
 *   <notes>
 */
static void settings_save_to_eeprom( void* source_address, uint16_t destination_address, size_t data_size )
{
    uint8_t* p_source = ( uint8_t* )source_address;
    
    asm cli;
    
    while( data_size-- > 0 )
    {
         EEPROM_Write( destination_address++, *p_source++ );
    }
    
    asm sei;

}

/*!
 *   \brief commits newly written data to eeprom
 *
 */
static void settings_commit_new_settings()
{
    uint16_t address;

    while( settings_flags != 0 )
    {
        if( settings_flags & SETTINGS_NETWORK )
        {
            address = EEPROM_START_ADDRESS + offsetof( settings_t, network_settings );
            settings_save_to_eeprom( ( void* )&current_settings.network_settings, address, sizeof( network_settings_t ) );
            settings_flags &= ~SETTINGS_NETWORK;
        }
        else if( settings_flags & SETTINGS_TIDES )
        {
            address = EEPROM_START_ADDRESS + offsetof( settings_t, tide_settings );
            settings_save_to_eeprom( ( void* )&current_settings.tide_settings, address, sizeof( tide_settings_t ) );
            settings_flags &= ~SETTINGS_TIDES;
        }
        else if ( settings_flags & SETTINGS_PUMP )
        {
            address = EEPROM_START_ADDRESS + offsetof( settings_t, pump_settings );
            settings_save_to_eeprom( ( void* )&current_settings.pump_settings, address, sizeof( pump_settings_t ) );
            settings_flags &= ~SETTINGS_PUMP;
        }
        else if( settings_flags & SETTINGS_RTC )
        {
            address = EEPROM_START_ADDRESS + offsetof( settings_t, rtc_settings );
            settings_save_to_eeprom( ( void* )&current_settings.rtc_settings, address, sizeof( rtc_settings_t ) );
            settings_flags &= ~SETTINGS_RTC;
        }
    }
}

/*!
 *  \brief Restore settings from remote region to start
 *
 *  \note
 *   <notes>
 */
void settings_restore_default( settings_t* settings )
{
    settings_read_from_eeprom( ( EEPROM_ENDING_ADDRESS - sizeof( settings_t ) ), ( void* )&current_settings, sizeof( settings_t ) );
    settings_save_to_eeprom( ( void* )&current_settings, EEPROM_START_ADDRESS, sizeof( settings_t ) );
}

/*!
 *  \brief EEPROM Initialization
 *
 */
void settings_init()
{
    // Settings are populated into our memory based buffer model
    settings_read_from_eeprom( EEPROM_START_ADDRESS, ( void* )&current_settings, sizeof( settings_t ) );
    Delay_ms(500);
}

/*!
 *  \brief Checks for new data that needs to be written to EEPROM
 *
 *  \note
 *   <notes>
 */
void settings_check_data()
{
   if( settings_flags != 0 )
   {
       settings_commit_new_settings();
   }
}

// Returns webserver settings
network_settings_t* settings_get_network()
{
    // Returns webserver settings in memory ( could be inline if available )
    return &current_settings.network_settings;
}

// Returns tide settings
tide_settings_t* settings_get_tide()
{
    return &current_settings.tide_settings;
}

// Returns pump settings
pump_settings_t* settings_get_pump()
{
    return &current_settings.pump_settings;
}

// Returns rtc settings
rtc_settings_t* settings_get_rtc()
{
    return &current_settings.rtc_settings;
}


void settings_commit( int module_to_save )
{
    switch( module_to_save )
    {
       case SETTINGS_NETWORK:
          settings_flags |= SETTINGS_NETWORK;
       break;
       case SETTINGS_TIDES:
          settings_flags |= SETTINGS_TIDES;
       break;
       case SETTINGS_PUMP:
          settings_flags |= SETTINGS_PUMP;
       break;
       case SETTINGS_RTC:
          settings_flags |= SETTINGS_RTC;
       break;
    }
}

char* settings_get_JSdata()
{
   return set_js;
}

void settings_generate_data()
{


}

// Sensors
//  Set TEMP_RESOLUTION to the corresponding resolution of used DS18x20 sensor:
//  18S20: 9  (default setting; can be 9,10,11,or 12)
//  18B20: 12
#include "ds18x20.h"

static char text[11];
static uint8_t res;
static float current_temp;

void ds18x20_init( uint8_t resolution, uint8_t alarm_high, uint8_t alarm_low )
{
    uint8_t config;
    res = resolution;
    config = RESET_CONF | resolution;                                 // Configure DS resolution

    Ow_Reset( DS18X20_BASE_ADDRESS, DS18X20_PIN );                    // Onewire reset signal
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, SKIP_ROM );          // Issue command SKIP_ROM
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, WRITE_SCRATCHPAD );  // Issue command CONFIG_RES
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, alarm_high );        // Th = 0
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, alarm_low );         // Tl = 0
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, config );            // Config N bits

    ds18x20_upd_temp();
}

bool ds18x20_is_alarm()
{


   return FALSE;
}

float ds18x20_get_temp()
{
    uint16_t temp = 0;
    int i = 0;
    uint8_t oData[9] = {0};

    Ow_Reset( DS18X20_BASE_ADDRESS, DS18X20_PIN );            // Onewire reset signal
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, SKIP_ROM );  // Issue command SKIP_ROM
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, CONVERT_T ); // Issue command CONVERT_T

    switch( res )
    {
        case RES12:
            Delay_ms( 750 );
            break;

        case RES11:
            Delay_ms( 375 );
            break;

        case RES10:
            Delay_ms( 187 );
            break;

        case RES9:
            Delay_ms( 94 );
            break;
    }

    Ow_Reset( DS18X20_BASE_ADDRESS, DS18X20_PIN );
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, SKIP_ROM );                 // Issue command SKIP_ROM
    Ow_Write( DS18X20_BASE_ADDRESS, DS18X20_PIN, READ_SCRATCHPAD );                 // Issue command READ_SCRATCHPAD

    for( i = 0; i < 9; i++ )
    {
        oData[i] = Ow_Read( DS18X20_BASE_ADDRESS, DS18X20_PIN );
    }

    temp = oData[0];

    if( oData[1] & 0x80 )
    {
        temp = ~temp + 1;
    }

    temp = temp >> 1;

    return ( temp - ( float )0.25 + ( oData[7] - oData[6] )/( float )oData[7] );
}

/*!
 *  \brief Gets current board temp
 *
 *  \param unsigned int *temp unsigned int used to populate
 *
 */
void ds18x20_upd_temp()
{
    current_temp = ds18x20_get_temp();
}

float ds18x20_get_upd_temp()
{
    return current_temp;
}

float ds18x20_CtoF( float val )
{
    float aux = ( val * 9 / 5 );
    return ( aux + 32 );
}

/*!
 *  \brief Takes a temp and returns formatted text
 *
 *  \return char * - formatted string
 *    \retval 22.0C
 *
 */
char* ds18x20_get_str_temp( int mode )
{
    static char txt[9];

    if( mode )
    {
        FloatToStr( ds18x20_CtoF( ds18x20_get_temp() ), txt );
        txt[5] = ' ';
        txt[6] = 'F';
        txt[7] = 176;
        txt[8] ='\0';
    }

    else
    {
        FloatToStr( ds18x20_get_temp(), txt );
        txt[5] = ' ';
        txt[6] = 'C';
        txt[7] = 176;
        txt[8] ='\0';
    }

    return txt;
}
#include "ds1307.h"
#include <stddef.h>

#define SECONDS   0x7F
#define MINUTES   0x7F
#define HOURS     0x3F
#define DAY       0x07
#define DATE      0x3F
#define MONTH     0x1F
#define YEAR      0xFF
#define CONTROL   0x93

#define DS1307_SECONDS_ADDR   0x00
#define DS1307_MINUTES_ADDR   0x01
#define DS1307_HOURS_ADDR     0x02
#define DS1307_DAY_ADDR       0x03
#define DS1307_DATE_ADDR      0x04
#define DS1307_MONTH_ADDR     0x05
#define DS1307_YEAR_ADDR      0x06
#define DS1307_CONFIG_ADDR    0x07
#define DS1307_RAM_START_ADDR 0x08

#define DS1307_ACK    1
#define DS1307_NO_ACK 0

#define DS1307_24_12_BIT 6
#define DS1307_OSC_BIT 7
#define DS1307_AMPM_BIT 0x20

#define DS1307_WRITE 0xD0
#define DS1307_READ  0xD1

static const code uint8_t* days_week[] =
{
    "NULL","Mon","Tue","Wed","Thu","Fri","Sat","Sun"
};

static const code uint8_t* months_year[] =
{
    "NULL","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static TimeStruct time_GMT, time_local, dst_time_start, dst_time_stop;
static uint32_t last_time_set;
static int8_t local_time_zone;
static bool dst_enable;
static bool mode_24hour;
static bool PM;  // AM == false PM == true;

/****************************************
 *  Private Prototypes
 ***************************************/
static void ds1307_write_configuration( uint8_t cfg );
static void ds1307_set_dst( uint16_t year );

/*!
 *  \brief <Basic Description>
 *
 *  \param <Parameters Accepted>
 *
 */
static void ds1307_write_configuration( uint8_t cfg )
{
    if( cfg & 0x6C )
    {
        return;
    }

    TWI_Start();
    TWI_Write( DS1307_WRITE );
    TWI_Write( DS1307_CONFIG_ADDR );
    TWI_Write( cfg );
    TWI_Stop();
}

/*
 * USA starting in 2007, extended DST to begin
 * on the 2nd Sunday in March (Day of month = 14 - (1+Y*5/4)mod 7) and
 * end on the 1st Sunday in November (Day of month = 7 - (1 + 5*Y/4) mod 7).
 */
static void ds1307_set_dst( uint16_t year )
{
    dst_time_start.ss = 0;
    dst_time_start.mn = 0;
    dst_time_start.hh = 2;
    dst_time_start.wd = SUNDAY;
    dst_time_start.md = 14 - ( ( 1 + ( ( year * 5 ) / 4 ) ) % 7 );
    dst_time_start.mo = MARCH;
    dst_time_start.yy = year;

    dst_time_stop.ss  = 0;
    dst_time_stop.mn  = 0;
    dst_time_stop.hh  = 2;
    dst_time_stop.wd  = SUNDAY;
    dst_time_stop.md  = 7 - ( ( 1 + ( ( year * 5 ) / 4 ) ) % 7 );
    dst_time_stop.mo  = NOVEMBER;
    dst_time_stop.yy  = year;
}



/****************************************
 *  Functions
 ***************************************/
/*!
 *  \brief Initializes rtc
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *     Please note that the initial power-on state of all registers is not defined. Therefore, it is important
 *     to enable the oscillator (CH bit = 0) during initial configuration.
 */
void ds1307_init( ds1307_config_t* rtc_config )
{
    if( rtc_config->time_zone >= -16 && rtc_config->time_zone <= 16 )
    {
        char config = 0;
        char hours = 0;

        local_time_zone = rtc_config->time_zone;
        dst_enable      = rtc_config->dst_enable;
        mode_24hour     = rtc_config->mode_24hour;

        TWI_Start();
        TWI_Write( DS1307_WRITE );
        TWI_Write( DS1307_SECONDS_ADDR );
        TWI_Start();
        TWI_Write( DS1307_READ );
        config = TWI_Read( DS1307_ACK );
        TWI_Read( DS1307_ACK );
        hours = TWI_Read( DS1307_NO_ACK );
        hours &= ( 1 << DS1307_24_12_BIT );
        TWI_Stop();

        /* This checks condition of the clock enable.  When the rtc is
         * initialized for the first time, or after the battery is replaced,
         * the state of the RTC register is undertermined.
         */
        if( config & 0x80 )
        {
            TWI_Start();               // Intialize TWI communication
            TWI_Write( DS1307_WRITE ); // Send a read command
            TWI_Write( DS1307_SECONDS_ADDR );       // Start at register 0
            config &= ~( 1 << DS1307_OSC_BIT );     // Reset the 8th bit to 0 to start
            TWI_Write( config );       // Write to 0x00 register
            TWI_Stop();
        }

        if( hours & ( 1 << DS1307_24_12_BIT ) && mode_24hour == false )
        {
            TWI_Start();
            TWI_Write( DS1307_WRITE );
            TWI_Write( DS1307_HOURS_ADDR );
            hours &= ~( 1 << DS1307_24_12_BIT );
            TWI_Write( hours );
            TWI_Stop();
        }

        if( dst_enable == true )
        {
            uint16_t year = 0;

            TWI_Start();
            TWI_Write( DS1307_WRITE );
            TWI_Write( DS1307_YEAR_ADDR );
            TWI_Start();
            TWI_Write( DS1307_READ );
            year = TWI_Read( DS1307_NO_ACK );
            TWI_Stop();

            ds1307_set_dst( year );
        }

        if( rtc_config->output_config != 0 )
        {
            ds1307_write_configuration( rtc_config->output_config );
        }
    }
}

/*!
 *  \brief Sets Time based on GMT timestamp
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void ds1307_set_time_GMT( uint32_t GMTTime )
{
    Time_epochToDate( GMTTime, &time_GMT );
    ds1307_set_time( &time_GMT );
}

/*!
 *  \brief Set Time based on TimeStruct
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void ds1307_set_time( TimeStruct* set_time )
{
    TWI_Start();                               // Intialize TWI communication
    TWI_Write( DS1307_WRITE );                         // Send a write command
    TWI_Write( DS1307_SECONDS_ADDR );                            // Start at register 0
    TWI_Write( ( Dec2Bcd ( set_time->ss ) & SECONDS ) );
    TWI_Write( ( Dec2Bcd ( set_time->mn ) & MINUTES ) );

    if( mode_24hour == true )
    {
        TWI_Write( ( Dec2Bcd ( set_time->hh ) & HOURS ) );
    }
    else
    {
        //TWI_Write( ( Dec2Bcd ( set_time->hh ) & HOURS ) );
    }

    TWI_Write( ( Dec2Bcd ( ( set_time->wd ) ) & DAY ) + 1 );
    TWI_Write( ( Dec2Bcd ( set_time->md ) & DATE ) );
    TWI_Write( ( Dec2Bcd ( set_time->mo ) & MONTH ) );
    TWI_Write( ( Dec2Bcd ( ( ( uint8_t )set_time->yy - 2000 ) ) ) & YEAR );
    TWI_Stop();

    last_time_set = ds1307_get_GMT_unix_time();
}

/*!
 *  \brief Get GMT Time struct
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
TimeStruct* ds1307_get_GMT_time()
{
    TWI_Start();                      // TWI Start
    TWI_Write( DS1307_WRITE );        // TWI Write
    TWI_Write( DS1307_SECONDS_ADDR ); // TWI Start at 0
    TWI_Stop();
    TWI_Start();                      // TWI Start
    TWI_Write( DS1307_READ );
    time_GMT.ss = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & SECONDS ) );
    time_GMT.mn = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & MINUTES ) );

    if( mode_24hour == true )
    {
        time_GMT.hh = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & HOURS ) );
    }
    else
    {
        uint8_t temp_hour = TWI_Read( DS1307_ACK );

        if( temp_hour & DS1307_AMPM_BIT )
        {
           temp_hour = ( uint8_t )Bcd2Dec( temp_hour & 0x1F );
           temp_hour += 12;
        }
        else
        {
            temp_hour = ( uint8_t )Bcd2Dec( temp_hour& 0x1F );

            if( temp_hour == 12 )
            {
                temp_hour = 0;
            }
        }

        time_GMT.hh = temp_hour;
    }

    time_GMT.wd = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & DAY ) -1 );
    time_GMT.md = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & DATE ) );
    time_GMT.mo = ( uint8_t )Bcd2Dec( ( TWI_Read( DS1307_ACK ) & MONTH ) );
    time_GMT.yy = ( uint16_t )Bcd2Dec( ( TWI_Read( DS1307_NO_ACK ) & YEAR ) ) + 2000 ;
    TWI_Stop();

    return &time_GMT;
}

/*!
 *  \brief Returns localtime
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
TimeStruct* ds1307_get_local_time()
{
    // Holds GMT time in timestamp
    uint32_t rtc_time = ds1307_get_GMT_unix_time();

    if( dst_enable == true )
    {
        int8_t new_time_zone = local_time_zone;

        if( time_local.mo >= MARCH && time_local.mo <= NOVEMBER )
        {
            if( time_local.mo == MARCH )
            {
                if( time_local.md >= 8 )
                {
                    new_time_zone += 1;
                }

                else
                {
                    if ( time_local.wd == SUNDAY )
                    {

                    }
                }
            }
            else if( time_local.mo == NOVEMBER )
            {
                if( time_local.md <= 8 )
                {
                    new_time_zone += 1;
                }

            }
            else
            {
                new_time_zone += 1;
            }
        }

        rtc_time += ( uint32_t )( new_time_zone * 3600 );
    }
    else
    {
        rtc_time += ( uint32_t )( local_time_zone * 3600 );
    }

    Time_epochToDate( rtc_time, &time_local );

    return &time_local;

}


/*!
 *  \brief Gets gmt unix timestamp
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int32_t ds1307_get_GMT_unix_time()
{
    TimeStruct* ts2 = ds1307_get_GMT_time();

    return Time_dateToEpoch( ts2 );
}

/*!
 *  \brief <Basic Description>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int32_t ds1307_get_local_unix_time()
{
   return Time_dateToEpoch( ds1307_get_local_time() );
}

/*!
 *  \brief Return formatted time
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 */
char* ds1307_get_local_time_str( int mode )
{
    static char formated_local_time[20];
    TimeStruct* tmp_time;

    tmp_time = ds1307_get_local_time();

    if( mode == MODE12HOUR )
    {
        uint8_t x;

        x = tmp_time->hh;

        if( x == 0 )
        {
            x += 12;
        }

        else if( x >= 13 )
        {
            x -= 12;
        }

        formated_local_time[0] = ( x / 10 ) + 48;
        formated_local_time[1] = ( x % 10 ) + 48;
    }
    else
    {
        formated_local_time[0] = ( tmp_time->hh / 10 ) + 48;
        formated_local_time[1] = ( tmp_time->hh % 10 ) + 48;
    }

    formated_local_time[2] = ':';
    formated_local_time[3] = ( tmp_time->mn / 10 ) + 48;
    formated_local_time[4] = ( tmp_time->mn % 10 ) + 48;
    formated_local_time[5] = ':';
    formated_local_time[6] = ( tmp_time->ss / 10 ) + 48;
    formated_local_time[7] = ( tmp_time->ss % 10 ) + 48;
    formated_local_time[8] = ' ';
    formated_local_time[9] = ( tmp_time->mo / 10 ) + 48;
    formated_local_time[10] = ( tmp_time->mo % 10 ) + 48;
    formated_local_time[11] = '/';
    formated_local_time[12] = ( tmp_time->md / 10 ) + 48;
    formated_local_time[13] = ( tmp_time->md % 10 ) + 48;
    formated_local_time[14] = '/';
    formated_local_time[15] = ( tmp_time->yy / 1000 ) + 48;
    formated_local_time[16] = ( ( tmp_time->yy % 1000 ) / 100 ) + 48;
    formated_local_time[17] = ( ( tmp_time->yy % 100 ) / 10 ) + 48;
    formated_local_time[18] = ( tmp_time->yy % 10 ) + 48;
    formated_local_time[19] = '\0';

    return formated_local_time;

}

/*!
 *  \brief <Basic Description>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
char* ds1307_get_GMT_time_str( int mode )
{
    static char formated_gmt_time[20];
    TimeStruct* tmp_time;

    tmp_time = ds1307_get_GMT_time();

    if( mode == MODE12HOUR )
    {
        uint8_t x;

        x = tmp_time->hh;

        if( x == 0 )
        {
            x += 12;
        }

        else if( x >= 13 )
        {
            x -= 12;
        }

        formated_gmt_time[0] = ( x / 10 ) + 48;
        formated_gmt_time[1] = ( x % 10 ) + 48;
    }

    else
    {
        formated_gmt_time[0] = ( tmp_time->hh / 10 ) + 48;
        formated_gmt_time[1] = ( tmp_time->hh % 10 ) + 48;
    }

    formated_gmt_time[2] = ':';
    formated_gmt_time[3] = ( tmp_time->mn / 10 ) + 48;
    formated_gmt_time[4] = ( tmp_time->mn % 10 ) + 48;
    formated_gmt_time[5] = ':';
    formated_gmt_time[6] = ( tmp_time->ss / 10 ) + 48;
    formated_gmt_time[7] = ( tmp_time->ss % 10 ) + 48;
    formated_gmt_time[8] = ' ';
    formated_gmt_time[9] = ( tmp_time->mo / 10 ) + 48;
    formated_gmt_time[10] = ( tmp_time->mo % 10 ) + 48;
    formated_gmt_time[11] = '/';
    formated_gmt_time[12] = ( tmp_time->md / 10 ) + 48;
    formated_gmt_time[13] = ( tmp_time->md % 10 ) + 48;
    formated_gmt_time[14] = '/';
    formated_gmt_time[15] = ( tmp_time->yy / 1000 ) + 48;
    formated_gmt_time[16] = ( ( tmp_time->yy % 1000 ) / 100 ) + 48;
    formated_gmt_time[17] = ( ( tmp_time->yy % 100 ) / 10 ) + 48;
    formated_gmt_time[18] = ( tmp_time->yy % 10 ) + 48;
    formated_gmt_time[19] = '\0';

    return formated_gmt_time;
}

/*!
 *  \brief <Basic Description>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
char* ds1307_get_system_time_str()
{
   return ds1307_get_local_time_str( MODE24HOUR );
}

/*!
 *  \brief Write to scratchpad - ram
 *
 *  \param unsigned char addr
 *  \param unsigned char datam
 *
 */
void ds1307_write_ram( uint8_t addr, uint8_t datam )
{
    if( addr < 0x08 || addr > 0x3F )
    {
       return;
    }

    TWI_Start();
    TWI_Write( DS1307_WRITE );
    TWI_Write( addr );
    TWI_Write( datam );
    TWI_Stop();
}

// Bulk write
void ds1307_write_ram_bulk( void* write_data, uint8_t size )
{
    if( size > 56 )
    {
       return;
    }
    else
    {
        uint8_t* p_data = ( uint8_t* )write_data;
        uint8_t start_addr = 0x08;

        TWI_Start();
        TWI_Write( DS1307_WRITE );
        TWI_Write( start_addr );

        while( size-- > 0 )
        {
            TWI_Write( *p_data++ );
        }

        TWI_Stop();
    }
}

/*!
 *  \brief Read scratchpad - ram
 *
 *  \param addr - address of ram area to read
 *
 *  \return unsigned char
 *
 */
uint8_t ds1307_read_ram( uint8_t addr )
{
    if( addr < 0x08 || addr > 0x3F )
    {
       return 0;
    }
    else
    {
        static uint8_t datam = 0;

        TWI_Start();
        TWI_Write( DS1307_WRITE );
        TWI_Write( addr );
        TWI_Start();
        TWI_Write( DS1307_READ );
        datam = TWI_Read( DS1307_NO_ACK );
        TWI_Stop();

        return datam;
    }
}

// Bulk read
void ds1307_read_ram_bulk( void* read_data, uint8_t size )
{
    if( size > 56 )
    {
       return;
    }
    else
    {

        uint8_t* p_data = ( uint8_t* )read_data;
        uint8_t start_addr = 0x08;

        TWI_Start();
        TWI_Write( DS1307_WRITE );
        TWI_Write( start_addr );
        TWI_Start();
        TWI_Write( DS1307_READ );

        while( size-- > 1 )
        {
            *p_data++ = TWI_Read( DS1307_ACK );
        }

        *p_data = TWI_Read( DS1307_NO_ACK );
        TWI_Stop();
    }
}

uint8_t ds1307_read_address( uint8_t address )
{
     static uint8_t cfg = 0;
     
     TWI_Start();
     TWI_Write( DS1307_WRITE );
     TWI_Write( address );
     TWI_Start();
     TWI_Write( DS1307_READ );
     cfg = TWI_Read( DS1307_NO_ACK );
     TWI_Stop();

     return cfg;
}


char* ds1307_get_http_gmt_str()
{
    //Tue, 15 Nov 1994 08:12:31 GMT
    TimeStruct* ts1;
    static char gmt_str[30];

    ts1 = ds1307_get_GMT_time();

    gmt_str[0] = days_week[ts1->wd][0];
    gmt_str[1] = days_week[ts1->wd][1];
    gmt_str[2] = days_week[ts1->wd][2];
    gmt_str[3] = ',';
    gmt_str[4] = ' ';
    gmt_str[5] = ( ts1->md / 10 ) + 48;
    gmt_str[6] = ( ts1->md % 10 ) + 48;
    gmt_str[7] = ' ';
    gmt_str[8]  = months_year[ts1->mo][0];
    gmt_str[9]  = months_year[ts1->mo][1];
    gmt_str[10] = months_year[ts1->mo][2];
    gmt_str[11] = ' ';
    gmt_str[12] = ( ts1->yy / 1000 ) + 48;
    gmt_str[13] = ( ( ts1->yy % 1000 ) / 100 ) + 48;
    gmt_str[14] = ( ( ts1->yy % 100 ) / 10 ) + 48;
    gmt_str[15] = ( ts1->yy % 10 ) + 48;
    gmt_str[16] = ' ';
    gmt_str[17] = ( ts1->hh / 10 ) + 48;
    gmt_str[18] = ( ts1->hh % 10 ) + 48;
    gmt_str[19] = ':';
    gmt_str[20] = ( ts1->mn / 10 ) + 48;
    gmt_str[21] = ( ts1->mn % 10 ) + 48;
    gmt_str[22] = ':';
    gmt_str[23] = ( ts1->ss / 10 ) + 48;
    gmt_str[24] = ( ts1->ss % 10 ) + 48;
    gmt_str[25] = ' ';
    gmt_str[26] = 'G';
    gmt_str[27] = 'M';
    gmt_str[28] = 'T';
    gmt_str[29] = '\0';

    return gmt_str;
}
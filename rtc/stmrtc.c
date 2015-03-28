#include "stmrtc.h"

#define AMPM       0x400000
#define HOUR       0x3f0000
#define MINUTE     0x007f00
#define SECOND     0x0000ff

#define YEAR       0xff0000
#define WEEK_DAY   0x00e000
#define MONTH      0x001f00
#define MONTH_DAY  0x00003f

static const code uint8_t* days_week[] = {"Nul","Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
static const code uint8_t* months_year[] = {"Nul","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

static TimeStruct time_GMT, time_local;
static TimeStruct dst_time_start, dst_time_stop;

static int8_t local_time_zone;
static bool dst_enable;

/****************************************
 *  Helper Prototypes
 ***************************************/
static void RTC_EnterConfigMode( void );
static void RTC_ExitConfigMode( void );


static void RTC_EnterConfigMode()
{
    PWR_CR.DBP = 1;

    RTC_WPR = 0xCA;                   //unlock write protection
    RTC_WPR = 0x53;                   //unlock write protection
    RTC_ISR = 0x00000080;             //enter initialization mode  bit 7

    while( RTC_ISR.INITF != 1 ) {};   // bit 6
}

static void RTC_ExitConfigMode()
{
    RTC_ISR = 0x00000000;             //exit initialization mode
    RTC_WPR = 0xFF;                   // Enable the write protection for RTC registers
}

/****************************************
 *  Functions
 ***************************************/
/*!
 *  \brief Initializes rtc
 *
 */
void stmrtc_init(  stmrtc_config_t* rtc_config  )
{
    local_time_zone = rtc_config->time_zone;
    dst_enable      = rtc_config->dst_enable;
    dst_time_start  = rtc_config->dst_begin;
    dst_time_stop   = rtc_config->dst_end;

    RCC_APB1ENR.PWREN = 1;            // Enable RTC clock

    if ( RCC_BDCR.RTCEN == 0 )        // if RTC is disabled ...
    {
        PWR_CR.DBP = 1;               // Allow access to RTC

        RTC_WPR  = 0xCA;              //unlock write protection
        RTC_WPR  = 0x53;              //unlock write protection

        RCC_BDCR = 0x00010000;        // make it possible to change clock source
        RCC_BDCR = 0x00008101;        // RTCEN = 1, LSE, LSEON
    }

    while( RTC_ISR.RSF != 1 ) {}      // Wait for RTC APB registers synchronisation

    while( RCC_BDCR.LSERDY != 1 ) {}; // Wait till LSE is ready

    RTC_WPR = 0xFF;

}

/*!
 *  \brief Sets RTC based on a GMT unix timestamp
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void stmrtc_set_time_GMT( int32_t GMTTime )
{
    Time_epochToDate( GMTTime, &time_GMT );

    stmrtc_set_time( &time_GMT );

}

/*!
 *  \brief Sets Time of RTC
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param Timestruct - set_time
 *
 */
void stmrtc_set_time( TimeStruct* set_time )
{
    uint32_t temp;

    RTC_EnterConfigMode();

    temp  = ( ( uint32_t )Dec2Bcd( set_time->hh ) << 16 ) & HOUR;
    temp += ( ( uint32_t )Dec2Bcd( set_time->mn ) << 8 )  & MINUTE;
    temp += ( uint32_t )Dec2Bcd( set_time->ss )           & SECOND;
    temp &= ~( 1 << 22 );

    RTC_TR = temp;

    temp  = ( ( uint32_t )Dec2Bcd( ( set_time->yy - 2000 ) ) << 16 ) & YEAR;
    temp += ( ( uint32_t )Dec2Bcd( ( set_time->wd + 1 ) ) << 13 ) & WEEK_DAY;
    temp += ( ( uint32_t )Dec2Bcd( set_time->mo ) << 8 ) & MONTH;
    temp += ( uint32_t )Dec2Bcd( set_time->md ) & MONTH_DAY;

    RTC_DR = temp;           // set date

    RTC_ExitConfigMode();
}

/*!
 *  \brief <Basic Description>
 *
 *  \note
 *   <notes>
 */
TimeStruct* stmrtc_get_GMT_time()
{
    uint32_t rtc_time;

    rtc_time = RTC_TR;

    time_GMT.hh = Bcd2Dec( ( uint8_t )( ( rtc_time & HOUR ) >> 16 ) );
    time_GMT.mn = Bcd2Dec( ( uint8_t )( ( rtc_time & MINUTE ) >> 8 ) );
    time_GMT.ss = Bcd2Dec( ( uint8_t )( rtc_time & SECOND ) );

    rtc_time = RTC_DR;

    time_GMT.yy  = ( ( uint16_t )Bcd2Dec( ( uint8_t )( ( rtc_time & YEAR ) >> 16 ) ) ) + 2000;
    time_GMT.wd  = Bcd2Dec( ( uint8_t )( ( rtc_time & WEEK_DAY ) >> 13 ) );
    time_GMT.mo  = Bcd2Dec( ( uint8_t )( ( rtc_time & MONTH ) >> 8 ) );
    time_GMT.md  = Bcd2Dec( ( uint8_t )( rtc_time & MONTH_DAY ) );

    return &time_GMT;
}

/*!
 *  \brief Gets local time
 *
 *  \note
 *   <notes>
 */
TimeStruct* stmrtc_get_local_time()
{
    uint32_t rtc_time = stmrtc_get_GMT_unix_time();
    uint8_t new_time_zone = local_time_zone;
    
    if( dst_enable == TRUE )
    {
        if( time_local.mo >= dst_time_start.mo && time_local.mo <= dst_time_stop.mo )
        {
            if( time_local.mo == dst_time_start.mo )
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
            else if( time_local.mo == dst_time_stop.mo )
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
    }

    rtc_time += ( new_time_zone * 60 * 60 );

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
int32_t stmrtc_get_GMT_unix_time()
{
    TimeStruct* ts2 = stmrtc_get_GMT_time();
    int32_t ret_time = Time_dateToEpoch( ts2 );

    return ret_time;

    //return Time_dateToEpoch( stmrtc_get_GMT_time() );
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
int32_t stmrtc_get_local_unix_time()
{
    return Time_dateToEpoch( stmrtc_get_local_time() );
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
char* stmrtc_get_local_time_str( int mode )
{
    static char formated_local_time[20];
    TimeStruct* tmp_time;

    tmp_time = stmrtc_get_local_time();

    if( mode==MODE12HOUR )
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
char* stmrtc_get_GMT_time_str( int mode )
{
    static char formated_gmt_time[20];
    TimeStruct* tmp_time;

    tmp_time = stmrtc_get_GMT_time();

    if( mode==MODE12HOUR )
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

char* stmrtc_get_system_time_str()
{
    return stmrtc_get_local_time_str( MODE24HOUR );
}

char* stmrtc_get_http_gmt_str()
{
    //Tue, 15 Nov 1994 08:12:31 GMT
    TimeStruct* ts1;
    static char gmt_str[30];

    ts1 = stmrtc_get_GMT_time();

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
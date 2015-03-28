// System Includes
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include "main.h"

// Compilied Libraries
#include "scheduler.h"

// Settings files
#include "settings_manager.h"

// Network files
#include "network_conf.h"
#include "network.h"
#include "page_manager.h"
#include "protocols/http_server/http_server.h"
#include "protocols/http_client/http_client.h"
#include "protocols/udp_ntp/udp_ntp.h"
#include "head_generator/head_generator.h"

// System files
#include "logger.h"
#include "http.h"
//#include "/resources/http_ajax.h"
#include "boardConf.h"
#include "ds1307.h"
//#include "ds18x20.h"
#include "__Lib_FAT32.h"

// System Timer Delay
#define SYSTEM_CLEANUP_DELAY 10

/**************************************
     prototypes
**************************************/
// Tasks
static void task_system_beat   ( void );
static void task_get_tide_data ( void );
static void task_maintanence   ( void );
static void task_check_alarms  ( void );

// Function prototypes
static void system_init        ( void );

// Handlers from callbacks
void ntp_handler ( uint32_t time );
void file_handler( char* file_name );
void form_handler( char* form_data, char* file_name );
void post_handler( char* post_data, char* file_name );

/************************************
 *  Global Variables
 ***********************************/

const unsigned long HEAP_SIZE = 4096; // Memory manager required

static bool file_download_flag     = FALSE;  // File downloaded signal flag
static bool filesystem_change_flag = FALSE;  // EEPROM needed to be saved

static char file_downloaded[ FILE_NAME_SIZE ];


/*************************************
 *   Task Functions
*************************************/
/*!
 *  \brief Task - Clean up open/un-used sockets, runs every 10 seconds
 *
 *  Several things happen with this task.  Network cleanup will
 *  close any open sockets.  It also increments the FAT32 file
 *  system, checks the system flags for any changes that might need
 *  to be saved.  If a file system change ( SD Card changed ) occurs
 *  then the file system is re-initialized.  If a new file is downloaded
 *  then a callback is called to process the file.
 *
 */
static void task_system_beat()
{
    // Cleans up un-used sockets
    network_cleanup();
    // Increments the filesystem timestamp
    FAT32_IncTime( SYSTEM_CLEANUP_DELAY );
    // Checks for any new data that needs to be stored
    settings_check_data();
    
    // Card changed detected
    if( filesystem_change_flag == TRUE )
    {
        configure_file_system();
        filesystem_change_flag = FALSE;
    }
    
    // Download callback has file to parse
    if( file_download_flag == TRUE )
    {
        tide_process_data( file_downloaded );
        file_download_flag = FALSE;
        log_event( LOG_NORMAL, "Successful File Download" );
    }
}

/*!
 *  \brief Task - Get New Tide Data
 *
 *  Task first retrieves the tide settings from EEPROM.  Then requests
 *  a new file download by passing the tide station and port number to
 *  the http_client_request_file function.
 */
static void task_get_tide_data()
{
    tide_settings_t* tide = settings_get_tide();
    http_client_request_file( tide_request_data( tide->station_id ), 80 );
}

/*!
 *  \brief Task - Check for Alarms
 *
 *  Reads scratch ram on ds1307, compare with time.  If the alarm time
 *  is after the current time then a pump cycle will be ran.
 *
 */
static void task_check_alarms()
{
    uint32_t alarm_time, current_time;
    ds1307_read_ram_bulk( ( void* )&alarm_time, sizeof( alarm_time ) );
    current_time = ds1307_get_GMT_unix_time();
    /* TODO: Need to compare current time to alarm and determine
       if we need to run a pump alarm
    */
}

/*!
 *  \brief Task - Perform system maintanence
 *
 *  This function starts by retrieving the settings from EEPROM.
 *  Using the NTP server saving in settings it will request a new
 *  NTP time sync.
 *
 */
static void task_maintanence()
{
    network_settings_t* p_net = settings_get_network();
    udp_request_NTP( p_net->time_server );
    // TODO: Filesystem maintanence needs to be added
}

/**************************************
 *   Webserver Callbacks
 *************************************/
/*!
 *  \brief ntp handler
 *
 *  Handles a NTP response returned from the server module.  What is passed
 *  to it is a 32bit timestamp.
 *
 *  \param: uint32_t* time - pointer to unix timestamp returned from NTP svr
 *
 */
void ntp_handler( uint32_t* time )
{
    // If the timestamp from the RTC is different from the NTP - resync
    if( *time != ds1307_get_GMT_unix_time() )
    {
        TimeStruct* tm1;  // Timestruct used by time library
        __TIME ft;        // Timestruct used by FAT32 library
        
        ds1307_set_time_GMT( *time );  // Resync RTC
        log_event( LOG_NORMAL, "Time was successfully updated with NTP" );
        
        tm1 = ds1307_get_local_time(); // Get local time from RTC for FAT32
        
        ft.Second = tm1->ss;
        ft.Minute = tm1->mn;
        ft.Hour   = tm1->hh;
        ft.Day    = tm1->md;
        ft.Month  = tm1->mo;
        ft.Year   = tm1->yy;
        
        // Re-synces filesystem to new returned timestamp
        FAT32_SetTime( &ft );
    }
}

/*!
 *  \brief File download handler
 *
 *  Handler that handles a newly downloaded file from a file request.
 *
 *  \param char* file_name - file name of the file downloaded from the svr
 *
 */
void file_handler( char* file_name )
{
    strcpy( file_downloaded, file_name ); // String is copied to global
    file_download_flag = TRUE;            // Set a flag to be handled
}

/*!
 *  \brief POST request handler
 *
 *  Handles a POST request from a client.  If the client POST is a valid
 *  file in the http pages, then data is handled.
 *
 *  \param char* post_data - Data on the POST request
 *  \param char* file_name - Page name data is to be modified on
 *
 */
void post_handler( char* post_data, char* file_name )
{
   if( strcmp( file_name, "tide.htm" ) == 0 )
   {
       tide_parse_form_values( post_data );
   }
   else if( strcmp( file_name, "admin.htm" ) == 0 )
   {
   
   }
   // TODO: Provide methods that will be used for post requests
}

/*!
 *  \brief GET request handler
 *
 *  Handles a GET request from a client.  If the client GET is a valid
 *  file in the http pages, then the data is handled.
 *
 *  \param char* form_data - Data on the GET request
 *  \param char* file_name - Page name data is to be modified on
 *
 */
void form_handler( char* form_data, char* file_name )
{
   #ifdef DEBUG
   printf( "Form: %s\r\nForm Data: %s\r\n", file_name, form_data );
   #endif
   // TODO: Handle form data from GET request
}


/**************************************
     Functions
**************************************/
/*!
 * \brief Initializes system modules
 *
 */
static void system_init()
{
    #ifdef DEBUG
    configure_terminal();   // Starts UART communications
    #endif
    configure_board();      // Starts bus and attached systems
    //configure_relays();     // Relays and motors
    configure_timers();     // Timers used for capture
    configure_rtc();        // Realtime clock
    
    // File system
    if( configure_file_system() >= 0 )
    {
        log_init( ds1307_get_system_time_str );
    }
    else
    {
        printf( "Filesystem Failed\r\n" );
    }
    
    // Webserver
    configure_webserver();
    
    // Callbacks
    http_client_set_file_callback( file_handler );
    http_set_form_callback       ( form_handler );
    http_set_post_callback       ( post_handler );
    udp_set_ntp_callback         ( ntp_handler );
    header_set_GMT_callback      ( ds1307_get_http_gmt_str );
    
    // Pages added to catalog
    page_manager_add_staticpage ( index_htm,    "index.htm",   sizeof( index_htm   ) );
    page_manager_add_staticpage ( admin_htm,    "admin.htm",   sizeof( admin_htm   ) );
    page_manager_add_staticpage ( logs_htm,     "logs.htm",    sizeof( logs_htm    ) );
    page_manager_add_staticpage ( pageCSS_css,  "pageCSS.css", sizeof( pageCSS_css ) );
    page_manager_add_staticpage ( tides_htm,    "tides.htm",   sizeof( tides_htm   ) );

    page_manager_add_flashpage  ( "error.txt",    TRUE );
    page_manager_add_flashpage  ( "system.txt",   TRUE );
    page_manager_add_dynamicpage( tide_get_JSdata(),     "tide.js",    tide_generate_data );
    page_manager_add_dynamicpage( pump_get_dynamic_data(),     "dynamic.js", pump_generate_data );
    page_manager_add_dynamicpage( settings_get_JSdata(), "set.js",     settings_generate_data );
    
    // Init scheduler with 1000 ms clock
    task_scheduler_init( 1000 );
    // Runs every 10 seconds - Normal system routines
    task_add( 0, task_system_beat,   SYSTEM_CLEANUP_DELAY );
    // Runs every 12 hours - Requests new NOAA data from remote server
    task_add( 1, task_get_tide_data, SCH_DAY_1 );
    // Runs every 15 minutes - Checks for tide schedule that needs to be ran
    task_add( 2, task_check_alarms,  SCH_MINUTES_15 );
    // Every 24 hours - Runs filesystem checks and resets all sockets
    task_add( 3, task_maintanence,   SCH_DAY_1 );
}

/*!
 *  \brief <Basic Description>
 *
 */
void main()
{
    Delay_us( 120 );         // Allow main clock to stabilize

    system_init();           // Initialize all systems
    task_scheduler_start();  // Starts the task scheduler

    #ifdef DEBUG
    tide_process_data("SOS_HIGH.CSV");
    #endif

    asm sei;                 // enable all interrupts

    while( 1 )
    {
        network_read_packet(); // Reads packets, if available, on network
        task_dispatch();       // Scheduler launcher

        #ifdef DEBUG
        if( UART1_Data_Ready() )
        {
            switch( UART1_Read() )
            {
                case 't':
                    task_maintanence();
                    break;
                case 'd':
                    task_get_tide_data();
                    break;
                case 'p':
                    FAT32_Dir();
                    break;
                default:
                    printf( "Bad command\r\n" );
            }
        }
        #endif
    }
}

/*************************************
 * ISRs
 ************************************/
// Task Timer
void task_clock_ISR() iv IVT_ADDR_INT0 ics ICS_AUTO
{
    task_scheduler_clock();
    network_time_update();
}

// File system
void card_change_ISR() iv IVT_ADDR_INT2 ics ICS_AUTO
{
    filesystem_change_flag = TRUE;
}

/******************************************************/
#include "telnet.h"
#include <stddef.h>

#include "ds1307.h"
#include "socket_manager.h"
#include "page_manager.h"

#ifdef NETWORK_USE_LOGGER
#include "logger.h"
#else
#define log_event( event, text ) asm nop
#endif

/***************************
 *  Private Prototypes
 **************************/
// Command functions
static void telnet_cmd_display_tcp_connections( socket_t* const socket );
static void telnet_cmd_get_free_memory( socket_t* const socket );
static void telnet_cmd_get_system_time( socket_t* const socket );
static void telnet_cmd_list_log( socket_t* const socket );
static void telnet_cmd_error_log( socket_t* const socket );
static void telnet_cmd_get_webpages( socket_t* const socket );
static void telnet_cmd_invalid_command( socket_t* const socket );


static void telnet_get_response( char* commands );
static void telnet_send_response( socket_t* const socket );

/***************************
 *  Globals
 **************************/
/** Welcome message to send to a TELNET client when a connection is first made. */
code const char welcome_header[] = 
"********************************************\r\n\
*       AlphaLoewe Webserver (TELNET)      *\r\n\
********************************************\r\n";

/** Main TELNET menu, giving the user the list of available commands they may issue */
code const char telnet_menu[] = 
"\r\n\
== Available Commands: ==\r\n\
c) List Active TCP Connections\r\n\
m) Free System Memory\r\n\
t) System Time\r\n\
a) Tide Alarms\r\n\
l) System log\r\n\
e) Error log\r\n\
w) Active Webpages\r\n\
x) Close Connection\r\n\
=========================\r\n\
\r\n>";

code const char goodbye_msg[] = 
"  Auf Wiedersehen  ";

/** Header to print before the current connections are printed to the client */
code const char CurrentConnectionsHeader[] = "\r\n* Current TCP Connections: *\r\n";

void ( *action_command )( socket_t* const socket ) = NULL;

/****************************
 *  Private Functions
 ***************************/
/** Sends a list of active TCP connections to the TELNET client. */
static void telnet_cmd_display_tcp_connections( socket_t* const socket )
{


}


static void telnet_cmd_get_free_memory( socket_t* const socket )
{
    char txt_buffer[12];
    
    LongToStr( MM_TotalFreeMemSize(), txt_buffer );
    strcpy( socket->buffer_large, "Total Free Memory: " );
    strcat( socket->buffer_large, Ltrim( txt_buffer ) );
    strcat( socket->buffer_large, " KB\r\n" );
}



static void telnet_cmd_get_system_time( socket_t* const socket )
{
     strcpy( socket->buffer_large, ds1307_get_system_time_str() );
}


static void telnet_cmd_list_log( socket_t* const socket )
{
    __HANDLE file_handle = FAT32_Open( "SYSTEM.TXT", FILE_READ );
    
    if( file_handle < 0 )
    {
        strcpy( socket->buffer_large, "Could not open file\r\n" );
    }
    else
    {
        uint32_t size = 0;
        
        FAT32_Size( "SYSTEM.TXT", &size );
        
        if( size > LARGE_BUFFER_SIZE )
        {
           FAT32_Seek( file_handle, size - LARGE_BUFFER_SIZE );
        }
        
        FAT32_Read( file_handle, socket->buffer_large, LARGE_BUFFER_SIZE );
        FAT32_Close( file_handle );
    }
}

static void telnet_cmd_error_log( socket_t* const socket )
{
     __HANDLE file_handle = FAT32_Open( "ERROR.TXT", FILE_READ );

    if( file_handle < 0 )
    {
        strcpy( socket->buffer_large, "Could not open file\r\n" );
    }
    else
    {
        uint32_t size = 0;
        
        FAT32_Size( "ERROR.TXT", &size );

        if( size > LARGE_BUFFER_SIZE )
        {
           FAT32_Seek( file_handle, size - LARGE_BUFFER_SIZE );
        }

        FAT32_Read( file_handle, socket->buffer_large, LARGE_BUFFER_SIZE );
        FAT32_Close( file_handle );
    }

}

static void telnet_cmd_get_webpages( socket_t* const socket )
{
    strncpy( socket->buffer_large, page_manager_print_page_list(), 256 );
}


static void telnet_cmd_invalid_command( socket_t* const socket )
{
    strcpy( socket->buffer_large, "Invalid Command\r\n" );
}



static void telnet_get_response( char* commands )
{
   switch( commands[0] )
   {
      case 'm':
          action_command = telnet_cmd_get_free_memory;
      break;
      case 'c':
          action_command = telnet_cmd_display_tcp_connections;
      break;
      case 'l':
          action_command = telnet_cmd_list_log;
      break;
      case 'x':
          action_command = close_telnet_socket;
      break;
      case 'e':
          action_command = telnet_cmd_error_log;
      break;
      case 't':
          action_command = telnet_cmd_get_system_time;
      break;
      case 'w':
          action_command = telnet_cmd_get_webpages;
      break;
      default:
          action_command = telnet_cmd_invalid_command;
   }
}

static void telnet_send_response( socket_t* const socket )
{
    Net_Ethernet_28j60_putStringTCP( socket->buffer_large, socket->eth_socket );
}


/****************************
 *  Public Functions
 ***************************/
/** Initialization function for the simple TELNET webserver. */
void telnet_init( void )
{
    /* Listen on port 23 for TELNET connections from hosts */


}


void close_telnet_socket( socket_t* const socket )
{
    Net_Ethernet_28j60_putConstStringTCP( goodbye_msg, socket->eth_socket );
    log_event( LOG_NORMAL, "Ended Telnet Session" );
    Net_Ethernet_28j60_disconnectTCP( socket->eth_socket );

    socket_manager_close( socket );
}


int telnet_handle_request( SOCKET_28j60_Dsc* eth_socket )
{
     socket_t* telnet_socket = socket_manager_find_socket( eth_socket );
     
     if( telnet_socket == NULL )
     {
         return -1;
     }

     telnet_socket->alive_time = 0;
     
     switch( telnet_socket->socket_state )
     {
          case SM_HTTP_IDLE:
              log_event( LOG_NORMAL, "Started telnet session" );
              telnet_socket->socket_state = SM_TELNET_SEND_HEADER;
          case SM_TELNET_SEND_HEADER:
              Net_Ethernet_28j60_putConstStringTCP( welcome_header, telnet_socket->eth_socket );
              telnet_socket->socket_state = SM_TELNET_SEND_MENU;
          case SM_TELNET_SEND_MENU:
              Net_Ethernet_28j60_putConstStringTCP( telnet_menu, telnet_socket->eth_socket );
              telnet_socket->socket_state = SM_TELNET_GET_COMMAND;
              break;
          case SM_TELNET_GET_COMMAND:
              if( telnet_socket->eth_socket->dataLength == 0 )
              {
                  break;
              }
              else
              {
                   char command[15];
                   int len = telnet_socket->eth_socket->dataLength;
                   
                   Net_Ethernet_28j60_getBytes( command, 0xffff, len );
                   
                   telnet_get_response( command );
                   telnet_socket->socket_state = SM_TELNET_SEND_RESPONSE;

              }
          case SM_TELNET_SEND_RESPONSE:
              action_command( telnet_socket );
              telnet_send_response( telnet_socket );
              telnet_socket->socket_state = SM_TELNET_SEND_MENU;
              break;
     }
}
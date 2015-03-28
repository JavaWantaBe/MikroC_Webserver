#include "__Lib_FAT32.h"
#include "logger.h"
#include <stddef.h>

/***********************
 *   Variables
 **********************/
static char* ( *get_time_string )( void ) = NULL;

/*******************************
 *  Functions
 ******************************/
/*!
 *  \brief Logger that writes ERROR messages to file
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 */
void log_event( log_event_t log, char* text )
{
        __HANDLE log_handle;

        switch ( log )
        {
            case LOG_NORMAL:
                log_handle = FAT32_Open( "SYSTEM.TXT",  FILE_APPEND );
                break;

            case LOG_ERROR:
                log_handle = FAT32_Open( "ERROR.TXT",   FILE_APPEND );
                break;

            case LOG_UNKNOWN:
                log_handle = FAT32_Open( "UNKNOWN.TXT", FILE_APPEND );
                break;
        }

        if( log_handle < 0 )
        {
            return;
        }

        else
        {
            char tmTxt[256];

            // Get the formatted string date and string time
            if( get_time_string != NULL )
            {   
                strcpy( tmTxt, get_time_string() );
            }
            else
            {
                strcpy( tmTxt, "No time" );
            }
            
            strcat( tmTxt, " - " );
            strcat( tmTxt, text );
            strcat( tmTxt, "\r\n" );

            FAT32_Write( log_handle, tmTxt, strlen( tmTxt ) );
            FAT32_Close( log_handle );
        }

}

/*!
 *  \brief MMC Card Initialization
 *
 */
void log_init( char*( *local_time_string )( void ) )
{
    get_time_string = local_time_string;   // Function pointer to time string

}
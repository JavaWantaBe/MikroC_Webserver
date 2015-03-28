/*!
 * \file
 *
 * \brief Logger that records events to SD media
 *
 * \copyright:
 *   (c) Richard Lowe, 1/25/2014
 * \version Revision History:
 *   1.0 - Initial release
 *
 * Status:
 *   90% completed.
 *
 * Test configuration:
 *   MCU:             ATMega1284P
 *   Dev.Board:       AVRPLC16
 *   Oscillator:      8Mhz
 *   Ext. Modules:    x
 *   SW:              MikroC 6.0
 *
 * \note
 *   <all that matters>
 */


#ifndef LOGGER_H
#define LOGGER_H

/*!
 *   \enum error types that the logger accepts
 *
 */
typedef enum
{
    LOG_NORMAL = 0, /*!< Normal events that might want to be monitored */
    LOG_ERROR,      /*!< Error occurances */
    LOG_UNKNOWN     /*!< Unknown events */

} log_event_t;

/*!
 *  \brief Log and event to the system
 *
 *  \pre FAT32 filesystem needs to be successfully initialized
 *
 *  \param log_event_t log - type of message to log
 *  \param char * text - string of the message
 *
 */
void log_event( log_event_t log, char* text );

/*!
 *  \brief Initializes the Logger
 *
 *  This requires a callback that will return a local time.
 *  Time returned will be added to the record of when the event
 *  happened.
 *
 *  \param char* ( *local_time_string )( void ) - function pointer to time
 *
 */
void log_init( char*( *local_time_string )( void ) );

#endif
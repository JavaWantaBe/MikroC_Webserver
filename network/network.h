/*!
 * \file webserver.h
 *
 * \brief Webserver
 *
 * \copyright:
 *   (c) Richard Lowe, 12/23/2012
 * Revision History:
 *   .01 - Working
 *   .02 - Added time request function
 *   .03 - Added number of free sockets function
 *   .04 - Changed method of closing sockets, added DHCP configuration as well as renew functions
 *   .05 - Added rotating sockets contained in array
 *   .06 - Added support for client requests
 *   .07 - Defined socket structure
 *   .08 - Added liked list and dynamic memory for better management of aging sockets
 *   .9  - Implimentation of file download
 *   .95 - Added file add functions
 *
 * Status:
 *   95% completed.
 *
 * \note
 * Test configuration:
 *   MCU:             ATMega1284p
 *   Dev.Board:       AVRPLC16
 *   Oscillator:      8Mhz
 *   Ext. Modules:    x
 *   SW:              MikroC 5.8
 *
 */

/*!
* \defgroup server_group Web Server
*    The AlphaLoewe web server is a full-featured HTTP 1.1 compatible TCP server intended for 8-bit microcontrollers.
*    While it is capable of supporting various file types such as: HTML, JPEG, GIF, JavaScript, Java applets, flash, and CGI.
*    It is not recommended for any of these files to be very large in size. This is due to the 8-bit nature and speed of a microcontroller
*    that is capable to run on AA batteries. Therefore, it is recommended that site support be limited to basic functionality.
*
*    Integrated into the Web server is basic authentication, so that before any webpages are provided the user must first authenticate
*    against a saved username and password. This feature, and many more are available to be configured at the user's discretion.
*
*  @{
*/


#ifndef _NETWORK_H
#define _NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include "__NetEthEnc28j60.h"

/********************************
     Globals
********************************/
// Configuration of server
typedef struct
{
    char myMacAddr[6];    // MAC address
    char myIpAddr [4];    // IP address
    char gwIpAddr [4];    // gateway (router) IP address
    char ipMask   [4];    // network mask (for example : 255.255.255.0)
    char dnsIpAddr[4];    // DNS server IP address
    bool DHCP_Mode;
    uint16_t port;        // HTTP Port the server will use

} network_config_t;

/*****************************
     Prototypes
*****************************/
/*!
 * \brief Initializes webserver
 *
 * \param int mode - Interrupts enabled or disabled
 */
int network_init( network_config_t* config );

/*!
 *  \brief Checks the ethernet registry for available packets to process.
 *
 *  \note
 *   This function can either be polled were driven by interrupt based on the initialization of the Web server.
 */
void network_read_packet( void );

/*!
 *  \brief Searches through working sockets and finds old or unused sockets.
 *
 *  \note
 *   This function should be called periodically to clean up rogue sockets that may
 *   have been disconnected but left file handles and or memory allocation behind.
 */
void network_cleanup( void );

/*!
 *  \brief Updates timer for DHCP and DNS functions as well as timestamp for GMT string
 *
 *  \note
 *  This function needs to be called every 1 second
 */
void network_time_update( void );

// Additional AVR functions not included with library
void Net_Ethernet_28j60_Reset( void );

// Gets buffer free size
uint16_t Net_Ethernet_28j60_bufferFreeSize( SOCKET_28j60_Dsc* socket_Dsc );

#endif
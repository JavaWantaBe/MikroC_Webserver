#ifndef _UDP_NTP_H
#define _UDP_NTP_H

#include "../../network_conf.h"
#include "../../__NetEthEnc28j60.h"
#include <stdint.h>


/*!
 *  \brief Handles ntp socket requests
 *
 *  \param UDP_Dsc* upd_socket
 *
 *  \return int
 *    \retval -1 == Error
 *    \retval  0 == OK
 *
 */
int udp_handle_NTP( UDP_28j60_Dsc* upd_socket );

/*!
 *  \brief Retrieves NTP timestamp from remote server
 *
 *  \param unsigned char *ntpSrv - web address of SNTP server e.g. "www.sntp.gov"
 *
 */
int8_t udp_request_NTP( char* const ntpSrv );

/*!
 *  \brief Sets callback when NTP packet received
 *
 *  \param void ( *receive_ntp )( uint32_t* time )
 *
 */
void udp_set_ntp_callback( void ( *receive_ntp )( uint32_t* time ) );


#endif
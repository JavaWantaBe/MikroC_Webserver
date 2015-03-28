#ifndef _HTTP_CLIENT_H
#define _HTTP_CLIENT_H

#include "../../__NetEthEnc28j60.h"
#include "../../socket_manager/socket_manager.h"
#include <stdint.h>


/*!
 *  \brief Handles Remote Server Responses
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int http_client_handle( SOCKET_28j60_Dsc * eth_socket );

/*!
 *  \brief Requests file from remote server
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int http_client_request_file( char* const request, uint16_t remote_port );

/*!
 *  \brief Closes client socket
 *
 */
void close_client_socket( socket_t* const socket );

/*!
 *  \brief Checks for pending file download request
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
void http_client_request_pending( void );

void http_client_set_file_callback( void ( *receive_file )( char* file_name ) );

#endif
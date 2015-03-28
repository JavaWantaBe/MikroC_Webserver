#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include "../../network_conf.h"
#include "../../__NetEthEnc28j60.h"
#include "../../socket_manager/socket_manager.h"

/*!
 *  \brief Handles Client Requests
 *
 *  \param SOCKET_Dsc* eth_socket : Socket referenced by ethernet library
 *
 *  \return int
 *    \retval 0 if no errors
 *    \retval -1 if error occurs
 *
 *  \note
 *   <notes>
 */
int http_handle_client( SOCKET_28j60_Dsc* eth_socket );

/*!
 *  \brief Close server socket
 *
 */
void close_server_socket( socket_t* const socket );

/*!
 *  \brief Sets callback to be called when form data is received
 *
 *  \param void ( *receive_form )( key_value_t* key_pairs )
 *
 *  \note
 *   <notes>
 */
void http_set_form_callback( void ( *receive_form )( char* form_data, char* form_name ) );

/*!
 *  \brief Sets callback to be called when post data is received
 *
 *  \param void ( *receive_post )( char* post, char* file_name )
 *
 *  \note
 *   <notes>
 */
void http_set_post_callback( void ( *received_post)( char* post_data, char* file_name ) );



#endif
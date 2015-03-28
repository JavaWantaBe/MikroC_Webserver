#ifndef _HEAD_GENERATOR_H
#define _HEAD_GENERATOR_H

#include "../network_conf.h"
#include "../socket_manager/socket_manager.h"

/*!
 *  \brief Generates header based on socket types
 *
 *  \param socket_t* socket
 *
 *  \return int
 *    \retval -1 == Error
 *    \retval  0 == OK
 *
 */
int header_generate( socket_t* socket );

/*!
 *  \brief Sets function callback for gmt string
 *
 *  \param char* ( *update_gmt_string ) ( void )
 *
 */
#ifdef NETWORK_USE_GMT_TIMESTAMP
void header_set_GMT_callback( char* ( *update_gmt_string )( void ) );
#endif




#endif
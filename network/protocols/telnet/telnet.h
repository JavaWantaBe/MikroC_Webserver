#ifndef TELNET_H
#define TELNET_H

#include "../../__NetEthEnc28j60.h"
#include "../../socket_manager/socket_manager.h"

/* Function Prototypes: */
int telnet_handle_request( SOCKET_28j60_Dsc* eth_socket );

// Initializes telnet
void telnet_init( void );
void close_telnet_socket( socket_t* const socket );

#endif
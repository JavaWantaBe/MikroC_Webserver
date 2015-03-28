#ifndef _SMTP_H
#define _SMTP_H

#include "../../network_conf.h"
#include NETWORK_HEADER

int smtp_handle_server( SOCKET_Dsc* eth_socket );

#endif
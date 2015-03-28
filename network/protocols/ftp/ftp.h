#ifndef _FTP_H
#define _FTP_H

#include "../../network_conf.h"
#include NETWORK_HEADER

int ftp_handle_request( SOCKET_Dsc* eth_socket );

#endif
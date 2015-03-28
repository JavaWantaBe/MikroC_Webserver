#ifndef _NETWORK_CONF_H
#define _NETWORK_CONF_H

/***************************
 *   Features to include
 *
 *   Uncomment out any
 *   features not to
 *   include.
 **************************/
#define NETWORK_USE_SERVER
#define NETWORK_USE_CLIENT
#define NETWORK_USE_NTP
#define NETWORK_USE_TELNET
//#define NETWORK_USE_MAIL
//#define NETWORK_USE_FTP
#define NETWORK_USE_FAT32
#define NETWORK_USE_GMT_TIMESTAMP
#define NETWORK_USE_LOGGER

/***************************
 *   PORTS
 **************************/
//TCP Ports
#define TCP_PORT_FTP_DATA                  20
#define TCP_PORT_FTP_COMMAND               21
#define TCP_PORT_TELNET                    23
#define TCP_PORT_SMTP                      25
#define TCP_PORT_REQUEST                   10021   /*!< Port that is used for remote server requests*/
#define TCP_PORT_HTTP                      80

//UDP Ports
#define UDP_PORT_SNTP                      123     /*!< Port that is used for SNTP*/

#ifdef NETWORK_USE_SERVER

#endif

// Sizes acceptable
#define FILE_NAME_SIZE                     13
#define DOMAIN_SIZE                        50
#define LARGE_BUFFER_SIZE                  256
#define GET_POST_SIZE                      256
#define MAX_POST_SIZE                      512
#define SWAP_FILE                          "DOWNLOAD.BIN"
#define SECTOR_IN_BYTES                    512
#define MAX_REQUEST_SIZE                   512
#define MAX_FILE_DOWNLOAD_SIZE             1024000

/****************************
 *  Webpages Variables
 ***************************/
#define MAX_PAGES                          25

/****************************
 *   SOCKET Parameters
 ***************************/
#define MAX_ALIVE_TIME                     10
#define CONNECT_ATTEMPTS                   5

#define NUM_OF_SOCKET                      NUM_OF_SOCKET_28j60
#define MY_MSS_SIZE                        128
#define MAX_SOCKET                         4

#endif // End of network configuration
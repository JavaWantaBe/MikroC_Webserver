/*! * \file
 *
 *  Webserver Source File
 *
 *  Webserver hardware is a MicroChip ENC28J60.
 *  Datasheet:  ac:ENC28J60_Ethernet
 *
 */

#include "network.h"
#include "network_conf.h"
#include "/socket_manager/socket_manager.h"
#include "/page_manager/page_manager.h"

#ifdef NETWORK_USE_SERVER
#include "/protocols/http_server/http_server.h"
#endif

#ifdef NETWORK_USE_CLIENT
#include "/protocols/http_client/http_client.h"
#endif

#ifdef NETWORK_USE_NTP
#include "/protocols/udp_ntp/udp_ntp.h"
#endif

#ifdef NETWORK_USE_TELNET
#include "/protocols/telnet/telnet.h"
#endif

#ifdef NETWORK_USE_FTP
#include "/protocols/ftp/ftp.h"
#endif

#ifdef NETWORK_USE_MAIL
#include "/protocols/smtp/smtp.h"
#endif

#ifdef NETWORK_USE_LOGGER
#include "logger.h"
#endif

#ifdef NETWORK_USE_FAT32
#include "__Lib_FAT32.h"
#endif

/*****************************
     Globals / All globals have static duration
*****************************/
// Port variables
static uint16_t http_port = 80;      /*!< Port that HTTP server serves pages */
extern sbit Net_Ethernet_28j60_CS;

/*************************************
 *    Functions
 ************************************/
void Net_Ethernet_28j60_Reset()
{
    Net_Ethernet_28j60_CS = 0;
    SPI1_Write( 0xFF );
    Net_Ethernet_28j60_CS = 1;
}

uint16_t Net_Ethernet_28j60_bufferFreeSize( SOCKET_28j60_Dsc* socket_Dsc )
{
    return ( TCP_TX_SIZE_28j60 - socket_Dsc->lastWritten );
}

/*************************************
 *    Utility Functions
 ************************************/


/************************************
 *
 *     Handlers
 *
 ************************************/
/*!
 *  \brief Handles all TCP packets
 *
 *  \param SOCKET_28j60_Dsc *socket - socket calling from do packet
 *
 */
void Net_Ethernet_28j60_UserTCP( SOCKET_28j60_Dsc* eth_socket )
{
    switch( eth_socket->destPort )
    {
    #ifdef NETWORK_USE_SERVER
        case TCP_PORT_HTTP:
            http_handle_client( eth_socket );
        break;
    #endif
    #ifdef NETWORK_USE_FTP
        case TCP_PORT_FTP_DATA:
            ftp_handle_request( eth_socket );
        break;
        case TCP_PORT_FTP_COMMAND:
            ftp_handle_request( eth_socket );
        break;
    #endif
    #ifdef NETWORK_USE_TELNET
        case TCP_PORT_TELNET:
            telnet_handle_request( eth_socket );
        break;
    #endif
    #ifdef NETWORK_USE_CLIENT
        case TCP_PORT_REQUEST:
            http_client_handle( eth_socket );
        break;
    #endif
    #ifdef NETWORK_USE_MAIL
        case TCP_PORT_SMTP:
            smtp_handle_server( eth_socket );
        break;
    #endif
    }
}

/*!
 *  \brief Handles all UDP packets
 *
 *  \param Struct of UDP socket
 *
 *  \return unsigned int - number of bytes returned
 *
 */
unsigned int Net_Ethernet_28j60_UserUDP( UDP_28j60_Dsc* udpDsc )
{
    unsigned int len = 0;
    
    switch( udpDsc->destPort )
    {
    #ifdef NETWORK_USE_NTP
        // Port 123 is a SNTP request
        case UDP_PORT_SNTP:
            len = udp_handle_NTP( udpDsc );
        break;
    #endif
    }
    return len;
}

/**********************************
 *
 *     Getters
 *
 **********************************/
/*!
 *  \brief Initializes the webserver
 *
 *  \param int mode - Either 0 for no IRQ 1 for IRQ mode
 *
 */
int network_init( network_config_t* config )
{
    http_port = config->port;

    // Initialize TCP stack, must be done first
    Net_Ethernet_28j60_stackInitTCP();
    
    /* UDP port 68 is used as DHCP client port and UDP port 67 is used as DHCP 
    server port. */
    if( config->DHCP_Mode == true )
    {
        char d_IP[4], d_GW[4], d_DNS[4], d_MASK[4];
        
        Net_Ethernet_28j60_initDHCP(5);
        memcpy( d_IP, Net_Ethernet_28j60_getIpAddress(), 4 );
        memcpy( d_GW, Net_Ethernet_28j60_getGwIpAddress(), 4 );
        memcpy( d_DNS, Net_Ethernet_28j60_getDnsIpAddress(), 4 );
        memcpy( d_MASK, Net_Ethernet_28j60_getIpMask(), 4 );

        Net_Ethernet_28j60_Init( config->myMacAddr, d_IP, Net_Ethernet_28j60_FULLDUPLEX );
        Net_Ethernet_28j60_confNetwork( d_MASK, d_GW, d_DNS );
        
    }
    else
    {
        Net_Ethernet_28j60_Init( config->myMacAddr, config->myIpAddr, Net_Ethernet_28j60_FULLDUPLEX );
        Net_Ethernet_28j60_confNetwork( config->ipMask, config->gwIpAddr, config->dnsIpAddr );
    }

    if( ( socket_manager_init() == SOCKET_ERROR ) || ( page_manager_init() == PAGE_ERROR ) )
    {
        return -1;
    }
}

/*
 *   tutorial5solution.htm?check0=on&text0=Richard&check1=on&text1=Lowe&check2=on&text2=ocdrichard%40gmail.com&check3=on&text3=05%2F25%2F1974
 */

/**********************************
 *
 *      Tasks
 *
 **********************************/
/*!
 *  \brief Cleans up closed sockets
 *
 */
void network_cleanup()
{
    socket_manager_cleanup();
}

/*!
 *  \brief Processes incoming packets
 *
 */
void network_read_packet()
{
    Net_Ethernet_28j60_doPacket();
    #ifdef NETWORK_USE_CLIENT
    http_client_request_pending();
    #endif
}


/*!
 *  \brief Updates both the hardware timer as well as GMT timestamp
 *
 */
void network_time_update()
{
    socket_manager_alive_time_update();
    Net_Ethernet_28j60_UserTimerSec++; /*<! Variable used with DHCP, DNS, and ARP functions */
}
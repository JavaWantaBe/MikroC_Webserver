#include "udp_ntp.h"
#include <stddef.h>
#include "built_in.h"

#ifdef NETWORK_USE_LOGGER
#include "logger.h"
#else
#define log_event( event, text ) asm nop
#endif

/*************************
 *   Globals
 ************************/
/*!
 *  \struct Response of SNTP Server
 *
 *  \brief Used to hold the return response from SNTP time server
 *
 */
typedef struct
{
    uint32_t epoch;          // Time stamp from Jan1 1900
    uint8_t serverFlags;
    uint8_t serverStratum;
    int8_t serverPrecision;

} SNTP_Response_t;

// Callback function pointer
static void ( * received_ntp_callback )( uint32_t* time ) = NULL;

// Variables
static uint32_t current_time;

/*******************************
 *   Public Functions
 ******************************/
void udp_set_ntp_callback( void ( *receive_ntp )( uint32_t* time ) )
{
    received_ntp_callback = receive_ntp;
}

// Retrieves SNTP timestamp from input
int8_t udp_request_NTP( char* const ntpSrv )
{
    char* p_remoteIP = NULL;
    int8_t tries = CONNECT_ATTEMPTS;

    do
    {
        p_remoteIP = Net_Ethernet_28j60_dnsResolve( ntpSrv, 5 );
    }
    while( ( p_remoteIP == NULL ) && ( tries-- > 0 ) );

    if( tries > 0 )
    {
        uint8_t remoteIpAddr[4];
        uint8_t sntpPkt[48];

        memcpy( remoteIpAddr, p_remoteIP, 4 );
        // FLAGS : byte 0
        sntpPkt[0] = 0b00011001; // STRATUM : byte 1 = 0 - LI = 0 ; VN = 3 ; MODE = 1
        sntpPkt[1] = 0x00;       // POLL : byte 2
        sntpPkt[2] = 0x0a;   // PRECISION : byte 3 - 1024 sec (arbitrary value)
        sntpPkt[3] = 0xfa;             // 0.015625 sec (arbitrary value)
        // DELAY : bytes 4 to 7 = 0.2656 sec (arbitrary value)
        sntpPkt[6] = 0x44 ;
        // DISPERSION : bytes 8 to 11 = 16 sec (arbitrary value)
        sntpPkt[9] = 0x10 ;
        // REFERENCE ID : bytes 12 to 15 = 0 (unspecified)
        // REFERENCE TIMESTAMP : bytes 16 to 23 (unspecified)
        // ORIGINATE TIMESTAMP : bytes 24 to 31 (unspecified)
        // RECEIVE TIMESTAMP : bytes 32 to 39 (unspecified)
        // TRANSMIT TIMESTAMP : bytes 40 to 47 (unspecified)

        tries = CONNECT_ATTEMPTS;

        while( ( Net_Ethernet_28j60_sendUDP( remoteIpAddr, UDP_PORT_SNTP, 
                                             UDP_PORT_SNTP, sntpPkt, 48 ) != 1 )
                                             && ( tries-- > 0 ) );

        if( tries == 0 )
        {
            return -1;
        }

        return 0;
    }

    else
    {
        return -1;
    }
}

// Handles socket requests on NTP port
int udp_handle_NTP( UDP_28j60_Dsc* upd_socket )
{
    SNTP_Response_t tmResp; //!< Holds packet framework for SNTP servers
    uint32_t tts = 0;       //!< Holds the time returned from the remote SNTP
    int ntpCount;

    tmResp.serverFlags = Net_Ethernet_28j60_getByte();
    tmResp.serverStratum = Net_Ethernet_28j60_getByte();
    // skip poll
    Net_Ethernet_28j60_getByte();
    tmResp.serverPrecision = Net_Ethernet_28j60_getByte();

    for( ntpCount = 0 ; ntpCount < 36 ; ntpCount++ )
    {
        Net_Ethernet_28j60_getByte();
    }

    // store transmit timestamp
    Highest( tts ) = Net_Ethernet_28j60_getByte();
    Higher( tts )  = Net_Ethernet_28j60_getByte();
    Hi( tts )      = Net_Ethernet_28j60_getByte();
    Lo( tts )      = Net_Ethernet_28j60_getByte();
    // convert sntp timestamp to unix epoch
    // The large number is the difference between 1900 and 1970
    tmResp.epoch = ( tts - 2208988800 );
    current_time = tmResp.epoch;
    received_ntp_callback( &current_time );

    return ( upd_socket->dataLength );

}
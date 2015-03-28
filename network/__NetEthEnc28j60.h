#ifndef __NETETHENC28J60_H
#define __NETETHENC28J60_H

/*
 * registers
 */
#define ECON1    0x1f
#define ECON2    0x1e
#define MIREGADR 0x14
#define MIWRL    0x16
#define MIWRH    0x17

#define ERDPT    0x00
#define ERDPTH   0x01
#define EWRPT    0x02
#define EWRPTH   0x03
#define ETXST    0x04
#define ETXSTH   0x05
#define ETXND    0x06
#define ETXNDH   0x07
#define ERXST    0x08
#define ERXSTH   0x09
#define ERXND    0x0a
#define ERXNDH   0x0b
#define ERXRDPT  0x0c
#define ERXRDPTH 0x0d
#define EDMAST   0x10
#define EDMASTH  0x11
#define EDMAND   0x12
#define EDMANDH  0x13
#define EDMADST  0x14
#define EDMADSTH 0x15
#define EDMACS   0x16
#define EDMACSH  0x17


#define ERXFCON  0x18
#define MACON1   0x00
#define MACON2   0x01
#define MACON3   0x02
#define MACON4   0x03
#define MAMXFL   0x0a
#define MAMXFLH  0x0b
#define MABBIPG  0x04
#define MAIPGL   0x06
#define MAIPGH   0x07
#define MAADR1   0x04
#define MAADR2   0x05
#define MAADR3   0x02
#define MAADR4   0x03
#define MAADR5   0x00
#define MAADR6   0x01
#define EREVID   0x12
#define ECOCON   0x15
#define EPKTCNT  0x19
#define EIR      0x1c
#define EIE      0x1b
#define ESTAT    0x1d

#define MACLCON1 0x08
#define MACLCON2 0x09

#define MICMD    0x12

#define MIIRD    0x01

// IRQ on ENC28J60
#define INTIE 0x80
#define PKTIE 0x40
#define DMAIE 0x20
#define LINKIE 0x01
#define TXIE 0x08
#define TXERIE 0x02
#define RXERIE 0x01
// Available interrupts
#define PKTIF 0x40
#define DMAIF 0x20
#define LINKIF 0x10
#define TXIF 0x08
#define TXERIF 0x02
#define RXERIF 0x01

/*
 * PHY registers
 */
#define PHLCON  0x14
#define PHCON2  0x10
#define PHCON1  0x00

/*
 * SPI commands
 */
#define WBMCMD  0b01111010      // write buffer memory
#define RBMCMD  0b00111010      // read buffer memory
#define BFSCMD  0b10000000      // bit field set
#define BFCCMD  0b10100000      // bit field clear
#define WCRCMD  0b01000000      // write control register
#define RCRCMD  0b00000000      // read control register
#define RSTCMD  0b11111111      // reset

/*
 * maximum packet length
 */
#define BUF_SIZE        1518

/*
 * ENC memory allocation
 */
#define RAM_SIZE        8192                            // 8kb RAM available
#define RECEIVE_START   0                               // receive buffer start address : Should be an even memory address; must be 0 for errata
#define RECEIVE_END     (TRANSMIT_START - 1)            // receive buffer end address   : Odd for errata workaround
#define TRANSMIT_START  (RAM_SIZE - (BUF_SIZE + 100))   // transmit buffer start address, a few more bytes for padding pseudo header + transmit status : Even memory address
#define REPLY_START     (TRANSMIT_START + 1)            // reply buffer starts after per packet control byte
#define RECEIVE_SIZE        (RECEIVE_END-RECEIVE_START+1)   // receive buffer size

#define Net_Ethernet_28j60_HALFDUPLEX   0b0
#define Net_Ethernet_28j60_FULLDUPLEX   0b1

#define NO_ADDR 0xFFFF

/*
 * library globals
 */
typedef struct
        {
        unsigned char   valid;                             // valid/invalid entry flag
        unsigned long   tmr;                               // timestamp
        unsigned char   ip[4];                             // IP address
        unsigned char   mac[6];                            // MAC address behind the IP address
        } Net_Ethernet_28j60_arpCacheStruct;

extern Net_Ethernet_28j60_arpCacheStruct Net_Ethernet_28j60_arpCache[]; // ARP cash, 3 entries max

extern unsigned char    Net_Ethernet_28j60_macAddr[6];            // MAC address of the controller
extern unsigned char    Net_Ethernet_28j60_ipAddr[4];             // IP address of the device
extern unsigned char    Net_Ethernet_28j60_gwIpAddr[4];           // GW
extern unsigned char    Net_Ethernet_28j60_ipMask[4];             // network mask
extern unsigned char    Net_Ethernet_28j60_dnsIpAddr[4];          // DNS serveur IP
extern unsigned char    Net_Ethernet_28j60_rmtIpAddr[4];          // remote IP Address of host (DNS server reply)

extern unsigned long    Net_Ethernet_28j60_userTimerSec;          // must be incremented by user 1 time per second

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
  char remoteIP[4];                // Remote IP address
  char remoteMAC[6];               // Remote MAC address
  unsigned int remotePort;         // Remote TCP port
  unsigned int destPort;           // Destination TCP port
  unsigned int dataLength;         // Current TCP payload size
  unsigned int broadcastMark;      // =0 -> Not broadcast; =1 -> Broadcast
} UDP_28j60_Dsc;
// Each field refers to the last received package

extern UDP_28j60_Dsc udpRecord_28j60;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//** Socket structure definition
typedef struct {
  char          remoteIP[4];        // Remote IP address
  char          remoteMAC[6];       // Remote MAC address
  unsigned int  remotePort;         // Remote TCP port
  unsigned int  destPort;           // Destination TCP port

  unsigned int  dataLength;         // TCP payload size (refers to the last received package)
  unsigned int  remoteMSS;          // Remote Max Segment Size
  unsigned int  myWin;              // My Window
  unsigned int  myMSS;              // My Max Segment Size
  unsigned long mySeq;              // My Current sequence
  unsigned long myACK;              // ACK

  char          stateTimer;         // State timer
  char          retransmitTimer;    // Retransmit timer
  unsigned int  packetID;           // ID of packet
  char          open;               // =0 -> Socket busy;  =1 -> Socket free
  char          ID;                 // ID of socket
  char          broadcastMark;      // =0 -> Not broadcast; =1 -> Broadcast
  char          state;              // State table:
                                     //             0 - connection closed
                                     //             1 - remote SYN segment received, our SYN segment sent, and We wait for ACK (server mode)
                                     //             3 - connection established
                                     //             4 - our SYN segment sent, and We wait for SYN response (client mode)
                                     //             5 - FIN segment sent, wait for ACK.
                                     //             6 - Received ACK on our FIN, wait for remote FIN.
                                     //             7 - Expired ACK wait time. We retransmit last sent packet, and again set Wait-Timer. If this happen again
                                     //                 connection close.
  // Buffer....................//
  unsigned int nextSend;       //    // "Pointer" on first byte in buffer we want to send.
  unsigned int lastACK;        //    // "Pointer" on last acknowledged byte in buffer.
  unsigned int lastSent;       //    // "Pointer" on last sent byte in buffer.
  unsigned int lastWritten;    //    // "Pointer" on last written byte in buffer which not sent yet.
  unsigned int numToSend;      //    // Number of bytes in buffer to be sent.
  char         buffState;      //    // Private variable.
  char        *txBuffer;       //    // Pointer on Tx bafer.
  //...........................//

} SOCKET_28j60_Dsc;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern const char NUM_OF_SOCKET_28j60;
extern const unsigned int TCP_TX_SIZE_28j60;
extern const unsigned int MY_MSS_28j60;
extern const unsigned int SYN_FIN_WAIT_28j60;
extern const unsigned int RETRANSMIT_WAIT_28j60;

extern char tx_buffers_28j60[][];
extern SOCKET_28j60_Dsc socket_28j60[];






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * prototypes for public functions
 */
extern  void            Net_Ethernet_28j60_Init(unsigned char *resetPort, unsigned char resetBit, unsigned char *CSport, unsigned char CSbit, unsigned char *mac, unsigned char *ip, unsigned char configuration);
extern  unsigned char   Net_Ethernet_28j60_doPacket();
extern  void            Net_Ethernet_28j60_putByte(unsigned char b);
extern  void            Net_Ethernet_28j60_putBytes(unsigned char *ptr, unsigned int n);
extern  void            Net_Ethernet_28j60_putConstBytes(const unsigned char *ptr, unsigned int n);
extern  unsigned char   Net_Ethernet_28j60_getByte();
extern  void            Net_Ethernet_28j60_getBytes(unsigned char *ptr, unsigned int addr, unsigned int n);
extern  unsigned int    Net_Ethernet_28j60_UserUDP(UDP_28j60_Dsc *udpDsc);
extern  void            Net_Ethernet_28j60_payloadInitUDP();
extern void             Net_Ethernet_28j60_UserTCP(SOCKET_28j60_Dsc *socket);
extern  void            Net_Ethernet_28j60_confNetwork(char *ipMask, char *gwIpAddr, char *dnsIpAddr);

extern char             Net_Ethernet_28j60_connectTCP(char *remoteIP, unsigned int remote_port ,unsigned int my_port, SOCKET_28j60_Dsc **used_socket);
extern char             Net_Ethernet_28j60_disconnectTCP(SOCKET_28j60_Dsc *socket);
extern char             Net_Ethernet_28j60_startSendTCP(SOCKET_28j60_Dsc *socket);

extern char             Net_Ethernet_28j60_putByteTCP(char ch, SOCKET_28j60_Dsc *socket);
extern unsigned int     Net_Ethernet_28j60_putBytesTCP(char *ptr,unsigned int n, SOCKET_28j60_Dsc *socket);
extern unsigned int     Net_Ethernet_28j60_putConstBytesTCP(const far char *ptr,unsigned int n, SOCKET_28j60_Dsc *socket);
extern unsigned int     Net_Ethernet_28j60_putStringTCP(char *ptr, SOCKET_28j60_Dsc *socket);
extern unsigned int     Net_Ethernet_28j60_putConstStringTCP(const far char *ptr, SOCKET_28j60_Dsc *socket);
extern char             Net_Ethernet_28j60_bufferEmptyTCP(SOCKET_28j60_Dsc *socket);
extern void             Net_Ethernet_28j60_stackInitTCP();

#endif
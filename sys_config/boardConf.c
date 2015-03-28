#include "boardconf.h"
#include <stdbool.h>
#include "main.h"
#include "settings/settings_manager.h"
#include "logger/logger.h"
#include "network/network.h"
#include "rtc/ds1307.h"



// Webserver pin definitions
sbit Net_Ethernet_28j60_Rst           at PORTA0_bit;
sbit Net_Ethernet_28j60_CS            at PORTC2_bit;
sbit Net_Ethernet_28j60_Rst_Direction at DDA0_bit;
sbit Net_Ethernet_28j60_CS_Direction  at DDC2_bit;
sbit Net_Ethernet_28j60_INT           at PORTD3_bit;
sbit Net_Ethernet_28j60_INT_Direction at DDD3_bit;

// FAT32 pin definitions
sbit Mmc_Chip_Select                  at PORTC4_bit;
sbit Mmc_Chip_Select_Direction        at DDC4_bit;
sbit Mmc_Card_Detect                  at PORTC5_bit;
sbit Mmc_Card_Detect_Direction        at DDC5_bit;

// RTC Pins
sbit rtc_out                          at PORTD2_bit;
sbit rtc_out_dir                      at DDD2_bit;

/*!
 *  \brief Board Initialization
 *
 *  Configures board memory management, two wire, and settings manager.
 *
 */
void configure_board()
{
    MM_Init();          // Memory manager
    TWI_Init( 100000 ); // Initialize to 100khz
    Delay_ms( 1000 );   // TWI needs a small delay for osc stability
    
    /* Setup a PC INT for SDCard insertion detection */
    Mmc_Card_Detect_Direction = 0;
    PCICR  |= ( 1<<PCIE2 );
    PCMSK2 |= ( 1<<PCINT21 );
    
    /* Setup INT0 external interrupt for RTC output */
    EICRA |= ( 1<<ISC01 );              // Falling edge generates an IRQ
    EIFR  &= ~( 1 << INTF0 );           // Clear IRQ flag
    EIMSK |= ( 1<<INT0 );               // Enable IRQ
    
    settings_init();    // EEPROM Intialization for settings manager
}

/*!
 *  \brief Timer Initialization
 *
 */
void configure_timers()
{

}

/*!
 *  \brief Configure the RTC
 *
 *  Configure the RTC settings as well as the input pin for
 *  INT0 used by the square wave generated from the RTC.
 *  Falling edge is required as per datasheet  ac:ds1307_rtc
 *
 */
void configure_rtc()
{
    rtc_settings_t* p_rtc = settings_get_rtc();
    ds1307_config_t rtc_conf;

    rtc_out_dir = 0;
    rtc_out     = 1;
    
    rtc_conf.output_config = DS1307_SQW_1HZ;
    rtc_conf.time_zone     = p_rtc->rtc_time_zone;
    rtc_conf.dst_enable    = p_rtc->dst_enable;
    rtc_conf.mode_24hour   = true;

    ds1307_init( &rtc_conf );
}

/*!
 *  \brief Configure the webserver
 *
 *   Mac address is set by a literal.  All other settings are read from
 *   EEPROM and used to initialize the webserver.
 *
 */
void configure_webserver()
{
    network_settings_t* p_net = settings_get_network();
    network_config_t net_config;

    uint8_t mac_addr[] = {0x00,0x14,0xA5,0x76,0x19,0x3A};
    
    memcpy( net_config.myMacAddr,   mac_addr, 6 );
    memcpy( net_config.myIpAddr, p_net->myIP, 4 );
    memcpy( net_config.gwIpAddr, p_net->gwIP, 4 );
    memcpy( net_config.ipMask,  p_net->subIP, 4 );
    memcpy( net_config.dnsIpAddr, p_net->dnsIP, 4 );
    net_config.DHCP_Mode = p_net->DHCP_enable;
    net_config.port = p_net->port;

    network_init( &net_config );

}

// File system
int8 configure_file_system()
{
    if( Mmc_Card_Detect == 1 )
    {
        return -1;
    }

    else
    {
        int8 status;
        uint8_t tries = 5;

        SPI1_Init_Advanced( _SPI_MASTER, _SPI_FCY_DIV128, _SPI_CLK_LO_LEADING );
        Delay_ms( 10 );
        
        do
        {
            /*
            E_READ              =  -1,  // Error in raw read access to media.
            E_WRITE             =  -2,  // Error in raw write access to media.
            E_INIT_CARD         =  -3,  // Error in media initialization sequence.
            E_BOOT_SIGN         =  -4,  // Sector with boot record sign not found.
            E_BOOT_REC          =  -5,  // Boot record not found.
            E_FILE_SYS_INFO     =  -6,  // Error retrieving file system info.
            E_DEVICE_SIZE       =  -7,  // Error retrieving file size.
            E_FAT_TYPE          =  -8,  // Wrong file system.
            */
            status = FAT32_Init();

        } while( status < 0 && tries-- > 0 );
        
        SPI1_Init_Advanced( _SPI_MASTER, _SPI_FCY_DIV2, _SPI_CLK_LO_LEADING );
        Delay_ms( 10 );
        
        return status;
    }
}

#ifdef DEBUG
/*!
 *  \brief Initializes terminal
 *
 */
void configure_terminal()
{
    UART1_Init_Advanced( 38400, _UART_NOPARITY, _UART_ONE_STOPBIT );
    Delay_ms( 100 );
    UART1_Write_Text( "Terminal online\r\n" );
}
#endif
#include "settings_manager.h"
#include <stdbool.h>
#include <stdint.h>


void main() 
{
     network_settings_t* network;
     tide_settings_t*    tides;
     pump_settings_t*    pumps;
     rtc_settings_t*     rtc;
     char myIp[]  = { 192, 168, 1, 151 };
     char myGw[]  = { 192, 168, 1, 1 };
     char myDns[] = { 192, 168, 1, 1 };
     char mySub[] = { 255, 255, 255, 0 };
     char timesvr[] = "time.nist.gov";
     uint32_t volume[12]  = { 592000, 575200, 612101, 722625, 598750, 554569, 510023, 422333, 112500, 90200, 432032, 544044 };
     uint32_t volume2[12] = { 525000, 510000, 325000, 212050, 485030, 592100, 635222, 755433, 980012, 1320412, 743011, 585502 };
     
     char txt_buffer[100];
     #if AVR
     UART1_Init( 38400 );
     #else
     UART1_Init_Advanced( 115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10 );
     #endif
     Delay_ms( 100 );
     
     UART1_Write_Text( "Terminal Ready\r\n" );
     
     #if AVR

     #else
     I2C1_Init_Advanced( 100000, &_GPIO_MODULE_I2C1_PB67 );
     #endif
     Delay_ms( 1000 );
     
     settings_init();

     network = settings_get_network();

     memcpy( network->myIP, myIp, 4 );
     memcpy( network->subIP, mySub, 4 );
     memcpy( network->gwIP, myGw, 4 );
     memcpy( network->dnsIP, myDns, 4 );
     network->DHCP_enable = FALSE;
     strcpy( network->time_server, timesvr );
     network->port = 80;

     settings_commit( SETTINGS_NETWORK );
     settings_check_data();
     network = settings_get_network();
     
     sprintf( txt_buffer, "Time server found is: %s\r\n", network->time_server );
     UART1_Write_Text( txt_buffer );

     pumps = settings_get_pump();

     pumps->last_pump_cycled = 1;
     pumps->last_pumping1_gpm = 560;
     pumps->last_pumping2_gpm = 520;
     memcpy( pumps->month_volume_pump1, volume,  ( sizeof( uint32_t ) * 12 ) );
     memcpy( pumps->month_volume_pump2, volume2, ( sizeof( uint32_t ) * 12 ) );

     settings_commit( SETTINGS_PUMP );
     settings_check_data();
     
     pumps = settings_get_pump();

     sprintf( txt_buffer, "Found in the month of Jan on pump 1: %u\r\n", pumps->month_volume_pump1[2] );
     UART1_Write_Text( txt_buffer );

}
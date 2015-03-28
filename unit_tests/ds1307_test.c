#include "ds1307.h"
#include <stdbool.h>

void main() {
     
     char txt_buffer[100];
     uint32_t test_num = 123456;
     uint32_t read_num = 0;
     ds1307_config_t rtc_conf;
     
     #if AVR
     UART1_Init_Advanced( 38400, _UART_NOPARITY, _UART_ONE_STOPBIT );
     TWI_Init( 100000 );
     Delay_ms( 1000 );
     #else
     UART1_Init_Advanced( 115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10 );
     I2C1_Init_Advanced( 100000, &_GPIO_MODULE_I2C1_PB67 );
     Delay_ms( 1000 );
     #endif
     
     UART1_Write_Text( "Starting DS1307 Test\r\n" );
     
     rtc_conf.time_zone = -8;     // Local time zone
     rtc_conf.dst_enable = TRUE;      // Enable or disable daylight savings time
     rtc_conf.config = 0;
     
     ds1307_init( &rtc_conf );
     
     sprintl( txt_buffer, "Local: %s - GMT: %s\r\n", ds1307_get_local_time_str( MODE24HOUR ), ds1307_get_GMT_time_str( MODE24HOUR ) );
     UART1_Write_Text( txt_buffer );

     UART1_Write_Text( "Writing CHAR c to DS1307 RAM\r\n" );
     ds1307_write_ram( DS1307_RAM_START_ADDR, 'c' );
     UART1_Write_Text( "Reading from DS1307 = " );
     UART1_Write( ds1307_read_ram( DS1307_RAM_START_ADDR ) );
     UART1_Write_Text( "\r\n" );
     sprintl( txt_buffer, "Now writting a series of bytes, writting: %u to ram\r\n", test_num );
     UART1_Write_Text( txt_buffer );
     ds1307_write_ram_bulk( (void*)&test_num, sizeof( test_num ) );
     ds1307_read_ram_bulk( (void*)&read_num, sizeof( read_num ) );
     sprintl( txt_buffer, "Now reading from bulk, number found: %u\r\n", read_num );
     UART1_Write_Text( txt_buffer );
     
     if( test_num == read_num )
     {
        UART1_Write_Text( "Test Succeeded\r\n" );
     }
     

}
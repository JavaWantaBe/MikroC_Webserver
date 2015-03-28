#include "websvr_socket.h"
#define WEBSERVER_HEADER  "__NetEthInternal.h"
#include <stddef.h>

const unsigned long HEAP_SIZE = 4096;
SOCKET_Dsc p_socket1, p_socket2, p_socket3, p_socket4, p_socket5;


void main() 
{
     char txt_buffer[50];
     socket_t* my_socket;
     
     MM_Init();

     UART1_Init_Advanced( 115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10 );
     Delay_ms( 100 );
     UART1_Write_Text( "Socket test begin\r\n" );

     sprintl( txt_buffer, "Start Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );

     websocket_init();
     websocket_get_count();
     
     my_socket = websocket_find_socket( &p_socket1 );
     my_socket->alive_time = 5;
     
     sprinti( txt_buffer, "Added socket 1, count: %u\r\n", websocket_get_count() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket2 );
     my_socket->alive_time = 6;
     
     sprinti( txt_buffer, "Added socket 2, count: %u\r\n", websocket_get_count() );
     UART1_Write_Text( txt_buffer );

     my_socket = websocket_find_socket( &p_socket3 );
     my_socket->alive_time = 7;
     
     sprinti( txt_buffer, "Added socket 3, count: %u\r\n", websocket_get_count() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket4 );
     my_socket->alive_time = 8;
     
     sprinti( txt_buffer, "Added socket 4, count: %u\r\n", websocket_get_count() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket5 );
     my_socket->alive_time = 8;
     
     sprinti( txt_buffer, "Added socket 5, count: %u\r\n", websocket_get_count() );
     UART1_Write_Text( txt_buffer );
     
     sprintl( txt_buffer, "End Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = NULL;
     
     my_socket = websocket_find_socket( &p_socket1 );
     sprinti( txt_buffer, "Alive time is: %u\r\n", my_socket->alive_time );
     UART1_Write_Text( txt_buffer );
     
     if( websocket_close( my_socket ) == SOCKET_OK )
     {
         sprinti( txt_buffer, "Closed socket 1, count now: %u\r\n", websocket_get_count() );
         UART1_Write_Text( txt_buffer );
     }
     
     sprintl( txt_buffer, "Removed 1 - Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket2 );
     sprinti( txt_buffer, "Alive time is: %u\r\n", my_socket->alive_time );
     UART1_Write_Text( txt_buffer );

     if( websocket_close( my_socket ) == SOCKET_OK )
     {
         sprinti( txt_buffer, "Closed socket 2, count now: %u\r\n", websocket_get_count() );
         UART1_Write_Text( txt_buffer );
     }
     
     sprintl( txt_buffer, "Removed 2 - Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket3 );
     sprinti( txt_buffer, "Alive time is: %u\r\n", my_socket->alive_time );
     UART1_Write_Text( txt_buffer );

     if( websocket_close( my_socket ) == SOCKET_OK )
     {
         sprinti( txt_buffer, "Closed socket 3, count now: %u\r\n", websocket_get_count() );
         UART1_Write_Text( txt_buffer );
     }
     
     sprintl( txt_buffer, "Removed 3 - Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket4 );
     sprinti( txt_buffer, "Alive time is: %u\r\n", my_socket->alive_time );
     UART1_Write_Text( txt_buffer );

     if( websocket_close( my_socket ) == SOCKET_OK )
     {
         sprinti( txt_buffer, "Closed socket 4, count now: %u\r\n", websocket_get_count() );
         UART1_Write_Text( txt_buffer );
     }

     sprintl( txt_buffer, "Removed 4 - Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );
     
     my_socket = websocket_find_socket( &p_socket5 );
     sprinti( txt_buffer, "Alive time is: %u\r\n", my_socket->alive_time );
     UART1_Write_Text( txt_buffer );

     if( websocket_close( my_socket ) == SOCKET_OK )
     {
         sprinti( txt_buffer, "Closed socket 5, count now: %u\r\n", websocket_get_count() );
         UART1_Write_Text( txt_buffer );
     }

     sprintl( txt_buffer, "Removed 5 - Free Mem: %u\r\n", MM_TotalFreeMemSize() );
     UART1_Write_Text( txt_buffer );

}
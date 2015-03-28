#include "websvr_page.h"
#include <stdbool.h>

// Test variables
sbit Mmc_Chip_Select                  at GPIOD_ODR.B3;
const code char mypage[] = "Happy is my page";
char dynamic_data[1024];
const unsigned long HEAP_SIZE = 2048;
int page_count;

// Prototypes
void page_update( void );

// Functions
void page_update()
{
   strcpy( dynamic_data, "I am pretty, oh so pretty" );
}

void main() 
{
    char txt_buffer[50];
    webpage_t* read_page;
    
    MM_Init();
    UART1_Init_Advanced( 115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10 );
    Delay_ms( 100 );
    
    UART1_Write_Text( "Testing started\r\n" );
    webpage_init();
    
    sprintl( txt_buffer, "Free memory: %u\r\n", MM_TotalFreeMemSize() );
    UART1_Write_Text( txt_buffer );
    
    webpage_add_staticpage( mypage, "index.htm", sizeof( mypage ) );
    webpage_add_flashpage( "happy.txt", TRUE );
    webpage_add_dynamicpage( dynamic_data, "data.js", page_update );
    
    page_count = webpage_get_page_count();
    
    sprinti( txt_buffer, "Webpage count: %u\r\n", page_count );
    UART1_Write_Text( txt_buffer );
    
    sprintl( txt_buffer, "Free memory: %u\r\n", MM_TotalFreeMemSize() );
    UART1_Write_Text( txt_buffer );
    
    UART1_Write_Text( webpage_print_page_list() );
    
    read_page = webpage_find_page( "data.js" );
    read_page->dynamic_page.page_update();
    
    sprinti( txt_buffer, "Page found, contains: %s\r\n", read_page->dynamic_page.start_ptr );
    UART1_Write_Text( txt_buffer );
    
    webpage_remove_page( "happy.txt" );
    
    page_count = webpage_get_page_count();

    sprinti( txt_buffer, "Webpage count: %u\r\n", page_count );
    UART1_Write_Text( txt_buffer );
    
    read_page = webpage_find_page( "index.htm" );
    sprinti( txt_buffer, "Static page, first char: %c\r\n", read_page->static_page.start_ptr[0] );
    UART1_Write_Text( txt_buffer );
    
    webpage_remove_page( "index.htm" );
    
    page_count = webpage_get_page_count();

    sprinti( txt_buffer, "Webpage count: %u\r\n", page_count );
    UART1_Write_Text( txt_buffer );
    
    sprintl( txt_buffer, "Free memory: %u\r\n", MM_TotalFreeMemSize() );
    UART1_Write_Text( txt_buffer );
    
    
    
}
#include <stddef.h>
#include <stdbool.h>

#include "../network/network.h"
#include "../network/protocols/http_server/http_server.h"
#include "../network/protocols/http_client/http_client.h"
#include "../network/page_manager/page_manager.h"
#include "../resources/http.h"
#include "__Lib_FAT32.h"
#include "../utils/lwprintf.h"
#include "../logger/logger.h"


static void system_init( void );
static void config_network( void );
static char* sys_time_str( void );
static void file_handler( char* file_name );
static void form_handler( key_value_t* key_pairs, char* form_name );
static void print_char( char c );


#if AVR
const unsigned int HEAP_SIZE = 4096;
#define CLOCK_VECTOR IVT_ADDR_TIMER1_COMPA
// Webserver definitions
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

#else
const unsigned int HEAP_SIZE = 8192;
#define CLOCK_VECTOR IVT_INT_TIM2
sbit Mmc_Chip_Select at GPIOD_ODR.B3;
sbit Mmc_Card_Detect at GPIOD_IDR.B15;
#endif

static volatile int count = 0;
char sys_time[30] = "22:48:13 - 25 Aug 2013";

char testRequest1[] = "http://opendap.co-ops.nos.noaa.gov/ioos-dif-sos/SOS?service=SOS&request=GetObservation&version=1.0.0&observedProperty=sea_surface_height_amplitude_due_to_equilibrium_ocean_tide&offering=urn:ioos:station:NOAA.NOS.CO-OPS:9432780&responseFormat=text%2Fcsv&eventTime=2013-08-26T00:00:00Z/2013-09-03T23:59:00Z&result=VerticalDatum%3D%3Durn:ioos:def:datum:noaa::MLLW&dataType=HighLowTidePredictions&unit=Meters";
char testRequest2[] = "http://download.linnrecords.com/test/mp3/tone.aspx";
char testRequest3[] = "http://people.oregonstate.edu/~peterseb/misc/test_files/btest001.pdf";

/*********************************
 *   Functions
 ********************************/
static void system_init()
{
    MM_Init();
    #if AVR
    UART1_Init_Advanced( 38400, _UART_NOPARITY, _UART_ONE_STOPBIT );
    TCCR1B = 0x0D;
    OCR1AH = 0x1E;
    OCR1AL = 0x84;
    TIMSK1 = 0x02;
    Mmc_Card_Detect_Direction = 0;
    PCMSK2 |= ( 1<<PCINT21 );
    PCICR  |= ( 1<<PCIE2 );
    PCIFR  |= ( 1<<PCIF2 );
    #else
    UART1_Init_Advanced( 115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10 );

    RCC_APB1ENR.TIM2EN = 1;
    TIM2_CR1.CEN = 0;
    TIM2_PSC = 2399;
    TIM2_ARR = 62500;
    NVIC_IntEnable(IVT_INT_TIM2);
    TIM2_DIER.UIE = 1;
    TIM2_CR1.CEN = 1;
    /*GPIO_Digital_Input( &GPIOD_BASE, _GPIO_PINMASK_15 );   // Enable clock for alternate pin functions
    SYSCFGEN_bit   = 1;
    SYSCFG_EXTICR4 |= ( 1 << EXTI150 ) | ( 1 << EXTI151 ); // Map external interrupt on PD15
    EXTI_RTSR      &= ~( 1 << MR15 );                      // Set interrupt on Rising edge (none)
    EXTI_FTSR      |= ( 1 << TR15 );                       // Set Interrupt on Falling edge (PD15)
    EXTI_IMR       |= ( 1 << MR15 );                       // Set mask
    NVIC_IntEnable( IVT_INT_EXTI15_10 );                   // Enable External interrupt*/
    #endif
}

void spi_enable_slow()
{
   #if AVR
   SPI1_Init_Advanced( _SPI_MASTER, _SPI_FCY_DIV128, _SPI_CLK_LO_LEADING );
   #else
   SPI3_Init_Advanced( _SPI_FPCLK_DIV64,
                        _SPI_MASTER | _SPI_8_BIT | _SPI_CLK_IDLE_LOW | _SPI_FIRST_CLK_EDGE_TRANSITION |
                        _SPI_MSB_FIRST | _SPI_SS_DISABLE |
                        _SPI_SSM_ENABLE | _SPI_SSI_1,
                        &_GPIO_MODULE_SPI3_PC10_11_12 );
   #endif
}

void spi_enable_fast()
{
   #if AVR
   SPI1_Init_Advanced( _SPI_MASTER, _SPI_FCY_DIV2, _SPI_CLK_LO_LEADING );
   #else
   SPI3_Init_Advanced( _SPI_FPCLK_DIV2,
                        _SPI_MASTER | _SPI_8_BIT | _SPI_CLK_IDLE_LOW | _SPI_FIRST_CLK_EDGE_TRANSITION |
                        _SPI_MSB_FIRST | _SPI_SS_DISABLE |
                        _SPI_SSM_ENABLE | _SPI_SSI_1,
                        &_GPIO_MODULE_SPI3_PC10_11_12 );
   #endif
}

static void config_network()
{
    #if AVR
    network_config_t net_config = {
    {0x00,0x14,0xA5,0x76,0x19,0x3A},  // Mac address
    {192,168,1,150},                  // Ip address
    {192,168,1,1},                    // Gateway address
    {255,255,255,0},                  // Subnet
    {192,168,1,1},                    // DNS
    80                               // HTTP Port
    };
    #else
    network_config_t net_config = {
    {0x00,0x14,0xA5,0x76,0x19,0x3E},  // Mac address
    {192,168,1,151},                  // Ip address
    {192,168,1,1},                    // Gateway address
    {255,255,255,0},                  // Subnet
    {192,168,1,1},                    // DNS
    80                               // HTTP Port
    };
    #endif
    
    log_init( sys_time_str, 1377499793 );
   
    if( network_init( &net_config ) < 0 )
    {
       printf( "Error in network initialization\r\n" );
       return;
    }

    page_manager_add_staticpage( index_htm,    "index.htm" ,  sizeof( index_htm )     );
    page_manager_add_staticpage( pageCSS_css,  "pageCSS.css", sizeof( pageCSS_css )   );
    page_manager_add_staticpage( pgBkgd_png,   "pgBkgd.png",  sizeof( pgBkgd_png )    );
    page_manager_add_staticpage( logo_png,     "logo.png",    sizeof( logo_png )      );
    page_manager_add_staticpage( err_404_htm,  "404_err.htm",  sizeof( err_404_htm )   );
    page_manager_add_staticpage( col_12_gif,   "col_12.gif",  sizeof( col_12_gif )    );
    page_manager_add_staticpage( cntbkgd_jpg,  "cntbkgd.jpg", sizeof( cntbkgd_jpg )   );
    page_manager_add_staticpage( style9_css,   "style9.css",  sizeof( style9_css )    );
    page_manager_add_staticpage( styledef_css, "styledef.css", sizeof( styledef_css ) );

    page_manager_add_flashpage( "richard.jpg",  FALSE );
    page_manager_add_flashpage( "linz.jpg",     FALSE );
    page_manager_add_flashpage( "system.txt",   TRUE );

    http_set_form_callback( form_handler );
    http_client_set_file_callback( file_handler );
}

void main() {

     system_init();
     printfInit( print_char );
     #if AVR
     SREG_I_bit = 1;
     #else
     EnableInterrupts();
     #endif
     config_network();

     UART1_Write_Text( "Ready for Test\r\n" );

     while( 1 )
     {
        network_read_packet();

        if( UART1_Data_Ready() )
        {
            switch( UART1_Read() )
            {
                case '1':
                    printf( "Download Test 1 - Chunked\r\nString: %s\r\n", testRequest1 );
                    http_client_request_file( testRequest1, 80 );
                    break;
                case '2':
                    printf( "Download Test 2 - Persistant\r\nString: %s\r\n", testRequest2 );
                    http_client_request_file( testRequest2, 80 );
                    break;
                case '3':
                    printf( "Download Test 3 - HTTP 1.0\r\nString: %s\r\n", testRequest3 );
                    http_client_request_file( &testRequest3, 80 );
                    break;
                default:
                    UART1_Write_Text( "Not a recognized key\r\n" );
            }
        }

        if( count >= 10 )
        {
            FAT32_IncTime( 10 );
            network_cleanup();
            count = 0;
        }
     }
}


static void print_char( char c )
{
    UART1_Write( c );
}

static char* sys_time_str()
{
    return sys_time;
}

static void file_handler( char* file_name )
{
    printf( "File downloaded : %s\r\n", file_name );
    FAT32_Dir();
}

static void form_handler( key_value_t* key_pairs, char* form_name )
{
   int i = 0;
   printf( "Form name passed: %s\r\n", form_name );

   while( key_pairs[i].key[0] != '\0' )
   {
       printf( "Key: %s\r\nValue: %s\r\n", key_pairs[i].key, key_pairs[i].value );
       i++;
   }
}


void Timer2_interrupt_ISR() iv CLOCK_VECTOR
{
    #ifndef AVR
    TIM2_SR.UIF = 0;
    #endif
    count++;
    network_time_update();
}
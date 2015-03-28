// gsmDriver

// Set of basic AT commands
const char atc0[] = "AT";                        // Every AT command starts with "AT"
const char atc1[] = "ATE0";                      // Disable command echo
const char atc2[] = "AT+CMGF=1";                 // TXT messages
const char atc3[] = "AT+CMGS=\"";                // sends SMS to desired number
const char atc4[] = "AT+CMGR=1";                 // Command for reading message from location 1 from inbox
const char atc5[] = "AT+CMGD=1,4";               // Erasing all messages from inbox
const char atc6[] = "AT+CMGL=\"ALL\"";           // Check status of received SMS
//
// SMS Message string
char SMS_Message[300];

// Responses to parse
const GSM_OK                       = 0;
const GSM_Ready_To_Receive_Message = 1;
const GSM_ERROR                    = 2;
const GSM_UNREAD                   = 3;

/*!
 *  \brief Wireless initialization
 *
 */
void gsmInit()
{
#if AVR
   /*UART1_Init_Advanced(9600,
                       _UART_EVENPARITY,
                       _UART_TWO_STOPBITS);*/
#else
    UART3_Init_Advanced(9600,
                        _UART_8_BIT_DATA,
                        _UART_NOPARITY,
                        _UART_ONE_STOPBIT,
                        &_GPIO_MODULE_USART3_PD89);
    GPIO_Digital_Input(&GPIOD_BASE, _GPIO_PINMASK_10);  //CTS
    GPIO_Digital_Output(&GPIOC_BASE, _GPIO_PINMASK_2);  //RTS
#endif
}


// phone number string
char phone_number[20];
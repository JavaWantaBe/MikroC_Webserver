#include "mcp9700a.h"


void mcp9700Init()
{
    GPIO_Analog_Input(&GPIOF_BASE, _GPIO_PINMASK_8);
    // ADC Init
    ADC3_Init();                        // Initialize ADC
}

unsigned long getMCP9700_C()
{
    unsigned long temp;

    // read temperature and display it in celsius degrees
    temp = ADC3_Get_Sample(6);
    temp = (unsigned long)(VREF * temp) / 4100;
    temp = temp - 500;
    
    return temp;
}

float getMCP9700_F(){

    float temp2;
    
    // convert and display in fahrenheit degrees
    temp2 = (float)(getMCP9700_C()) / 10 * 1.8 + 32;
    temp2 = temp2 * 10;

    return temp2;
}

/*!
 *  \brief Takes a temp and returns formatted text
 *
 *  \return char * - formatted string
 *    \retval 22.0C
 *
 */
char *getMCP9700TxtTemp(float tmp, short CF)
{
      static char txt[9];

      if(CF){
          FloatToStr(getMCP9700_F(), txt);
          txt[5] = ' ';
          txt[6] = 'F';
          txt[7] = 176;
          txt[8] ='\0';
      } else {
          LongToStr(getMCP9700_C(), txt);
          txt[5] = ' ';
          txt[6] = 'C';
          txt[7] = 176;
          txt[8] ='\0';
      }

      return txt;
}
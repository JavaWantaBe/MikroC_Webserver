/*!
 *   \file
 *
 *   \brief Smart Pump - Intelligent Self Programming Controller
 *
 *   \mainpage
 *   \section Specs Specifications
 *   MCU:             <br>Stellaris M4<br>
 *                    datasheet:  : <br><br>
 *   Develoment board:
 *                    <br>avrplc16 v6 & EasyAVR7<br>
 *                    manual:   <br>
 *                    schematic:   <br><br>
 *                    http://www.mikroe.com/eng/products/view/539/avrplc16-v6-plc-system/ <br><br>
 *   Oscillator:      External clock, 8.0000 MHz <br><br>
 *
 *   SW:              <br>mikroC PRO for AVR<br>
 *                    http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/ <br><br>
 *
 *   \section Modules Modules Used
 *
 *   Modules used: <br>
 *   <ul>
 *   <li><strong>RTC</strong></li>
 *   <li><strong>ADC</strong></li>
 *   <li><strong>ETHERNET</strong></li>
 *   <li><strong>MMC</strong></li>
 *   <li><strong>Relays</strong></li>
 *   <li><strong>RS232</strong></li>
 *   </ul>
 *
 *  \page Configuration
 *
 *  <p><strong>Switch:</strong> (SW5 -  NONE), (SW7 7.1, 7.2, 7.3, 7.4), (SW8 8.2, 8.5, 8.6), (SW9 9.4, 9.5, 9.6, 9.7),  (SW10 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8), (SW11 - NONE)<br>
 *  <strong><h2>Pin Configuration:</h2></strong><br>
 *  <strong><h3>PORTA:</h3></strong><br>
 *  Board                                           AVRPLC16                  EASYAVR7                 STELLARIS MINI       <strong>STELLARIS M3 MxPro</strong><br>
 *  PA0: (ADC0/PCINT0)                      - Ethernet Reset                Purge Packets BTN          RS232 Recieve                RS232 Recieve<br>
 *  PA1: (ADC1/PCINT1)                      - Tank Full Sensor              Ethernet Reset             RS232 Transmit               RS232 Transmit<br>
 *  PA2: (ADC2/PCINT2)                      - STAT Indicator                STAT Indicator             SPI Clock Sync               SPI Clock Sync<br>
 *  PA3: (ADC3/PCINT3)                      -                               Ethernet Chip Select                                    TFT - PWM Backlighting<br>
 *  PA4: (ADC4/PCINT4)                      - DATA Indicator                DATA Indicator             Master In Slave Out          Master In Slave Out<br>
 *  PA5: (ADC5/PCINT5)                      -                               MMC Chip Select            Master Out Slave In          <br>
 *  PA6: (ADC6/PCINT6)                      -                                                                                       Buzzer<br>
 *  PA7: (ADC7/PCINT7)                      -                               MMC Card Detect            Ethernet Reset               MMC Card Select<br>
 *  <br>
 *  <strong><h3>PORTB:</h3></strong><br>
 *  PB0: (PCINT8/XCK0/T0)                   - Relay - Inlet Pump 1          Relay - Inlet Pump 1         <br>
 *  PB1: (PCINT9/CLKO/T1)                   - Relay - Pump 1                Relay - Pump 1                          <br>
 *  PB2: (PCINT10/INT2/AIN0)                - Relay - Inlet Pump 2          Relay - Inlet Pump 2                                    EEPROM SCL<br>
 *  PB3: (PCINT11/OC0A/AIN1)                - Relay - Pump 2                Relay - Pump 2                                          EEPROM SDA<br>
 *  PB4: (PCINT12/OC0B/SS)                  - ADC_Chip_Select               ADC_Chip_Select            ADC_Chip_Select              TFT - READ X<br>
 *  PB5: (PCINT13/ICP3/MOSI)                - Master Out Slave In           Master Out Slave In        Relay - Pump 2               TFT - READ Y<br>
 *  PB6: (PCINT14/OC3A/MISO)                - Master In Slave Out           Master In Slave Out        Relay - Prime Pump2          <br>
 *  PB7: (PCINT15/OC3B/SCK)                 - SPI Clock Sync                SPI Clock Sync             Relay - Prime Pump1          DS1820 Temp Sensor - Case<br>
 *  <br>
 *  <p><strong><h3>PORTC:</h3></strong><br>
 *  PC0: (SCL/PCINT16)                      - RTC Clock Line                RTC Clock Line             Relay - Inlet Pump 2         <br>
 *  PC1: (SDA/PCINT17)                      - RTC Data Line                 RTC Data Line              Relay - Pump 1               <br>
 *  PC2: (TCK/PCINT18)                      - Ethernet Chip Select          Relay - Prime Pump1        Relay - Outlet Pump1         <br>
 *  PC3: (TMS/PCINT19)                      - Prime Pressure Switch         Relay - Prime Pump2        Relay - Outlet Pump2         <br>
 *  PC4: (TDO/PCINT20)                      - SD Card Chip Select                                                                   <br>
 *  PC5: (TDI/PCINT21)                      - SD Card Detect                                                                        TFT - PMRD<br>
 *  PC6: (TOSC1/PCINT22)                    -                                                          Ethernet Interrupt           <br>
 *  PC7: (TOSC2/PCINT23)                    -                                                          Relay - Inlet Pump 1         <br>
 *  <br>
 *  <strong><h3>PORTD:</h3></strong><br>
 *  PD0: (PCINT24/RXD0/T3)                  - RS232 Recieve                 RS232 Recieve                                           Relay - Prime Pump1<br>
 *  PD1: (PCINT25/TXD0)                     - RS232 Transmit                RS232 Transmit                                          Relay - Prime Pump2<br>
 *  PD2: (PCINT26/RXD1/INT0)                - Relay Prime Pump1             Tank Full - Sensor                                      Relay - Outlet Pump1<br>
 *  PD3: (PCINT27/TXD1/INT1)                - Ethernet Interrupt            Ethernet Interrupt                                      Relay - Outlet Pump2<br>
 *  PD4: (PCINT28/XCK1/OC1B)                - Relay - Outlet Pump1          Relay - Outlet Pump1                                    Relay - Inlet Pump 1<br>
 *  PD5: (PCINT29/OC1A)                     - Relay - Outlet Pump2          Prime Pressure Switch                                   Relay - Inlet Pump 2<br>
 *  PD6: (PCINT30/OC2B/ICP)                 - Water Flow Sensor             Water Flow Sensor                                       Relay - Pump 1<br>
 *  PD7: (OC2A/PCINT31)                     - Relay - Prime Pump2           Relay - Outlet Pump2                                    Relay - Pump 2<br>
 *  <br>
 *  PE0:                                                                                                                            TFT - DRIVE A<br>
 *  PE1:                                                                                                                            TFT - DRIVE B<br>
 *  PE2:                                                                                               Prime Pressure Switch        <br>
 *  PE3:                                                                                               Tank Full - Sensor           <br>
 *  PE4:                                                                                                                            <br>
 *  PE5:                                                                                                                            <br>
 *  PE6:                                                                                                                            <br>
 *  PE7:                                                                                                                            <br>
 *  <br>
 *  PF0:                                                                                               Ethernet Chip Select         <br>
 *  PF1:                                                                                               Water Flow Sensor            <br>
 *  PF2:                                                                                                                            Ethernet LED A<br>
 *  PF3:                                                                                                                            Ethernet LED B<br>
 *  PF4:                                                                                                                            <br>
 *  PF5:                                                                                                                            <br>
 *  PF6:                                                                                                                            <br>
 *  PF7:                                                                                                                            <br>
 *  <br>
 *  PG0:                                                                                               Ethernet Chip Select         <br>
 *  PG1:                                                                                               Water Flow Sensor            <br>
 *  PG2:                                                                                                                            <br>
 *  PG3:                                                                                                                            <br>
 *  PG4:                                                                                                                            <br>
 *  PG5:                                                                                                                            <br>
 *  PG6:                                                                                                                            <br>
 *  PG7:                                                                                                                            TFT - RS<br>
 *  <br>
 *  PH0:                                                                                               Ethernet Chip Select         <br>
 *  PH1:                                                                                                                            <br>
 *  PH2:                                                                                                                            <br>
 *  PH3:                                                                                                                            <br>
 *  PH4:                                                                                                                            TFT - PMWR<br>
 *  PH5:                                                                                                                            TFT - Reset<br>
 *  PH6:                                                                                                                            TFT - Chip Select<br>
 *  PH7:                                                                                                                            SD Card Detect<br>
 *  <br>
 *  PI0:                                                                                                                            <br>
 *  PI1:                                                                                                                            <br>
 *  PI2:                                                                                                                            <br>
 *  PI3:                                                                                                                            <br>
 *  PI4:                                                                                                                            <br>
 *  PI5:                                                                                                                            <br>
 *  PI6:                                                                                                                            <br>
 *  PI7:                                                                                                                            <br>
 *  <br>
 *  PJ0:                                                                                                                            TFT - DATA 0<br>
 *  PJ1:                                                                                                                            TFT - DATA 1<br>
 *  PJ2:                                                                                                                            TFT - DATA 2<br>
 *  PJ3:                                                                                                                            TFT - DATA 3<br>
 *  PJ4:                                                                                                                            TFT - DATA 4<br>
 *  PJ5:                                                                                                                            TFT - DATA 5<br>
 *  PJ6:                                                                                                                            TFT - DATA 6<br>
 *  PJ7:                                                                                                                            TFT - DATA 7<br>
 *  \section Needed Things Needed:
 *  <p>
 *  <ul>
 *  <li>InletValve</li>
 *  </ul>
 *  <p><br>
 *  <strong>Inputs</strong><br>
 *  Power Outage Input (input through optoisolated)<br>
 *  Battery Alert (INT2)<br>
 *  <br>
 *  <strong>Timers:</strong><br>
 *  Timer0 = not used "yet"<br>
 *  Timer1 = Input capture / overflow ppm<br>
 *  Timer2 = System scheduler, 10hz overflow
 *  WDT = Enabled</p>
 */

/*!
 * \page SuperPump_License
 *
 * THIS SOFTWARE IS PROVIDED BY ALPHALOEWE "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#ifndef _MAIN_H
#define _MAIN_H



#endif
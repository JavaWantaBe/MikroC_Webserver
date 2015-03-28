#ifndef DS18X20_H
#define DS18X20_H

#include <stdint.h>
#include <stdbool.h>

#define DS18X20_BASE_ADDRESS    &PORTA
#define DS18X20_PIN             3

//#define DS18X20_BASE_ADDRESS    &GPIOA_BASE
//#define DS18X20_PIN             3

#define RES9                    0x00
#define RESET_CONF              0x1F
#define RES10                   0x20
#define READ_ROM                0x33
#define RES11                   0x40
#define CONVERT_T               0x44
#define COPY_SCRATCHPAD         0x48
#define WRITE_SCRATCHPAD        0x4E
#define MATCH_ROM               0x55
#define RES12                   0x60
#define READ_POWER              0xB4
#define RECALL_ALARM            0xB8
#define READ_SCRATCHPAD         0xBE
#define SKIP_ROM                0xCC
#define ALARM_SEARCH            0xEC
#define SEARCH_ROM              0xF0

#define DS18X20_MODE_F          1
#define DS18X20_MODE_C          0

/*******************************
 *  DS18x20 Temp Functions
 ******************************/
// Initialize temp sensor
void ds18x20_init( uint8_t resolution, uint8_t alarm_high, uint8_t alarm_low );
// Check for alarms
bool ds18x20_is_alarm( void );
// Read and return temp reading
float ds18x20_get_temp( void );
// Get static temp stored
float ds18x20_get_upd_temp( void );
// Convert C to F
float ds18x20_CtoF( float val );
// Returns a string in F or C based on mode
char *ds18x20_get_str_temp( int mode );
// Update the temp buffer to be read at a later time
void  ds18x20_upd_temp( void );

#endif
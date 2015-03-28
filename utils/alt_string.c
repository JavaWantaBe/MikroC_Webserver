#include "alt_string.h"
#include <stddef.h>

/*!
 *  \brief Copies a ROM string to RAM
 *
 *  \note
 *   <notes>
 */
void strcpy_const( char* destination, const char* source )
{
    while( *destination++ = *source++ );

    *destination = '\0';
}
/*!
 *  \brief Concatenates a ROM string to RAM
 *
 *  \note
 *   <notes>
 */
void strcat_const( char* destination, const char* source )
{
    while( *destination != '\0' )
    {
        destination++;
    }

    strcpy_const( destination, source );
}
/*!
 *  \brief Compares a ROM string to RAM
 *
 *  \note
 *   <notes>
 */
int strncmp_const( char* str1, const char* str2, int num )
{
    while( num-- != 0 )
    {
        if( *str1++ != *str2++ )
        {
            return 1;
        }

    }

    return 0;
}
/*uint32_t strlen_const ( const char* str )
{
    uint32_t str_size = 0;

    while( *str != '\0' && str_size < INT32_MAX )
    {
        str++;
        str_size++;
    }

    return str_size;
}*/
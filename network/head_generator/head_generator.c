#include "head_generator.h"
#include <stddef.h>
#include "alt_string.h"


// File type definitions
typedef enum
{
    HTTP_TXT = 0,            // File is a text document
    HTTP_HTM,                // File is HTML (extension .htm)
    HTTP_HTML,               // File is HTML (extension .html)
    HTTP_CGI,                // File is HTML (extension .cgi)
    HTTP_XML,                // File is XML (extension .xml)
    HTTP_CSS,                // File is stylesheet (extension .css)
    HTTP_GIF,                // File is GIF image (extension .gif)
    HTTP_PNG,                // File is PNG image (extension .png)
    HTTP_JPG,                // File is JPG image (extension .jpg)
    HTTP_JAVA,               // File is java (extension .java)
    HTTP_WAV,                // File is audio (extension .wav)
    HTTP_UNKNOWN             // File type is unknown

} HTTP_FILE_TYPE;


// Server variables
// File type extensions corresponding to HTTP_FILE_TYPE
static const code char* http_file_extensions[HTTP_UNKNOWN+1] =
{
    "txt",                       // HTTP_TXT
    "htm",                       // HTTP_HTM
    "html",                      // HTTP_HTML
    "cgi",                       // HTTP_CGI
    "xml",                       // HTTP_XML
    "css",                       // HTTP_CSS
    "gif",                       // HTTP_GIF
    "png",                       // HTTP_PNG
    "jpg",                       // HTTP_JPG
    "js",                        // HTTP_JAVA
    "wav",                       // HTTP_WAV
    "\0\0\0"                     // HTTP_UNKNOWN
};

// Content-type strings corresponding to HTTP_FILE_TYPE
static const code char* http_content_types[HTTP_UNKNOWN+1] =
{
    "text/plain\r\n",            // HTTP_TXT
    "text/html\r\n",             // HTTP_HTM
    "text/html\r\n",             // HTTP_HTML
    "text/html\r\n",             // HTTP_CGI
    "text/xml\r\n",              // HTTP_XML
    "text/css\r\n",              // HTTP_CSS
    "image/gif\r\n",             // HTTP_GIF
    "image/png\r\n",             // HTTP_PNG
    "image/jpeg\r\n",            // HTTP_JPG
    "application/x-javascript\r\n",  // HTTP_JAVA
    "audio/x-wave\r\n",          // HTTP_WAV
    ""                       // HTTP_UNKNOWN
};

// Strings
static const code char* http_response_version   = "HTTP/1.1 ";
static const code char* http_response_OK        = "200 OK\r\n";
static const code char* http_response_unauth    = "401 Unauthorized\r\n";
static const code char* http_response_not_found = "404 Not Found\r\n";
static const code char* http_response_not_avail = "503 Service Unavailable\r\n";
static const code char* http_response_server    = "Server: Alphaloewe/1.1\r\n";
static const code char* http_response_connect   = "Connection: keep-alive\n\n";
static const code char* http_response_length    = "Content-Length: ";
static const code char* http_response_content   = "Content-Type: ";
static const code char* http_response_date      = "Date: ";
static const code char* http_response_end       = "\r\n\r\n";

// Buffer that will be used to transmit
//static char header_buffer[256];
#ifdef NETWORK_USE_GMT_TIMESTAMP
char* ( *update_GMT_timestamp )( void ) = NULL;
#endif
/********************************
 *   Private Prototypes
 ********************************/
static void header_get_page_info( socket_t* socket );

/********************************
 *   Private Functions
 *******************************/
static void header_get_page_info( socket_t* socket )
{
    char* p_file_extension, *p_buffer = socket->get_post_buffer;
    int index;
    
    strcat_const( p_buffer, http_response_length );
    strcat( p_buffer, socket->page->page_size_str );
    strcat( p_buffer, "\r\n" );
    strcat_const( p_buffer, http_response_content );
    //TODO Content type
    p_file_extension = strchr( socket->page->page_name, '.' );
    p_file_extension++;
    
    for( index = 0; index < HTTP_UNKNOWN; index++ )
    {
        if( strncmp_const( p_file_extension, http_file_extensions[index], 3 ) == 0 )
        {
            strcat_const( p_buffer, http_content_types[index] );
            return;
        }
    }
}


/*********************************
 *   Functions
 ********************************/
int header_generate( socket_t* socket )
{
    char* p_buffer = socket->get_post_buffer;
    //memset( header_buffer, 0, 256 );
    memset( p_buffer, 0, LARGE_BUFFER_SIZE );
    
    strcpy_const( p_buffer, http_response_version );

    switch( socket->response_type )
    {
        case RESPONSE_200_OK:
            strcat_const( p_buffer, http_response_OK );
            break;
        case RESPONSE_401_NOT_AUTHORIZED:
            strcat_const( p_buffer, http_response_unauth );
            break;
        case RESPONSE_404_NOT_FOUND:
            strcat_const( p_buffer, http_response_not_found );
            break;
        case RESPONSE_503_NOT_AVAILABLE:
            strcat_const( p_buffer, http_response_not_avail );
            break;
    }

    strcat_const( p_buffer, http_response_server );
    
    #ifdef NETWORK_USE_GMT_TIMESTAMP
    if( update_GMT_timestamp != NULL )
    {
        strcat_const( p_buffer, http_response_date );
        strcat( p_buffer, update_GMT_timestamp() );
        strcat( p_buffer, "\r\n" );
    }
    #endif

    if( socket->page != NULL )
    {
        header_get_page_info( socket );
    }

    strcat_const( p_buffer, http_response_connect );
    //strcat_const( header_buffer, http_response_end );
    
    return 0;
}

#ifdef NETWORK_USE_GMT_TIMESTAMP
void header_set_GMT_callback( char* ( *update_gmt_string )( void ) )
{
     update_GMT_timestamp = update_gmt_string;
}
#endif
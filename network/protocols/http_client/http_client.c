#include "http_client.h"
#include <stdbool.h>
#include <stddef.h>
#include "../../parser/line_parser.h"
#include "../../../utils/alt_string.h"
#include "../../network_conf.h"

#ifdef NETWORK_USE_FAT32
#include "__Lib_FAT32.h"
#endif

#ifdef NETWORK_USE_LOGGER
#include "logger.h"
#else
#define log_event( event, text ) asm nop
#endif

/*!
 *  \struct Holds server request related variables
 *
 */
typedef struct
{
    // Send flag
    bool request_send_flag;
    // Ethernet Socket
    SOCKET_28j60_Dsc* eth_socket_download;
    // File length
    //uint32_t file_length;
    // Transmit buffer
    char request_buffer[ MAX_REQUEST_SIZE ];
    // Capture length
    uint32_t capture_length;
    // Position counter for chunked encoding
    uint32_t chunked_position;
    // Content length - may not be needed.
    uint16_t remaining;
    // Swap handle
    __HANDLE swap_handle;
    // Downloaded file handle
    __HANDLE download_handle;
    // Start of swap sector
    __SECTOR swap_sector;

} client_request_t;

/******************************
 *  Globals
 *****************************/
// Request struct
static client_request_t client_request;
static char downloaded_filename[ FILE_NAME_SIZE ];

// Strings used in header parsing - needs to be in line parser
static const code char* http_request_host       = " HTTP/1.1\r\nHost: ";
static const code char* http_request_end        = "\r\nConnection: keep-alive\r\nAccept: */*\r\n\r\n";

// Webserver Callbacks
static void ( *received_file_callback )( char* file_name ) = NULL;

/************************************
 *     Server Request functions
 ***********************************/
// Step 1
static void send_request( socket_t* const socket );
// Step 2
static void get_response( socket_t* const socket );
// Step 4
static void open_file   ( socket_t* const socket );
// Step 5
static void get_data    ( socket_t* const socket );


/************************************
 *        Server Request Functions
 ***********************************/
/*!
 *  \brief SERVER Step 1
 *
 *  \param socket_t* const socket
 *
 *  \note
 *   <notes>
 */
static void send_request( socket_t* const socket )
{
    // Step 1. The request needs to be sent to the server
    Net_Ethernet_28j60_putBytesTCP( client_request.request_buffer,
                                    strlen( client_request.request_buffer ),
                                    socket->eth_socket );
    socket->socket_state = SM_HTTP_SVR_GET_RESPONSE;
}

/*!
 *  \brief SERVER Step 2
 *
 *  \param socket_t* const socket
 *
 *  \note
 *   <notes>
 */
static void get_response( socket_t* const socket )
{
    if( ( socket->eth_socket->state != 3 ) && ( socket->eth_socket->dataLength == 0 ) )
    {
        close_client_socket( socket );
    }

    else
    {
        uint16_t len = socket->eth_socket->dataLength;
        char buffer;
        uint8_t termination = 0;

        while( ( len-- > 0 ) && ( socket->buffer_position < LARGE_BUFFER_SIZE ) )
        {
            buffer = Net_Ethernet_28j60_getByte();

            switch( buffer )
            {
                case '\r':  // Get rid of those nasty carriage returns
                    break;

                case '\n':  // Ah, found a line feed.
                    termination++;
                    socket->buffer_large[socket->buffer_position] = '\0';

                    if( ( socket->buffer_position > 10 ) )
                    {               
                        if( lineparse_server_response( socket ) == LINEPARSE_ERROR_NOT_VALID )
                        {
                            socket->socket_state = SM_HTTP_SVR_DISCONNECT;
                            return;
                        }
                    }

                    // If \n\n is found, then terminate the loop and parse
                    if( termination == 2 )
                    {
                        if( socket->encoding_type == ENCODING_CHUNKED )
                        {
                            char txt_buffer[50];
                            len -= lineparse_get_chunked_size( socket, &client_request.chunked_position );
                            //sprintl( txt_buffer, "First chunk size: %u\r\n", client_request.chunked_position );
                            //UART1_Write_Text( txt_buffer );
                        }

                        // Transfer remaining bytes to swapfile
                        client_request.remaining = len;
                        socket->socket_state = SM_HTTP_SVR_OPEN_FILE;
                        return;
                    }

                    socket->buffer_position = 0;
                    break;

                default:
                    socket->buffer_large[socket->buffer_position++] = buffer;
                    termination = 0;
            }
        }
    }
}

/*!
 *  \brief SERVER Step 4
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void open_file( socket_t* const socket )
{
    // SWAP_FILE creation and opening
    if( FAT32_Exists( SWAP_FILE ) == 1 )
    {
        if( FAT32_Delete( SWAP_FILE ) != 0 )
        {
            log_event( LOG_ERROR, "Could not delete download swap file" );
            socket->socket_state = SM_HTTP_SVR_DISCONNECT;
            return;
        }
    }

    // Create a swapfile
    if( FAT32_MakeSwap( SWAP_FILE, ( ( MAX_FILE_DOWNLOAD_SIZE / SECTOR_SIZE ) + 1 ), &client_request.swap_sector ) == 0 )
    {
        client_request.swap_handle = FAT32_Open( SWAP_FILE, FILE_WRITE );

        if( client_request.swap_handle < 0 )
        {
            log_event( LOG_ERROR, "Could not open new download swap" );
            socket->socket_state = SM_HTTP_SVR_DISCONNECT;
            return;
        }

        else
        {
            // Hold length of packet datalength, this SHOULD be less than MY_MSS_SIZE
            uint16_t len = client_request.remaining;
            uint8_t* const buffer = socket->buffer_large;
            
            memset( socket->buffer_large, 0, LARGE_BUFFER_SIZE );

            // Retrieve the entire buffer remaining from the first read
            Net_Ethernet_28j60_getBytes( buffer, 0xFFFF, len );

            // Commit to SWAP_FILE
            if( FAT32_WriteSwap( client_request.swap_handle, buffer, len ) != 0 )
            {
                socket->socket_state = SM_HTTP_SVR_DISCONNECT;
                return;
            }

            else
            {
                // Must add to capture length total
                //client_request.capture_length += ( uint32_t )len;
                client_request.capture_length += ( uint32_t )len;
                
                if( socket->encoding_type == ENCODING_CHUNKED )
                {
                    client_request.chunked_position -= ( uint32_t )len;
                }
                
                // TODO: What if size of file is already complete here?
                
                socket->socket_state = SM_HTTP_SVR_GET_DATA;
            }
        }
    }

    else
    {
        socket->socket_state = SM_HTTP_SVR_DISCONNECT;
    }

}

/*!
 *  \brief SERVER Step 5
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void get_data( socket_t* const socket )
{
    uint8_t* const buffer = socket->buffer_large;
    // Hold length of packet datalength
    uint16_t len           = socket->eth_socket->dataLength;
    bool download_flag     = FALSE;
    
    if( ( socket->eth_socket->state != 3 ) )
    {
        UART1_Write_Text( "Premature\r\n" );
        log_event( LOG_ERROR, "Download ended prematurely" );
        socket->socket_state = SM_HTTP_SVR_DISCONNECT;
        return;
    }

    // Chunked encoding implimentation
    if( socket->encoding_type == ENCODING_CHUNKED )
    {
        int buffer_index = 0;

        while( len-- > 0 )
        {
            if( client_request.chunked_position == 0 )
            {
                UART1_Write_Text( "Position reached zero\r\n" );
                len -= lineparse_get_chunked_size( socket, &client_request.chunked_position );

                if( client_request.chunked_position == 0 )
                {
                    download_flag = TRUE;
                }
            }

            else
            {
                buffer[ buffer_index++ ] = Net_Ethernet_28j60_getByte();
                client_request.chunked_position--;
            }
        }
        
        len = buffer_index;
        
        //len = ( uint16_t )strlen( buffer );
    }

    else
    {
        Net_Ethernet_28j60_getBytes( buffer, 0xFFFF, len );  // Get entire window and copy to buffer
    }

    // Write all the data to the swap
    if( FAT32_WriteSwap( client_request.swap_handle, buffer, len ) != 0 )
    {
        UART1_Write_Text( "Swap could not be written\r\n" );
        socket->socket_state = SM_HTTP_SVR_DISCONNECT;
    }

    else
    {
        client_request.capture_length += ( uint32_t )len;        // Increase capture count

        if( socket->content_length > 0 )
        {
            if( client_request.capture_length >= socket->content_length )
            {
                UART1_Write_Text( "Complete File Downloaded\r\n" );
                download_flag = TRUE;
            }
        }
        
        if( download_flag == TRUE )
        {
            UART1_Write_Text( "Download complete\r\n" );
            socket->socket_state = SM_HTTP_SVR_DOWNLOAD_COMPLETE;
        }
    }
}


/*!
 *  \brief Close server socket
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void close_client_socket( socket_t* const socket )
{
    if( Net_Ethernet_28j60_disconnectTCP( socket->eth_socket ) == 1 )
    {
        bool file_ready = FALSE;

        // To keep filesystem integrity we need to do a good job of making sure we don't leave files open
        if( FAT32_GetFileHandle( SWAP_FILE, &client_request.swap_handle ) != -1 )
        {
            // Issue close if open
            if( FAT32_Close( client_request.swap_handle ) == -1 )
            {
                client_request.capture_length = 0;
                UART1_Write_Text( "Could not close swap file\r\n" );
            }
        }
        
        // Represents we have bytes on the buffer to write
        if( client_request.capture_length > 0 )
        {
            if( FAT32_Exists( SWAP_FILE ) == 1 )
            {
                if( FAT32_Exists( socket->file_name ) == 1 )
                {
                    FAT32_Delete( socket->file_name );
                }

                client_request.swap_handle = FAT32_Open( SWAP_FILE, FILE_READ );              // Open swap for reading
                client_request.download_handle = FAT32_Open( socket->file_name, FILE_WRITE );
                
                if( ( client_request.swap_handle == -1 ) || ( client_request.download_handle == -1 ) )
                {
                    FAT32_Close( client_request.swap_handle );
                    FAT32_Close( client_request.download_handle );
                    FAT32_Delete( SWAP_FILE );
                }

                else
                {
                    // Read and write 1 sector at a time, this is the buffer to hold it
                    char* p_data_buffer;
                    uint16_t sectors, remain;

                    p_data_buffer = ( char* )Malloc( SECTOR_IN_BYTES * sizeof( char ) );

                    if( p_data_buffer == NULL )
                    {
                        UART1_Write_Text( "No mem for swap\r\n" );
                        return;
                    }

                    sectors = ( client_request.capture_length / SECTOR_IN_BYTES ); // How many sectors to write
                    remain  = ( client_request.capture_length % SECTOR_IN_BYTES ); // How many remaining bytes

                    while( sectors-- != 0 )
                    {
                        FAT32_ReadSwap( client_request.swap_handle, p_data_buffer, SECTOR_IN_BYTES );
                        FAT32_Write   ( client_request.download_handle, p_data_buffer, SECTOR_IN_BYTES );
                    }

                    // These are the remaining bytes to read and write
                    FAT32_ReadSwap( client_request.swap_handle,     p_data_buffer, remain );
                    FAT32_Write   ( client_request.download_handle, p_data_buffer, remain );
                    // CRITICAL!!! CLOSE FILE HANDLES
                    FAT32_Close ( client_request.swap_handle );
                    FAT32_Close ( client_request.download_handle );
                    // Make sure we delete SWAP_FILE
                    FAT32_Delete( SWAP_FILE );
                    // Copy our new downloaded file name to the global
                    strcpy( downloaded_filename, socket->file_name );
                    // Free the memory
                    Free( ( char* )p_data_buffer, ( SECTOR_IN_BYTES * sizeof( char ) ) );

                    file_ready = TRUE;
                }
            }
        }

        // Remove the socket from the pool
        socket_manager_close( socket );
        // Reset the client request
        memset( &client_request, 0, sizeof( client_request_t ) );
        // Woohoo! Got a file!
        if( file_ready == TRUE )
        {
            if( received_file_callback != NULL )
            {
                received_file_callback( downloaded_filename );
            }
        }
    }
}



/*!
 *  \brief Requests a file from a remote server
 *
 *  \param char* - request Needs to be formatted http://www.happyserver.com/myfile.pdf etc...
 *  \param uint16_t - port number of port to be used on request.
 *
 *  \return Success of resolution and generation of request
 *    \retval 0 - Success
 *    \retval -1 - Failed
 *
 */
int http_client_request_file( char* const request, uint16_t remote_port )
{
    if( client_request.eth_socket_download != NULL )
    {
        log_event( LOG_ERROR, "Attempted concurrent download" );
        return -1;
    }
    
    else if( strncmp( request, "http://", 7 ) != 0 )       // Check to see if the request is formatted correctly, must begin with http://
    {
        log_event( LOG_ERROR, "Invalid request formed" );
        return -1;
    }

    else
    {
        char* p_remoteIP = NULL, *p_request = NULL, domain[ DOMAIN_SIZE ] = {0};
        int tries = CONNECT_ATTEMPTS, domainCount = 0;
        
        // Clear global server request
        memset( &client_request, 0, sizeof( client_request_t ) );

        // Forward pointer past the http:
        p_request = strstr( request, "//" );
        
        if( p_request == NULL )
        {
            return -1;
        }
        
        // Now forward 2 more to get ahead of the forward slashes
        p_request += 2;

        // Past the slashes is the domain lets get it!
        while( ( *p_request != '/' ) && ( domainCount < DOMAIN_SIZE ) )
        {
            domain[ domainCount++ ] = *( p_request++ );
        }

        // Just incase, lets add that termination char
        domain[ domainCount ] = '\0';

        // TODO: if this is the same as last time, no need to resolve, speed things up
        do
        {
            p_remoteIP = Net_Ethernet_28j60_dnsResolve( domain, 5 );
        }
        while( ( p_remoteIP == NULL ) && ( tries-- > 0 ) );

        /* We know that if the tries variable is greater than 0, a successful
           DNS resolution happened */
        if( tries > 0 )
        {
            char remoteIPAddr[4] = {0}, *p_buffer;
            socket_t* client_socket = socket_manager_find_socket( client_request.eth_socket_download );
            int i;

            // Very important with dynamic memory
            if( client_socket == NULL )
            {
                log_event( LOG_ERROR, "Failed to allocate socket in request" );
                return -1;
            }

            // Use the socket buffer to contain the request
            p_buffer = client_request.request_buffer;

            strcpy      ( p_buffer, "GET " );
            strcat      ( p_buffer, p_request );
            strcat_const( p_buffer, http_request_host );
            strcat      ( p_buffer, domain );
            strcat_const( p_buffer, http_request_end );

            // TODO: Works, but needs to be revised. Get filename if in request
            for( i = 0; request[i]; i++ )
            {
                if( request[i] == '/' )
                {
                    if( request[i+1] )
                    {
                        p_request = &request[i+1];
                    }
                }
            }

            if( strchr( p_request, '.' ) != 0 )
            {
                strcpy( client_socket->file_name, p_request );
            }

            // Reset tries
            tries = CONNECT_ATTEMPTS;
            memcpy( remoteIPAddr, p_remoteIP, 4 );
            // Set state to transmit request
            client_socket->socket_state = SM_HTTP_SVR_SEND_REQUEST;
            // Set flag that represents we are requesting a file
            client_request.request_send_flag = TRUE;

            /* Connect to remote server, passing the address of the pointer that will be used for the response */
            while( ( Net_Ethernet_28j60_connectTCP( remoteIPAddr, 
                                                    remote_port, 
                                                    TCP_PORT_REQUEST,
                                                    &client_request.eth_socket_download ) != 1 )
                                                    && ( tries-- > 0 ) );

            if( tries > 0 )
            {
                client_socket->eth_socket = client_request.eth_socket_download;
                return 0;
            }

            else
            {
                close_client_socket( client_socket );
                client_request.eth_socket_download = NULL;
                return -1;
            }
        }

        else
        {
            return -1;
        }

    }
}

/*!
 *  \brief Sets function callback when file download is received
 *
 *  \note
 *   <notes>
 */
void http_client_set_file_callback( void ( *receive_file )( char* file_name ) )
{
    received_file_callback = receive_file;
}

/*!
 *  \brief Main entry point for socket handling
 *
 *  \param SOCKET_Dsc* eth_socket - socket passed by main entry point
 *
 *  \note
 *   <notes>
 */
int http_client_handle( SOCKET_28j60_Dsc* eth_socket )
{
    // If socket doesn't exist in catalog it will initialize a new one
    socket_t* current_socket = socket_manager_find_socket( eth_socket );

    if( current_socket == NULL )  // VERY important with dynamic memory allocation
    {
        return -1;
    }

    else
    {
        current_socket->alive_time = 0;
        
        switch( current_socket->socket_state )
        {
            case SM_HTTP_SVR_SEND_REQUEST:
                // Step 1. Send the request for the file to the server
                send_request( current_socket );
                break;

            case SM_HTTP_SVR_GET_RESPONSE:
                // Step 2. Retrieve the server response and parse
                get_response( current_socket );

                if( current_socket->socket_state != SM_HTTP_SVR_OPEN_FILE )
                {
                    break;
                }

            case SM_HTTP_SVR_OPEN_FILE:
                // Step 3. Open the swapfile, take the remainder of the window and write to swap
                open_file( current_socket );

                if( current_socket->socket_state != SM_HTTP_SVR_GET_DATA )
                {
                    current_socket->socket_state = SM_HTTP_SVR_DISCONNECT;
                }
                // We need to break here because we "should" have, a empty frame buffer
                break;

            case SM_HTTP_SVR_GET_DATA:
                // Step 4. Get all the data from the server
                get_data( current_socket );
                
                if( current_socket->socket_state != SM_HTTP_SVR_DOWNLOAD_COMPLETE )
                {
                    break; // Keep returning to this state until download is complete
                }

            case SM_HTTP_SVR_DOWNLOAD_COMPLETE:
                // Step 5. Close the socket will also write any captured bytes into a file on the SD Card
                close_client_socket( current_socket );

            case SM_HTTP_SVR_DISCONNECT:

                close_client_socket( current_socket );
                break;
            default:
                close_client_socket( current_socket );

        }

    }

}

/*!
 *  \brief Polled function to detect incoming server response
 *
 *  \pre Request needs to be made previously
 *
 *  \note
 *   <notes>
 */
void http_client_request_pending()
{
    // If request has been made AND socket state is CONNECTED, start send ACK packet
    if( ( client_request.eth_socket_download->state == 3 ) && client_request.request_send_flag == TRUE )
    {
        if( Net_Ethernet_28j60_startSendTCP( client_request.eth_socket_download ) == 1 )
        {
            client_request.request_send_flag = FALSE;
        }
    }
}
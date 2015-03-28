#include "http_server.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../page_manager/page_manager.h"
#include "../../head_generator/head_generator.h"
#include "../../parser/line_parser.h"

#ifdef NETWORK_USE_FAT32
#include "__Lib_FAT32.h"
#endif

#ifdef NETWORK_USE_LOGGER
#include "logger.h"
#else
#define log_event( event, text ) asm nop
#endif

// Post variables
char post_response[ MAX_POST_SIZE ];
uint16_t post_capture;
// Callback
static void ( *received_form_callback )( char* form_data, char* form_name ) = NULL;
static void ( *received_post_callback )( char* post_data, char* form_name ) = NULL;
/******************************
 *   Private Prototypes
 *****************************/
// Step 1
static void get_client_request  ( socket_t* const socket );
// Step 2a
static void process_request     ( socket_t* const socket );
// Step 3
static void serve_header        ( socket_t* const socket );
// Step 4
static void find_requested_file ( socket_t* const socket );
// Step 5
static void serve_body          ( socket_t* const socket );

/*******************************
 * Client TCP handler functions
 ******************************/
/*!
 *  \brief Step 1 - Get Client Request
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void get_client_request( socket_t* const socket )
{

    if( ( socket->eth_socket->state != 3 ) || ( socket->eth_socket->dataLength == 0 ) )
    {
        close_server_socket( socket );
    }

    else
    {
        uint16_t len = socket->eth_socket->dataLength;
        char buffer;
        int termination = 0;

        while( ( len-- > 0 ) && ( socket->buffer_position < LARGE_BUFFER_SIZE ) )
        {
            buffer = Net_Ethernet_28j60_getByte(); // Get byte at a time

            switch( buffer )
            {
                case '\r': // Remove carriage return
                    break;

                case '\n':
                    termination++;
                    socket->buffer_large[ socket->buffer_position ] = '\0';
                    // Only if at least 10 chars are found, parse
                    if( ( socket->buffer_position > 10 )  )
                    {
                        if( lineparse_client_request( socket ) == LINEPARSE_ERROR_NOT_VALID )
                        {
                             socket->socket_state = SM_HTTP_DISCONNECT;
                             return;
                        }
                    }
                    // If termination happens ( \n\n )
                    if( termination == 2 )
                    {
                        switch( socket->request_type )
                        {
                            case REQUEST_GET:
                                // Found GET request
                                socket->socket_state = SM_HTTP_PROCESS_REQUEST;
                                return;

                            case REQUEST_POST:
                                // Found POST request
                                if( socket->content_length > MAX_POST_SIZE )
                                {
                                    socket->socket_state = SM_HTTP_DISCONNECT;
                                    return;
                                }
                                
                                memset( post_response, 0, MAX_POST_SIZE );
                                post_capture = 0;
                                lineparse_process_client( socket );
                                // Retrieve remaining bytes on the buffer
                                Net_Ethernet_28j60_getBytes( post_response, 0xFFFF, len );
                                post_capture += len;
                                socket->socket_state = SM_HTTP_PROCESS_REQUEST;
                                return;
                            case REQUEST_PUT:
                                //socket->socket_state = SM_HTTP_PROCESS_REQUEST;
                                // TODO: Not yet supported
                                return;
                            default:
                                // Nope, nothing here
                                socket->socket_state = SM_HTTP_DISCONNECT;
                                return;
                        }
                    }

                    else
                    {
                        // Reset buffer position
                        socket->buffer_position = 0;
                        // May not be nessasary
                        memset( socket->buffer_large, 0, LARGE_BUFFER_SIZE );
                    }

                    break;

                default:
                    termination = 0;
                    socket->buffer_large[socket->buffer_position++] = buffer;
            
            }
        
        }
        
        if( socket->buffer_position == LARGE_BUFFER_SIZE )
        {
              socket->socket_state = SM_HTTP_DISCONNECT;
        }
    
    }

}


/*!
 *  \brief Step 2a - Process GET request
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void process_request( socket_t* const socket )
{
    switch( socket->request_type )
    {
        case REQUEST_GET:
            // Line parse simply getes the filename for GET but
            switch( lineparse_process_client( socket ) )
            {
                case LINEPARSE_OK_FILE:
                    // Find file
                    socket->socket_state = SM_HTTP_FIND_FILE;
                    break;
                case LINEPARSE_OK_FORM:
                    // Parse a form
                    if( received_form_callback != NULL )
                    {
                        received_form_callback( socket->get_post_buffer, socket->file_name );
                    }
                    
                    socket->socket_state = SM_HTTP_FIND_FILE;
                    break;
                case LINEPARSE_ERROR_NOT_VALID:
                    // Disconnect socket
                    socket->socket_state = SM_HTTP_DISCONNECT;
                    break;
            }

            memset( socket->buffer_large, 0, LARGE_BUFFER_SIZE );
            break;
            
        case REQUEST_POST:
            // Ok, so if dataLength is not 0, that means read?
            if( socket->eth_socket->dataLength != 0 )
            {
                char* p_post = post_response;
                p_post += post_capture;

                Net_Ethernet_28j60_getBytes( socket->buffer_large, 0xFFFF, socket->eth_socket->dataLength );
                
                strncpy( p_post, socket->buffer_large, socket->eth_socket->dataLength );
                post_capture += socket->eth_socket->dataLength;
            }
            // If we have a complete capture, then let's process
            if( post_capture >= socket->content_length )
            {
                if( received_post_callback != NULL )
                {
                    received_post_callback( &post_response, socket->file_name );
                }
                
                socket->socket_state = SM_HTTP_FIND_FILE;
            }

            else
            {
                return;
            }
        
        break;
        case REQUEST_PUT:
        
        break;
    }
}


/*!
 *  \brief Step 4 - Validate file
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void find_requested_file( socket_t* const socket )
{
    // Assign a webpage
    socket->page = page_manager_find_page( socket->file_name );

    if( socket->page == NULL )
    {
        socket->response_type = RESPONSE_404_NOT_FOUND;
    }

    else
    {
        char txt_buffer[12];     // Buffer used for converting size to string

        switch ( socket->page->storage_type )
        {
            case PAGE_STATIC:
                // Static page
                socket->response_type = RESPONSE_200_OK;
                socket->page->static_page.current_ptr = socket->page->static_page.start_ptr;
                break;

            case PAGE_DYNAMIC:
            {
                uint32_t p_size;         // Variable for calculating size of dynamic page
                // Callback that updates data
                socket->page->dynamic_page.page_update();
                // Refresh size of dynamic page
                p_size = strlen( socket->page->dynamic_page.start_ptr );
                // Convert size to string
                LongToStr( p_size, txt_buffer );
                strcpy( socket->page->page_size_str, Ltrim( txt_buffer ) );
                // Refresh dynamic page end pointer
                socket->page->dynamic_page.end_ptr = socket->page->dynamic_page.start_ptr + p_size;
                socket->page->dynamic_page.current_ptr = socket->page->dynamic_page.start_ptr;
                socket->response_type = RESPONSE_200_OK;
            }

            break;
#ifdef NETWORK_USE_FAT32

            case PAGE_FLASH:
                // Open file
                socket->page->flash_page.file_handle = FAT32_Open( socket->page->page_name, FILE_READ );

                // If file cannot be opened then file not found
                if( socket->page->flash_page.file_handle < 0 )
                {
                    socket->response_type = RESPONSE_404_NOT_FOUND;
                    socket->page = NULL;
                }

                else
                {

                    if( socket->page->flash_page.dynamic_filesize == TRUE )
                    {
                        // Get file size
                        FAT32_Size( socket->page->page_name, &socket->page->flash_page.file_size );
                        // Get string of size
                        LongToStr( socket->page->flash_page.file_size, txt_buffer );
                        // Copy size string to webpage
                        strcpy( socket->page->page_size_str, Ltrim( txt_buffer ) );
                    }

                    FAT32_Read( socket->page->flash_page.file_handle, socket->buffer_large, LARGE_BUFFER_SIZE );
                    socket->page->flash_page.buffer_position = 0;

                }

                socket->response_type = RESPONSE_200_OK;

                break;
#endif
        }
    }

    // Ready for generation of header
    socket->socket_state = SM_HTTP_SERVE_HEADERS;
}


static void serve_header( socket_t* const socket )
{
    header_generate( socket );
    Net_Ethernet_28j60_putStringTCP( socket->get_post_buffer, socket->eth_socket );
    socket->socket_state = SM_HTTP_SERVE_BODY;
}

/*!
 *  \brief Step 6 - Serve body
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void serve_body( socket_t* const socket )
{
    bool done_flag = FALSE;              // Flag that represents done sending body
    uint16_t remaining_bytes;            // Bytes remaining from current to end

    if( socket->page == NULL || socket->eth_socket->state != 3 )
    {
        //close_server_socket( socket );
        Net_Ethernet_28j60_putStringTCP( "File Not Found\r\n", socket->eth_socket );
        socket->socket_state = SM_HTTP_WAIT_TILL_DONE;
        return;
    }

    switch( socket->page->storage_type )
    {
        case PAGE_STATIC:
            // Get remaining bytes
            remaining_bytes = ( socket->page->static_page.end_ptr - socket->page->static_page.current_ptr );
            // Increments current pointer by how many bytes were successfully written to buffer
            socket->page->static_page.current_ptr += Net_Ethernet_28j60_putConstBytesTCP(
                        socket->page->static_page.current_ptr,
                        remaining_bytes,
                        socket->eth_socket );

            // If current pointer == end pointer, transmit complete
            if( socket->page->static_page.current_ptr == socket->page->static_page.end_ptr )
            {
                done_flag = TRUE;
                socket->page->static_page.current_ptr = socket->page->static_page.start_ptr;
            }

            break;

        case PAGE_DYNAMIC:
            // Get remaining bytes
            remaining_bytes = ( socket->page->dynamic_page.end_ptr - socket->page->dynamic_page.current_ptr );
            // Increments current pointer by how many bytes were successfully written to buffer
            socket->page->dynamic_page.current_ptr += Net_Ethernet_28j60_putBytesTCP(
                        socket->page->dynamic_page.current_ptr,
                        remaining_bytes,
                        socket->eth_socket );

            // If current pointer == end pointer, transmit complete
            if( socket->page->dynamic_page.current_ptr == socket->page->dynamic_page.end_ptr )
            {
                done_flag = TRUE;
                socket->page->dynamic_page.current_ptr = socket->page->dynamic_page.start_ptr;
            }

            break;
#ifdef NETWORK_USE_FAT32

        case PAGE_FLASH:

            //remaining_bytes = Net_Ethernet_Intern_bufferFreeSize( socket->eth_socket );
            while( Net_Ethernet_28j60_putByteTCP( socket->buffer_large[socket->page->flash_page.buffer_position],
                                            socket->eth_socket ) != 0 )
            {
                if( socket->page->flash_page.buffer_position == LARGE_BUFFER_SIZE-1 )
                {
                    FAT32_Read( socket->page->flash_page.file_handle, socket->buffer_large, LARGE_BUFFER_SIZE );
                    socket->page->flash_page.buffer_position = 0;
                }

                else
                {
                    socket->page->flash_page.buffer_position++;
                    socket->page->flash_page.file_position++;
                }

                // If file position == file size, done!
                if( socket->page->flash_page.file_position >= socket->page->flash_page.file_size )
                {
                    done_flag = TRUE;
                    socket->page->flash_page.file_position = 0;
                    FAT32_Close( socket->page->flash_page.file_handle );
                    break;
                }
            }

            break;
#endif
    }

    // Checks the flag, if TRUE, transmission of body complete
    if( done_flag == TRUE )
    {
        socket->socket_state = SM_HTTP_WAIT_TILL_DONE;
    }
}

/*!
 *  \brief Step 8 - Close connection
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
void close_server_socket( socket_t* const socket )
{
    // If socket happens to be download socket
    if( Net_Ethernet_28j60_disconnectTCP( socket->eth_socket ) == 1 )
    {
        // If the page was found, it has a page assigned, if not it would be a null reference
        if( socket->page != NULL )
        {
            switch( socket->page->storage_type )
            {
                case PAGE_STATIC:
                    // Reset the current pointer to start
                    socket->page->static_page.current_ptr = socket->page->static_page.start_ptr;
                    break;

                case PAGE_DYNAMIC:
                    // Reset the current pointer to start
                    socket->page->dynamic_page.current_ptr = socket->page->dynamic_page.start_ptr;
                    break;
                    #ifdef NETWORK_USE_FAT32
                case PAGE_FLASH:
                    socket->page->flash_page.file_position = LARGE_BUFFER_SIZE-1;

                    if( FAT32_GetFileHandle( socket->page->page_name, &socket->page->flash_page.file_handle ) == 0 )
                    {
                        FAT32_Close( socket->page->flash_page.file_handle );
                    }

                    break;
                    #endif
            }

        }

        socket_manager_close( socket );
    }

    else
    {
        UART1_Write_Text( "Could not close socket\r\n" );
    }

}

/**
 *  @brief Handles incoming requests from clients on http port
 *
 *  @pre <Preconditions that need to be met before called>
 *
 *  @param <Parameters Accepted>
 *
 *  @return <Returns>
 *    @retval <Values that might be returned>
 *
 *  @note
 *   <notes>
 */
int http_handle_client( SOCKET_28j60_Dsc* eth_socket )
{
    socket_t* current_socket = socket_manager_find_socket( eth_socket );

    if( !current_socket )
    {
        return -1;
    }

    current_socket->alive_time = 0;

    switch( current_socket->socket_state )
    {
        case SM_HTTP_IDLE:
            current_socket->socket_state = SM_HTTP_GET_REQUEST;
            
        case SM_HTTP_GET_REQUEST:
            // Step 1. Get request from client
            get_client_request( current_socket );

            if( current_socket->socket_state != SM_HTTP_PROCESS_REQUEST )
            {
                if( current_socket->socket_state == SM_HTTP_DISCONNECT )
                {
                     close_server_socket( current_socket );
                     return;
                }
                
                break;
            } 
            
            else if( current_socket->request_type == REQUEST_POST )
            {
               if( post_capture < current_socket->content_length )
               {
                   return;
               }
            }

        case SM_HTTP_PROCESS_REQUEST:
            // Step 3. Process get
            process_request( current_socket );

            if( current_socket->socket_state != SM_HTTP_FIND_FILE )
            {
                if( current_socket->socket_state == SM_HTTP_DISCONNECT )
                {
                    close_server_socket( current_socket );
                    return;
                }
                
                break;
            }

        case SM_HTTP_FIND_FILE:
            // Step 4. Find file, AVR 43ms
            find_requested_file( current_socket );

        case SM_HTTP_SERVE_HEADERS:
            // Step 5. Server header
            serve_header( current_socket );

        case SM_HTTP_SERVE_BODY:
            // Step 6. Serve body
            serve_body( current_socket );

            if( current_socket->socket_state != SM_HTTP_WAIT_TILL_DONE )
            {
                break;
            }

        case SM_HTTP_WAIT_TILL_DONE:

            // Step 7. Wait until buffer empty
            if( Net_Ethernet_28j60_bufferEmptyTCP( current_socket->eth_socket ) == 1 )
            {
                switch( current_socket->connect_type )
                {
                    case CONNECTION_KEEP_ALIVE:
                    
                        if( current_socket->eth_socket->dataLength != 0 || current_socket->eth_socket->state != 3 )
                        {
                            memset( current_socket->buffer_large, 0, LARGE_BUFFER_SIZE );
                            memset( current_socket->get_post_buffer, 0, GET_POST_SIZE );
                            current_socket->buffer_position = 0;
                            current_socket->socket_state = SM_HTTP_GET_REQUEST;
                            current_socket->page = NULL;
                            break;
                        }

                        else
                        {
                            current_socket->socket_state = SM_HTTP_DISCONNECT;
                        }

                    case CONNECTION_CLOSE:
                        current_socket->socket_state = SM_HTTP_DISCONNECT;
                        break;
                }
            }

            if( current_socket->socket_state != SM_HTTP_DISCONNECT )
            {
                break;
            }

        case SM_HTTP_DISCONNECT:
            // Step 8. Disconnect and remove from list
            close_server_socket( current_socket );
            break;
    }

    return 0;
}

/*!
 *  \brief Sets callback for forms
 *
 *  \note
 *   <notes>
 */
void http_set_form_callback( void ( *receive_form )( char* form_data, char* form_name ) )
{
    received_form_callback = receive_form;
}

/*!
 *  \brief Sets callback for posts
 *
 *  \note
 *   <notes>
 */
void http_set_post_callback( void ( *received_post)( char* post_data, char* file_name ) )
{
    received_post_callback = received_post;
}
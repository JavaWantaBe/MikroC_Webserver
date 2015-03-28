#include "line_parser.h"
#include <stddef.h>
#include "alt_string.h"

/*
 * Example Header Responses:
 *
 * GET /tutorials/other/top-20-mysql-best-practices/ HTTP/1.1
 * Host: net.tutsplus.com
 * User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)
 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9
 * Accept-Language: en-us,en;q=0.5
 * Accept-Encoding: gzip,deflate
 * Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
 * Keep-Alive: 300
 * Connection: keep-alive
 * Cookie: PHPSESSID=r2t5uvjq435r4q7ib3vtdjq120
 * Pragma: no-cache
 * Cache-Control: no-cache
 *
 *
 * POST /foo.php HTTP/1.1
 * Host: localhost
 * User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)
 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9
 * Accept-Language: en-us,en;q=0.5
 * Accept-Encoding: gzip,deflate
 * Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
 * Keep-Alive: 300
 * Connection: keep-alive
 * Referer: http://localhost/test.php
 * Content-Type: application/x-www-form-urlencoded
 * Content-Length: 43
 *
 * first_name=John&last_name=Doe&action=Submit
 *
 *
 *
 */

/*********************************
 *   Private Prototypes
 ********************************/
static void get_file_name( socket_t* socket, char* line );

// Variables
static const code char* http_response_OK        = "HTTP/1.1 200 OK";
static const code char* http_response_OK_old    = "HTTP/1.0 200 OK";

/*********************************
 *   Private Functions
 ********************************/
/*!
 *  \brief Gets the file name from a get request
 *
 */
static void get_file_name( socket_t* socket, char* line )
{
    int i = 0, j = 0;
    char* p_parser = line, *p_temp;
    char file[FILE_NAME_SIZE] = {0};

    do
    {
        p_parser++;
        p_temp = p_parser;
        p_parser = strchr( p_parser, '/' );
    }
    while( p_parser != 0 );

    p_parser = p_temp;

    while( ( *p_parser != '\0' && *p_parser != '.' ) && ( i < 8 ) )
    {
        file[i++] = *p_parser++;
    }

    p_parser = strchr( line, '.' );

    while( ( j < 4 ) && ( *p_parser != '\0' ) )
    {
        file[i++] = *p_parser++;
        j++;
    }

    file[i] = '\0';

    strcpy( socket->file_name, file );

}


/******************************************
 *   Public Functions
 *****************************************/
/*!
 *  \brief Parser for client requests
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *
 */
int lineparse_client_request( socket_t* const socket )
{
    char* const p_buffer = socket->buffer_large;
    
    // Only run if request is unknown
    if( socket->request_type == REQUEST_UNKNOWN )
    {
        if( strncmp( p_buffer, "GET ", 3 ) == 0 )      // GET /foo.php HTTP/1.1
        {
            socket->request_type = REQUEST_GET;
            strcpy( socket->get_post_buffer, p_buffer );

            return LINEPARSE_OK;
        }

        else if( strncmp( p_buffer, "POST", 3 ) == 0 ) // POST /foo.php HTTP/1.1
        {
            socket->request_type = REQUEST_POST;
            strcpy( socket->get_post_buffer, p_buffer );

            return LINEPARSE_OK;
        }
        
        else if( strncmp( p_buffer, "PUT", 3 ) == 0 ) // PUT /user/123 HTTP/1.1
        {
            socket->request_type = REQUEST_PUT;
            strcpy( socket->get_post_buffer, p_buffer );
            
            return LINEPARSE_OK;
        }

        else
        {
            return LINEPARSE_ERROR_NOT_VALID;
        }
    }

    else
    {
        char* p_parser;

        if( strncmp( p_buffer, "Connection", 5 ) == 0 )
        {
            p_parser = strchr( p_buffer, ' ' );

            if( p_parser == NULL )
            {
                return LINEPARSE_ERROR_NOT_VALID;
            }
            
            else if( strncmp( p_parser, " close", 4 ) == 0 )
            {
                socket->connect_type = CONNECTION_CLOSE;
                return LINEPARSE_OK;
            }

            else if( strncmp( p_parser, " keep-alive", 4 ) == 0 )
            {
                socket->connect_type = CONNECTION_KEEP_ALIVE;
                return LINEPARSE_OK;
            }
            
            else
            {
                return LINEPARSE_ERROR_NOT_VALID;
            }

        }

        else if( strncmp( p_buffer, "WWW-Authenticate", 5 ) == 0 )
        {
            return LINEPARSE_OK;
        }
        
        else if( strncmp( p_buffer, "Content-Length", 9 ) == 0 )  // Start of POST support
        {
            // Go to space in : Content-Length: 22036
            p_parser = strchr( p_buffer, ' ' );

            if( p_parser == NULL )
            {
                return LINEPARSE_ERROR_NOT_VALID;
            }

            p_parser++; // Extend past space

            socket->content_length = atol( p_parser );  // Convert to long
        }

    }

    return LINEPARSE_OK;
}


/*!
 *  \brief Process GET request from client
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int lineparse_process_client( socket_t* const socket )
{
    char* const p_buffer = socket->get_post_buffer;
    char* p_parser;

    /* GET /foo.php?first_name=John&last_name=Doe&action=Submit HTTP/1.1 
     * POST /foo.php HTTP/1.1
    */
    p_parser = strchr( p_buffer, ' ' );

    if( p_parser == NULL )
    {
        return LINEPARSE_ERROR_NOT_VALID;
    }

    p_parser++;

    // If no file is requested we automaticly assume it is index.htm
    if( strncmp( p_parser, "/ HTTP/1.1", 4 ) == 0 )
    {
        strcpy( socket->file_name, "index.htm" );
        return LINEPARSE_OK_FILE;
    }

    else
    {
        int i = 0;
        char temp_buffer[ GET_POST_SIZE ] = {0};

        // If we find a ? in the string it could be 1 of two things, FORM or AJAX request
        if( strchr( p_parser, '?' ) != NULL )
        {
            while( ( *p_parser != '?' ) && ( *p_parser != '\0' ) )
            {
                temp_buffer[i++] = *p_parser++;
            }

            temp_buffer[i] = '\0';
            // Get only the file name of the form or ajax request
            get_file_name( socket, temp_buffer );

            p_parser = strchr( p_buffer, '?' );
            p_parser++;
            
            i = 0;
            memset( temp_buffer, 0, GET_POST_SIZE );
            // We only want the data, no filename and no HTTP status.
            while( ( ( *p_parser != ' ' ) && ( *p_parser != '\0' ) ) && ( i < GET_POST_SIZE ) )
            {
                temp_buffer[i++] = *p_parser++;
            }
            // Clear our get buffer because it has the raw request
            memset( socket->get_post_buffer, 0, GET_POST_SIZE );
            // Since we kept track of how many bytes we copied in i, use strncpy
            strncpy( socket->get_post_buffer, temp_buffer, i );
            
            return LINEPARSE_OK_FORM;

        }

        else
        {
            while( ( *p_parser != ' ' ) && ( *p_parser != '\0' ) )
            {
                temp_buffer[i++] = *p_parser++;
            }

            get_file_name( socket, temp_buffer );

            return LINEPARSE_OK_FILE;

        }
    }

    return LINEPARSE_ERROR_NOT_VALID;
}


/***********************************
 *   Server Response Section
 **********************************/
/*!
 *  \brief Parses line by line from server response
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   HTTP/1.1 200 OK
 *   Date: Thu, 18 Jul 2013 03:47:14 GMT
 *   Server: Apache-Coyote/1.1
 *   Content-Disposition: attachment;filename="SOS_HighLowTidePredictions_8454000_2013-07-17T19-43-00Z_2013-08-17T14-19-00Z.csv"
 *   Content-Type: application/vnd.ms-excel;charset=ISO-8859-1
 *   Set-Cookie: SERVERID=A; path=/
 *   Connection: close
 *   Transfer-Encoding: chunked
 */
int lineparse_server_response( socket_t* const socket )
{
    char* const p_buffer = socket->buffer_large;
    char* ptr;

    if( socket->response_type == RESPONSE_UNKNOWN )
    {
        // Looking for a OK in the first line, but we don't want to compare this one every iteration
        if( ( strncmp_const( p_buffer, http_response_OK , 13 ) == 0 )
                || ( strncmp_const( p_buffer, http_response_OK_old , 13 ) == 0 ) )
        {
            socket->response_type = RESPONSE_200_OK;
            return LINEPARSE_OK;
        }
    }

    switch( p_buffer[ 0 ] )
    {
        case 'C':
            switch( p_buffer[ 8 ] )
            {
                // Content-Disposition is where we abtain the file name from the attackment token
                case 'D':
                {
                    int i, j;

                    ptr = strchr( p_buffer, '"' ); // Move to quotation after attachement;

                    // If we don't have quotes, then this isn't a valid file attachment
                    if( ptr == NULL )
                    {
                        return LINEPARSE_ERROR_NOT_VALID;
                    }

                    ptr++;  // Move past the quotation

                    // This while loop will truncate the filename if more than 8 chars
                    for( i = 0; i < 8 && *ptr != '.'; i++, ptr++ )
                    {
                       socket->file_name[i] = *ptr;
                    }

                    // Fast forward until we do find an extension
                    if( *ptr != '.' && *ptr != '\0' )
                    {
                        while( *( ++ptr ) != '.' );
                    }

                    // Add the period or 0 if null
                    socket->file_name[i++] = *ptr++;

                    // Add the extension onto the filename if present
                    for( j = 0; j < 3 && *ptr != '"' && *ptr != '\0'; j++, i++, ptr++ )
                    {
                        socket->file_name[i] = *ptr;
                    }
                    
                    // Finally, terminate the filename
                    socket->file_name[i] = '\0';

                }
                break;

                case 'L':
                    // Go to space in : Content-Length: 22036
                    ptr = strchr( p_buffer, ' ' );

                    if( ptr == NULL )
                    {
                        return LINEPARSE_ERROR_NOT_VALID;
                    }

                    ptr++; // Extend past space

                    socket->content_length = atol( ptr );
                    //*file_size = atol( ptr );  // Convert to long
                    break;

                case 'o':
                    // Go to space in : Connection: close
                    ptr = strchr( p_buffer, ' ' );

                    if( ptr == NULL )
                    {
                        return LINEPARSE_ERROR_NOT_VALID;
                    }

                    ptr++; // Extend past space

                    if( strncmp( ptr, "keep-alive", 6 ) == 0 )
                    {
                        socket->connect_type = CONNECTION_KEEP_ALIVE;
                    }

                    else if( strncmp( ptr, "close", 5 ) == 0 )
                    {
                        socket->connect_type = CONNECTION_CLOSE;
                    }

                    break;
            }

            break;

        case 'T':
            if( p_buffer[9] == 'E' )
            {
                // Go to space in : Transfer-Encoding: chunked
                ptr = strchr( p_buffer, ' ' );

                if( ptr == NULL )
                {
                    return LINEPARSE_ERROR_NOT_VALID;
                }

                ptr++;  // Extend past space

                if( strncmp( ptr, "chunked", 3 ) == 0 )
                {
                    socket->encoding_type = ENCODING_CHUNKED;
                }
            }

            break;
    }

    return LINEPARSE_OK;
}

/*!
 *  \brief Gets size of transfer encoding chunk
 *
 *  \param address of int that will be populated with result
 *
 *  \return
 *     \retval int: number of bytes removed from rx buffer
 *
 *  \note
 *   <notes>
 */
uint8_t lineparse_get_chunked_size( socket_t* const socket, uint32_t* new_size )
{
    /* Chunked encoding keeps the length of the response in following hex chars followed by a CR and LF */
    char chunked_size[ 7 ] = {0};  // Only need a few bytes to hold upto 4 hex + CRLF + termination
    int counter = 0;
    char buffer;

    while( counter < 7 )
    {
        buffer = Net_Ethernet_28j60_getByte();

        switch( buffer )
        {
            case '\r':
                break;

            case '\n':
                *new_size = xtoi( chunked_size );
                return counter;
                break;
                
            default:
                chunked_size[ counter ] = buffer;
        }
        
        counter++;
    }
    
    return 0;
}
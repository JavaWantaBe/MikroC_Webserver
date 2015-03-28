#ifndef _LINE_PARSER_H
#define _LINE_PARSER_H

#include <stdint.h>
#include "../protocols/http_server/http_server.h"
#include "../socket_manager/socket_manager.h"

typedef enum
{
    LINEPARSE_OK = 0,
    LINEPARSE_OK_FORM,
    LINEPARSE_OK_FILE,
    LINEPARSE_ERROR,
    LINEPARSE_ERROR_NOT_VALID
};

/*!
 *  \brief Line parser for client requests received
 *
 */
int lineparse_client_request( socket_t* const socket );

/*!
 *  \brief Process get line from client request
 *
 */
int lineparse_process_client( socket_t* const socket );

/*!
 *  \brief Line parser for response from server
 *
 */
int lineparse_server_response( socket_t* const socket );

/*!
 *  \brief Get chunked data size for encoding type chunked
 *
 */
uint8_t lineparse_get_chunked_size( socket_t* const socket, uint32_t* new_size );


#endif
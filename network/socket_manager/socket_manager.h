/**
 * @file socket_manager.h
 *
 * @brief Manages current sockets being served
 *
 * @author Richard Lowe
 * @copyright AlphaLoewe
 *
 * @date 03/05/2015
 *
 * @version .01 - Initial
 *          .02 - Uses internal list manager for efficiency
 *
 * @details
 *
 * Status: <XX% completed.>
 *
 * @note
 * Test configuration:
 *   MCU:             %DEVICE%
 *   Dev.Board:       x
 *   Oscillator:      %DEVICE_CLOCK%
 *   Ext. Modules:    x
 *   SW:              %COMPILER%
 *
 * @par
 *   <all that matters>
 */

#ifndef _SOCKET_MANAGER_T
#define _SOCKET_MANAGER_T

#include <stdint.h>
#include "network_conf.h"
#include "page_manager/page_manager.h"
#include "__NetEthEnc28j60.h"

typedef enum {

    SOCKET_OK = 0,
    SOCKET_ERROR,
    SOCKET_ERROR_NONE_AVAIL,
    SOCKET_ERROR_NOT_FOUND
};

/**
 *  @struct type of request
 *
 */
typedef enum
{
    REQUEST_UNKNOWN = 0,
    REQUEST_GET,
    REQUEST_POST,
    REQUEST_PUT,

} SOCKET_REQUEST_TYPE_T;

/**
 *  @struct type of response to header
 *
 */
typedef enum
{
    RESPONSE_UNKNOWN = 0,
    RESPONSE_200_OK,
    RESPONSE_401_NOT_AUTHORIZED,
    RESPONSE_404_NOT_FOUND,
    RESPONSE_503_NOT_AVAILABLE
    
} SOCKET_RESPONSE_T;

/**
 *  @struct type of connection
 *
 */
typedef enum
{
    CONNECTION_UNKNOWN = 0,
    CONNECTION_KEEP_ALIVE,
    CONNECTION_CLOSE

} SOCKET_CONNECTION_T;

/**
 *  @struct Encoding of connection
 *
 */
typedef enum
{
    ENCODING_UNKNOWN = 0,
    ENCODING_TEXT,
    ENCODING_CHUNKED,
    ENCODING_ZIP

} SOCKET_TRANSFER_ENCODING_T;

/**
 *  @brief States for Sockets
 *  @enum State of Sockets
 *
 *  Socket states are represented in this enumerated type. The various stages of how a socket is
 *  parsed, validated, and served are representative in the members located in this structure.
 *
 *  @note
 *    As of current, not all modes are implemented.
 *
 */
typedef enum
{
    SM_HTTP_IDLE = 0,                /*!< Socket is idle*/
    #ifdef NETWORK_USE_SERVER
    SM_HTTP_GET_REQUEST,             /*!< Gets the full request from client */
    SM_HTTP_PROCESS_REQUEST,         /*!< Begins the process of returning data*/
    SM_HTTP_FIND_FILE,               /*!< Checks for file existance*/
    SM_HTTP_AUTHENTICATE,            /*!< Validates the current authorization state*/
    SM_HTTP_SERVE_HEADERS,           /*!< Sends any required headers for the response*/
    SM_HTTP_SERVE_BODY,              /*!< Serves the actual content*/
    SM_HTTP_WAIT_TILL_DONE,          /*!< Waits until all bytes are served*/
    SM_HTTP_DISCONNECT,              /*!< Disconnects the server and closes all files*/
    #endif
    #ifdef NETWORK_USE_CLIENT
    SM_HTTP_SVR_SEND_REQUEST,        /*!< */
    SM_HTTP_SVR_GET_RESPONSE,        /*!< */
    SM_HTTP_SVR_PARSE_RESPONSE,      /*!< */
    SM_HTTP_SVR_OPEN_FILE,           /*!< */
    SM_HTTP_SVR_GET_DATA,            /*!< */
    SM_HTTP_SVR_DOWNLOAD_COMPLETE,   /*!< */
    SM_HTTP_SVR_DISCONNECT,          /*!< */
    #endif
    #ifdef NETWORK_USE_TELNET
    SM_TELNET_SEND_HEADER,           /*!< Currently sending welcome header to the client */
    SM_TELNET_SEND_MENU,             /*!< Currently sending the command list menu to the client */
    SM_TELNET_GET_COMMAND,           /*!< Currently waiting for a command from the client */
    SM_TELNET_SEND_RESPONSE,         /*!< Processing the issued command and sending a response */
    SM_TELNET_DISCONNECT,
    #endif
    SM_STATE_UNKNOWN
    
} SOCKET_STATE_T;

/**
 * @struct socketNode_desc
 *
 * @brief Socket struct with file buffer information from SD card
 *
 * @note Multiple socket_desc can be used to speed up transfers, however
 * the memory requirement for this can grow quickly based on buffer sizes.
 */
typedef struct
{
    // Socket for TCP connection
    SOCKET_28j60_Dsc*     eth_socket;
    // Request type
    SOCKET_REQUEST_TYPE_T request_type;
    // Response type
    SOCKET_RESPONSE_T     response_type;
    // State of socket
    SOCKET_STATE_T        socket_state;
    // Connection Type -
    SOCKET_CONNECTION_T   connect_type;
    // Encoding Type
    SOCKET_TRANSFER_ENCODING_T encoding_type;
    // Socket buffers
    char buffer_large[LARGE_BUFFER_SIZE];
    // Buffer position
    uint16_t              buffer_position;
    // Get or Post buffer
    char get_post_buffer[GET_POST_SIZE];
    // Filename
    char file_name[FILE_NAME_SIZE];
    // Content_length
    uint32_t              content_length;
    // Alive time for persistent connections
    volatile uint8_t      alive_time;
    // Webpage
    page_t*               page;

} socket_t;

/**
 *  @brief Initializes sockets as well as list structure
 *
 *  @pre Memory manager needs to be initialized
 *
 *  @return status of initialization
 *    @retval SOCKET_OK
 *
 *  @note
 *   <notes>
 */
void socket_manager_init( void );

/**
 *  @brief Returns number of sockets currently being used
 *
 *  @return uint8_t
 *    @retval 4
 *
 *  @note
 *   <notes>
 */
uint8_t socket_manager_get_count( void );

/**
 *  @brief Finds socket based on ethernet socket
 *
 *  @param SOCKET_Dsc* eth_socket : socket structure defined in TCP layer
 *
 *  @return socket_t* :
 *    \retval NULL if not found
 *
 *  @note
 *   <notes>
 */
socket_t* socket_manager_find_socket( SOCKET_28j60_Dsc* eth_socket );

/**
 *  @brief Removes socket from list
 *
 *  @param socket_t* socket: socket to be removed
 *
 *  @return <Returns>
 *    @retval NULL if not found
 *
 *  @note
 *   <notes>
 */
int socket_manager_close( socket_t* socket );

/**
 *  @brief Cleanup function to be called to clean up disconnected sockets
 *
 *  @note
 *   <notes>
 */
void socket_manager_cleanup( void );

void socket_manager_alive_time_update( void );

char* socket_manager_print_status( socket_t* socket );

#endif
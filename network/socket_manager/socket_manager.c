#include "socket_manager.h"
#include <stddef.h>

#ifdef NETWORK_USE_SERVER
#include "protocols/http_server/http_server.h"
#endif

#ifdef NETWORK_USE_CLIENT
#include "protocols/http_client/http_client.h"
#endif

#ifdef NETWORK_USE_FTP
#include "protocols/ftp/ftp.h"
#endif

#ifdef NETWORK_USE_MAIL
#include "protocols/smtp/smtp.h"
#endif

#ifdef NETWORK_USE_TELNET
#include "protocols/telnet/telnet.h"
#endif

#define socket_list_size socket_list.size

typedef struct node
{
    socket_t* socket;
    struct node* next;
} node_t;

struct
{
    uint8_t size;
    node_t* head;
} socket_list;


static int initialize_new_socket( node_t** new_node, SOCKET_28j60_Dsc* eth_socket );
static int destroy_socket( node_t** node );


/***************************
 *  Globals
 **************************/


/**************************
 *  Private
 *************************/
static int destroy_socket( node_t** node )
{
    Free( ( char* )( *node )->socket, sizeof( socket_t ) );
    Free ( ( char* )*node, sizeof( node_t ) );
}


static int initialize_new_socket( node_t** new_node, SOCKET_28j60_Dsc* eth_socket )
{
    if( socket_list.size == MAX_SOCKET )
    {
        return SOCKET_ERROR;
    }
    else
    {
        socket_t* new_socket;
        
        if( !*new_node = ( node_t* )Malloc( sizeof( node_t ) )
            return SOCKET_ERROR;

        if( !new_socket = ( socket_t* )Malloc( sizeof( socket_t ) )
        {
            Free( ( char* )*new_node, sizeof( node_t ) );
            return SOCKET_ERROR;
        }

        memset( new_socket, 0, sizeof( socket_t ) );
        new_socket->eth_socket = eth_socket;
        
        ( *new_node )->next = socket_list.head;
        socket_list.size++;
    
    }
}

/**************************
 *  Public Functions
 *************************/
void socket_manager_init( )
{
    socket_list.size = 0;
    socket_list.head = NULL;
}


// Find
socket_t* socket_manager_find_socket( SOCKET_28j60_Dsc* eth_socket )
{
    node_t* curr_node = socket_list.head;
    char not_found = 1;

    while( not_found && curr_node != NULL )
    {
        if( curr_node->socket->eth_socket == eth_socket )
        {
            not_found = 0;
        }
        else
        {
            curr_node = curr_node->next;
        }
    }
    
    if( not_found )
    {
        if( initialize_new_socket( &curr_node, eth_socket ) != SOCKET_OK )
            return NULL;
    }
    else
    {
        return curr_node->socket;
    }
}


// Removes socket from list
int socket_manager_close( socket_t* socket )
{
    node_t* curr_node = socket_list.head;
    char not_found = 1;
    int status = SOCKET_ERROR_NOT_FOUND;
    
    while( not_found && curr_node != NULL )
    {
        if( curr_node->socket = socket )
        {
            node_t* tmp_node = curr_node->next;
            
            
            socket_list.size--;
            not_found = 0;
            status = SOCKET_OK;
        }
        else
        {
            curr_node = curr_node->next;
        }
    }

    return status;
}


// Updated every second
void socket_manager_alive_time_update()
{
    uint8_t i;
    node_t* curr_node = socket_list.head;
    
    for( i = 0; i <= socket_list_size; i++ )
    {
        curr_node->socket->alive_time++;
        curr_node = curr_node->next;
    }
}

// Cleans old sockets
void socket_manager_cleanup()
{
    if( dlist_size( &sockets ) != 0 )
    {
        dlist_element* curr_element = dlist_head( &sockets );
        socket_t* curr_soc = ( socket_t* )curr_element->payload;

        while( curr_element != NULL )
        {
            if( ( curr_soc->eth_socket->state == 0 ) || ( curr_soc->alive_time > MAX_ALIVE_TIME ) )
            {
                switch( curr_soc->eth_socket->destPort )
                {
                    case TCP_PORT_HTTP:
                        close_server_socket( curr_soc );
                        break;

                    case TCP_PORT_REQUEST:
                        close_client_socket( curr_soc );
                        break;

                    case TCP_PORT_SMTP:
                        // TODO:
                        break;

                    case TCP_PORT_TELNET:
                        close_telnet_socket( curr_soc );
                        break;
                }

            }

            curr_element = curr_element->next;
            curr_soc = ( socket_t* )curr_element->payload;
        }

    }
}

// Returns socket count
uint8_t socket_manager_get_count()
{
    //return dlist_size( &sockets );
    return socket_list_size;
}


// Print status
char* socket_manager_print_status( socket_t* socket )
{
    //static char socket_txt[256];

    //sprintl( socket_txt, "Socket: %p\r\nWebpage: %p - Name: %s\r\n", socket->eth_socket, socket->page, socket->page->page_name );

    return 0;
}
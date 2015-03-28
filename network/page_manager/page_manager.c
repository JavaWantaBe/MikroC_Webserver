#include "page_manager.h"
#include <stddef.h>
#include <stdint.h>
#include "linklist_d.h"


/****************************
 *  Prototypes
 ***************************/
static void destroy_page( void* payload );
static void page_set_details( page_t* const page );

/***************************
 *   Globals
 **************************/
static dlist pages;

/***************************
 *  Private Functions
 **************************/
static void destroy_page( void* payload )
{
    Free( ( char* ) payload, sizeof( page_t ) );
}

/*!
 *  \brief Webpage details
 *
 *  \param <Parameters Accepted>
 *
 *  \note
 *   <notes>
 */
static void page_set_details( page_t* const page )
{
    char txt_buffer[12];
    uint32_t page_size;

    switch( page->storage_type )
    {
        case PAGE_STATIC:
            page_size = page->static_page.end_ptr - page->static_page.start_ptr;
            break;

        case PAGE_DYNAMIC:
            page_size = page->dynamic_page.end_ptr - page->dynamic_page.start_ptr;
            break;
#ifdef NETWORK_USE_FAT32

        case PAGE_FLASH:
            page_size = page->flash_page.file_size;
            break;
#endif
    }

    // Get string of size
    LongToStr( page_size, txt_buffer );
    strcpy( page->page_size_str, Ltrim( txt_buffer ) );
}

/***************************************
 *   Public Functions
 **************************************/
/*!
 *  \brief Initializes webpage
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int page_manager_init()
{
    return ( dlist_init( &pages, destroy_page ) == DLIST_OK ) ? PAGE_OK : PAGE_ERROR;
}

/*!
 *  \brief Gets the webpage count
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
uint8_t page_manager_get_page_count()
{
    return dlist_size( &pages );
}


char* page_manager_print_page_list( void )
{
    static char page_list[256];
    dlist_element* curr_element = dlist_head( &pages );

    strcpy( page_list, "*************PAGE LIST**************\r\n" );

    while( curr_element != NULL )
    {
        strcat( page_list, ( ( page_t* )curr_element->payload )->page_name );
        strcat( page_list, "\r\n" );
        curr_element = curr_element->next;
    }

    strcat( page_list, "*************END LIST****************\r\n" );

    return page_list;

}

/*!
 *  \brief Finds page
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 */
page_t* page_manager_find_page( char* page_name )
{
    dlist_element* curr_element = dlist_head( &pages );

    while( curr_element != NULL )
    {
        if( strcmp( ( ( page_t* )curr_element->payload )->page_name, page_name ) == 0 )
        {
            return ( page_t* )curr_element->payload;
        }

        curr_element = curr_element->next;
    }

    return NULL;

}

// page removal
int page_manager_remove_page( char* page_name )
{
    dlist_element* curr_element = dlist_head( &pages );

    while( curr_element != NULL )
    {
        if( strcmp( ( ( page_t* )curr_element->payload )->page_name, page_name ) == 0 )
        {
            dlist_remove( &pages, curr_element );
            return PAGE_OK;
        }

        curr_element = curr_element->next;
    }

    return PAGE_ERROR_NOT_FOUND;
}

/*!
 *  \brief Addes a static webpage to the server catalog
 *
 *  \param const char* page: pointer to static page
 *
 *  \return int8_t successful add or not
 *    \retval 0 = success, -1 = unsuccessful
 *
 *  \note
 *   <notes>
 */
int page_manager_add_staticpage( const char* page, char* page_name, long page_size )
{

    page_t* tmp_page = ( page_t* )Malloc( sizeof( page_t ) );

    if( tmp_page == NULL )
    {
        return PAGE_ERROR_INSERT;
    }

    // String representation of page
    strcpy( tmp_page->page_name, page_name );
    // Set flag to represent a static page
    tmp_page->storage_type = PAGE_STATIC;
    // Set pointers
    tmp_page->static_page.start_ptr   = page;
    tmp_page->static_page.current_ptr = page;
    tmp_page->static_page.end_ptr     = ( page + page_size );

    page_set_details( tmp_page );

    if( dlist_insert_prev( &pages, tmp_page ) != DLIST_OK )
    {
        Free( ( char* ) tmp_page, sizeof( page_t ) );
        return PAGE_ERROR_INSERT;
    }

    return PAGE_OK;

}

/*!
 *  \brief Addes a dynamic webpage to the server catalog
 *
 *  \param const char* page: pointer to static page
 *
 *  \return int8_t successful add or not
 *    \retval 0 = success, -1 = unsuccessful
 *
 *  \note
 *   <notes>
 */
int page_manager_add_dynamicpage( char* const page, char* page_name, void ( *page_upd_callback )( void ) )
{

    page_t* tmp_page = ( page_t* )Malloc( sizeof( page_t ) );

    if( tmp_page == NULL )
    {
        return PAGE_ERROR_INSERT;
    }

    else
    {
        // String representation of page
        strcpy( tmp_page->page_name, page_name );
        // Set flag to represent a static page
        tmp_page->storage_type = PAGE_DYNAMIC;
        // Set pointers
        tmp_page->dynamic_page.start_ptr = page;
        tmp_page->dynamic_page.current_ptr = page;
        tmp_page->dynamic_page.end_ptr = page + strlen( page );
        tmp_page->dynamic_page.page_update = page_upd_callback;

        page_set_details( tmp_page );

        if( dlist_insert_prev( &pages, tmp_page ) != DLIST_OK )
        {
            Free( ( char* ) tmp_page, sizeof( page_t ) );
            return PAGE_ERROR_INSERT;
        }

        return PAGE_OK;
    }

}

#ifdef NETWORK_USE_FAT32
/*!
 *  \brief Adds a FLASH file to the webserver
 *
 *  \pre FAT32 filesystem needs to be initialized
 *
 *  \param char* : file_name
 *
 *  \return success or failure
 *    \retval -1 if failed positive if succeeded.
 *
 *  \note
 *   <notes>
 */
int page_manager_add_flashpage( char* file_name, bool changing_size )
{
    page_t* tmp_page = ( page_t* )Malloc( sizeof( page_t ) );

    if( tmp_page == NULL )
    {
        return PAGE_ERROR_INSERT;
    }

    // String representation of page
    strcpy( tmp_page->page_name, file_name );
    // Set flag to represent a static page
    tmp_page->storage_type = PAGE_FLASH;
    // Set pointers
    tmp_page->flash_page.file_position = 0;
    tmp_page->flash_page.file_handle = -1;
    tmp_page->flash_page.dynamic_filesize = changing_size;
    FAT32_Size( tmp_page->page_name, &tmp_page->flash_page.file_size );
    page_set_details( tmp_page );

    if( dlist_insert_prev( &pages, tmp_page ) != DLIST_OK )
    {
        Free( ( char* ) tmp_page, sizeof( page_t ) );
        return PAGE_ERROR_INSERT;
    }

    return PAGE_OK;

}

#endif
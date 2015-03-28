/**
 * @file page_manager.h
 *
 * @brief Webpage Manager
 *
 * @copyright AlphaLoewe
 *
 * @author Richard Lowe
 *
 * @date 3/6/2015
 *
 * @version <versionNo> - <change_description>
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
 
#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "network_conf.h"

#ifdef NETWORK_USE_FAT32
#include "__Lib_FAT32.h"
#endif


typedef enum
{
    PAGE_ERROR_NOT_FOUND = 0,
    PAGE_OK,
    PAGE_ERROR,
    PAGE_ERROR_INSERT,
    PAGE_ERROR_REMOVE
};

/*!
 *  \struct type of storage
 *
 */
typedef enum
{
    
    PAGE_UNKNOWN = 0,
    PAGE_DYNAMIC,
    PAGE_STATIC,
    #ifdef NETWORK_USE_FAT32
    PAGE_FLASH
    #endif

} PAGE_STORAGE_TYPE;

/*!
 *  \brief Structure of system webpage
 *
 *  \note
 *     as of current, each page takes 47 bytes
 */
typedef struct
{
    char page_name[13];                  // String representation of page
    char page_size_str[12];              // Size of webpage in string
    PAGE_STORAGE_TYPE storage_type;
    
    // If the page is a static page located in ROM pointer is here
    struct
    {
        
        const char* const start_ptr;     // Start pointer
        const char* current_ptr;
        const char* const end_ptr;       // End pointer

    } static_page;
    // If the page is dynamic and will be located in RAM pointer here
    struct
    {
        
        char* const start_ptr;           // Start pointer
        char* current_ptr;
        char* end_ptr;                   // End pointer
        void ( *page_update )( void );   // Function callback for dynamic updates

    } dynamic_page;
    
    #ifdef NETWORK_USE_FAT32
    // If file is on SD Card (FLASH)
    struct
    {
        
        __HANDLE file_handle;            // File handle for page
        uint32_t file_position;
        uint16_t buffer_position;        // Position in buffer
        uint32_t file_size;
        bool dynamic_filesize;
        
    } flash_page;
    #endif
    
} page_t;

/*******************************
 *   Functions
 ******************************/
/*!
 *  \brief Initializes webpage manager
 *
 *  \pre Memory Manager must be initialized
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int page_manager_init( void );

/*!
 *  \brief Returns page count
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
int page_manager_get_page_count( void );

/*!
 *  \brief Returns a string list of all pages
 *
 *  \pre <Preconditions that need to be met before called>
 *
 *  \param <Parameters Accepted>
 *
 *  \return <Returns>
 *    \retval <Values that might be returned>
 *
 *  \note
 *   <notes>
 */
char* page_manager_print_page_list( void );

/*!
 *  \brief Finds webpage in catalog
 *
 *  \param char* page_name: name of file searching for
 *
 *  \return webpage_t* - found webpage or NULL if not found
 *    \retval NULL == Not found
 *
 *  \note
 *   <notes>
 */
page_t* page_manager_find_page( char* page_name );
#ifdef NETWORK_USE_DYNAMIC_MEMORY
/*!
 *  \brief Removes page from catalog
 *
 *  \param char* page_name: name of file to be removed
 *
 *  \return
 *    \retval -1 failed to remove page
 *    \retval 0  successfully removed
 *
 *  \note
 *   <notes>
 */
int page_manager_remove_page( char* page_name );
#endif
/*!
 *  \brief Addes a static webpage to the server catalog
 *
 *  \param const char* page: pointer to static page
 *
 *  \return int8_t index of page added or failure
 *    \retval 0+ = success index, -1 = unsuccessful
 *
 *  \note
 *   <notes>
 */
int page_manager_add_staticpage( const char* page, char* page_name, long page_size );

/*!
 *  \brief Addes a dynamic webpage to the server catalog
 *
 *  \param const char* page: pointer to static page
 *  \param char* page_name: string name of page
 *  \param void( *page_upd_callback )( void ): callback that updates the dynamic data
 *
 *  \return int8_t index of page added or failure
 *    \retval 0+ = success index, -1 = unsuccessful
 *
 *  \note
 *   <notes>
 */
int page_manager_add_dynamicpage( char* const page, char* page_name, void ( *page_upd_callback )( void ) );

#ifdef NETWORK_USE_FAT32
/*!
 *  \brief Addes a dynamic webpage to the server catalog
 *
 *  \param const char* page: pointer to static page
 *  \param char* page_name: string name of page
 *  \param void( *page_upd_callback )( void ): callback that updates the dynamic data
 *
 *  \return int8_t index of page added or failure
 *    \retval 0+ = success index, -1 = unsuccessful
 *
 *  \note
 *   <notes>
 */
int page_manager_add_flashpage( char* file_name, bool changing_size );
#endif

#endif
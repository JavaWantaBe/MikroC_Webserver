/**
 * @file scheduler.h
 *
 * @brief Simple round robin task scheduler
 *
 * @author Richard Lowe
 * @copyright AlphaLoewe
 *
 * @date 08/11/2014
 *
 * @version .1 - Initial
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
 */


/**
 * @code
 *  void main()
 *  {
 *      initTimer();  // Initialize timer
 *
 *      task_scheduler_init( 1000 );
 *      task_add( task1, SCH_SECONDS_1 );
 *      task_add( task2, SCH_SECONDS_5 );
 *
 *      task_scheduler_start();
 *
 *      while(1)
 *      {
 *          task_dispatch();
 *      }
 *   }
 *   ... Call task_scheduler_clock() in ISR
 *
 *  #endcode
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

// task states
#define MAX_TASKS 7

#define SCH_SECONDS_1   1000
#define SCH_SECONDS_5   5000
#define SCH_SECONDS_10  10000
#define SCH_SECONDS_15  15000
#define SCH_SECONDS_30  30000
#define SCH_MINUTES_1   SCH_SECONDS_1 * 60
#define SCH_MINUTES_15  SCH_MINUTES_1 * 15
#define SCH_MINUTES_30  SCH_MINUTES_15 * 2
#define SCH_HOURS_1     SCH_MINUTES_30 * 2
#define SCH_HOURS_12    SCH_HOURS_1 * 12
#define SCH_DAY_1       SCH_HOURS_12 * 2

/* pointer to a void function with no arguments */
typedef void ( *task_t )( void );

/**
 * @enum Status of tasks in scheduler
 *
 */
typedef enum
{
    TASK_EMPTY = 0,
    TASK_STOPPED,       /**< Task is stopped           */
    TASK_RUNNABLE,      /**< Task is ready to be ran   */
    TASK_RUNNING,       /**< Task is currently running */
    TASK_ERROR = 99     /**< Error has occurred        */
} task_status_e;


/**
 *  @brief Initializes RR Scheduler
 *
 *  Initialization only requires a clock parameter.  Clock represents the
 *  time that expires between calles to task_scheduler_clock().
 *
 *  @param uint16_t clock - number of ms scheduler_timer_clock will be called
 *
 *  @code
 *    task_scheduler_init( 1000 ); // 1000 ms between calls
 *  @endcode
 *
 *  @note
 *   Sets all tasks to NULL
 */
void task_scheduler_init( uint16_t clock );


/**
 *  @brief Adds a task to the scheduler
 *
 *  @pre Scheduler must be initialized first
 *
 *  @param task_t task- Function that will be called when scheduler executes
 *  @param uint32_t period - how many nx100ms of delay the task requires
 *
 *  @returns uint8_t - id of created task
 */
uint8_t task_add( task_t task, uint32_t period );


/**
 *  @brief Deletes task from scheduler
 *
 *  @param uint8_t id - ID of task
 *
 */
void task_delete( uint8_t id );


/**
 *  @brief Get Task Status
 *
 *  @pre Scheduler must be initialized first
 *
 *  @param unsigned char id - ID of task
 *
 *  @return enum taskStatus_e
 *    @retval TASK_RUNNABLE
 *    @retval TASK_RUNNING
 *    @retval TASK_STOPPED
 *    @retval TASK_ERROR
 */
task_status_e task_get_status( uint8_t id );


/**
 *  @brief Stops the task from running
 *
 *  @param uint8_t id - id of task
 *
 */
void task_stop( uint8_t id );


/**
 *  @brief Resumes the task from a running task
 *
 *  @param uint8_t id - id of task
 *
 */
void task_resume( uint8_t id );


/**
 *  @brief Starts the task scheduler
 *
 *  @note
 *       At system initialization, tasks are undesirable to be running.
 *       Scheduler will not run until this function is called.
 */
void task_scheduler_start ( void );


/**
 *  @brief Stops the task scheduler
 *
 *  @note
 *
 */
void task_scheduler_stop ( void );


/**
 *  @brief Calls Ready Tasks
 *
 *  @pre Scheduler must be initialized first
 *
 *  @note
 *   Needs to be called in main while loop
 */
void task_dispatch( void );


/**
 *  @brief Returns the number of tasks in scheduler
 *
 *  @return - uint8_t
 */
uint8_t task_get_count( void );


/**
 *  @brief Function called by timer
 *
 *  @pre Clock needs to be initialized
 *
 */
void task_scheduler_clock( void );

#endif
#include "scheduler.h"


// basic task control block (TCB)
typedef struct
{
    uint8_t   id;                // task ID
    task_t    task;              // pointer to the task
    volatile uint32_t  delay;             // delay before execution
    uint32_t  period;
    task_status_e task_status;   // status of task
} task_control_t;

// Array of tasks
static task_control_t task_list[MAX_TASKS];
// used to calculate delay based on user input
static float count_per_ms;
// flag for enabling / disabling scheduler
static uint8_t task_scheduler_running;

/*******************
 *  Private
 ******************/
static task_control_t* find_task( uint8_t id );


static task_control_t* find_task( uint8_t id )
{
    int i;

    for( i = 0; i < MAX_TASKS; i++ )
    {
        if( task_list[i].id == id )
        {
            return &task_list[i];
        }
    }
    
    return 0;
}


// initialises the task list
void task_scheduler_init( uint16_t clock )
{
    task_scheduler_running = 0;
    // Should be set to zero since array is a global
    // memset( task_list, 0, sizeof( task_list ) );
    
    count_per_ms = 1.0f / ( float )clock;
}


/* adds a new task to the task list
   scans through the list and
   places the new task data where
   it finds free space */
uint8_t task_add( task_t task, uint32_t period )
{
    uint8_t task_id = 0;
    float time_calc = ( ( float )period ) * count_per_ms;
    
    if( time_calc < 1 ) time_calc = 1.0f;

    for( task_id = 0; task_id < MAX_TASKS; task_id++ )
    {
        if( task_list[task_id].task_status == TASK_EMPTY )
        {
            task_list[task_id].task_status = TASK_RUNNABLE;
            task_list[task_id].id          = task_id + 1;
            task_list[task_id].task        = task;
            task_list[task_id].delay       = ceil( time_calc );
            task_list[task_id].period      = task_list[task_id].delay;

            return task_list[task_id].id;
        }
    }
    
    return TASK_ERROR;
}

// remove task from task list
// note STOPPED is equivalent
// to removing a task
void task_delete( uint8_t id )
{
    task_control_t* task = find_task( id );
    
    if( task == 0 ) return;
    
    task->task = ( task_t ) 0x00;
    task->task_status = TASK_EMPTY;
}

// gets the task status
// returns ERROR if id is invalid
task_status_e task_get_status( uint8_t id )
{
    task_control_t* task = find_task( id );
    
    return ( task == 0 ) ? TASK_ERROR : task->task_status;
}

// Stops the task from running
void task_stop( uint8_t id )
{
    task_control_t* task = find_task( id );
    
    if( task == 0 ) return;
    
    task->task_status = TASK_STOPPED;
}

// Resumes the task
void task_resume( uint8_t id )
{
    task_control_t* task = find_task( id );
    
    if( task == 0 ) return;
    
    task->task_status = TASK_RUNNABLE;
}

// Starts the scheduler
void task_scheduler_start()
{
    task_scheduler_running = 1;
}

// Stops the scheduler
void task_scheduler_stop()
{
    task_scheduler_running = 0;
}


// dispatches tasks when they are ready to run
void task_dispatch()
{
    if( task_scheduler_running == 1 )
    {
        int i;

        for( i = 0; i < MAX_TASKS; i++ )
        {
            // check for a valid task ready to run
            if( ( task_list[i].delay == 0 ) && ( task_list[i].task_status == TASK_RUNNABLE ) )
            {
                task_list[i].task_status = TASK_RUNNING;  // task is now running
                ( *task_list[i].task )();                 // call the task
                task_list[i].delay = task_list[i].period; // reset the delay
                task_list[i].task_status = TASK_RUNNABLE; // task is runnable again
            }
        }
    }
}

// Get the number of tasks in scheduler
uint8_t task_get_count()
{
    uint8_t i, count = 0;
    
    for( i = 0; i < MAX_TASKS; i++ )
    {
        if( task_list[i].task_status == TASK_EMPTY )
            count++;
    }
    
    return count;
}

// clock
void task_scheduler_clock()
{
    
    if( task_scheduler_running == 1 )
    {
        int i;

        // cycle through available tasks
        for( i = 0; i < MAX_TASKS; i++ )
        {
            if( task_list[i].task_status == TASK_RUNNABLE )
            {
                if( task_list[i].delay > 0 )
                {
                    task_list[i].delay--;
                }
            }
        }

    }
}
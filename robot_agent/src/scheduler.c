/**
 * @file	scheduler.c
 * @author  Eriks Zaharans and Massimiiliano Raciti
 * @date    1 Jul 2013
 *
 * @section DESCRIPTION
 *
 * Cyclic executive scheduler library.
 */

/* -- Includes -- */
/* system libraries */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
/* project libraries */
#include "scheduler.h"
#include "task.h"
#include "timelib.h"


/* -- Defines -- */

#define MAJOR_CYCLE 1000

/* -- Global -- */

/* -- Functions -- */

/**
 * Initialize cyclic executive scheduler
 * @param minor Minor cycle in miliseconds (ms)
 * @return Pointer to scheduler structure
 */
scheduler_t *scheduler_init(void)
{
	// Allocate memory for Scheduler structure
	scheduler_t *ces = (scheduler_t *) malloc(sizeof(scheduler_t));
	
	/* --- Set minor cycle period --- */
	ces->minor = 125;
	
	init_stat(&g_stat);
	
	return ces;
}

/**
 * Deinitialize cyclic executive scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_destroy(scheduler_t *ces)
{
	// Free memory
	free(ces);
}

/**
 * Start scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_start(scheduler_t *ces)
{
	// Set timers
	timelib_timer_set(&ces->tv_started);
	timelib_timer_set(&ces->tv_cycle);
}

/**
 * Wait (sleep) till end of minor cycle
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_wait_for_timer(scheduler_t *ces)
{
	int sleep_time; // Sleep time in microseconds

	// Calculate time till end of the minor cycle
	sleep_time = (ces->minor * 1000) - (int)(timelib_timer_get(ces->tv_cycle) * 1000);

	// Add minor cycle period to timer
	timelib_timer_add_ms(&ces->tv_cycle, ces->minor);

	// Check for overrun and execute sleep only if there is no
	if(sleep_time > 0)
	{
		// Go to sleep (multipy with 1000 to get miliseconds)
		usleep(sleep_time);
	}
}

/**
 * Execute task
 * @param ces Pointer to scheduler structure
 * @param task_id Task ID
 * @return Void
 */
void scheduler_exec_task(scheduler_t *ces, int task_id)
{
	switch(task_id)
	{
		// Mission
		case s_TASK_MISSION_ID :
			task_mission();
			break;
			
		// Navigate
		case s_TASK_NAVIGATE_ID :
			task_navigate();
			break;
			
		// Control
		case s_TASK_CONTROL_ID :
			task_control();
			break;
			
		// Refine
		case s_TASK_REFINE_ID :
			task_refine();
			break;
			
		// Report
		case s_TASK_REPORT_ID :
			task_report();
			break;
			
		// Communicate
		case s_TASK_COMMUNICATE_ID :
			task_communicate();
			break;
			
		// Collision detection
		case s_TASK_AVOID_ID :
			task_avoid();
			break;
			
		// Other
		default :
			// Do nothing
			break;
		}
}

//#define LENGTH_MONITORING

/**
 * Execute task with monitoring
 * @param ces Pointer to scheduler structure
 * @param task_id Task ID
 * @return Void
 */
void scheduler_run_task(scheduler_t *ces, int task_id, struct timeval *timer)
{
	#ifdef LENGTH_MONITORING
		double before,after;
		
		before = timelib_timer_get(ces->tv_started);
		scheduler_exec_task(ces, task_id);
		after = timelib_timer_get(ces->tv_started);
		
		if(task_id==s_TASK_REFINE_ID)
			printf("REFINE %i %f\n",task_id, after-before);
	#else
		scheduler_exec_task(ces, task_id);
	#endif
	
	if(timelib_timer_get(*timer) > ces->minor )
	{
		stat_overruns_add(&g_stat,task_id);
	}
}	

/**
 * Run scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_run(scheduler_t *ces)
{
	struct timeval minor_cycle_start;
	
	int nb_minor_cycle = MAJOR_CYCLE/ces->minor;
	
	/* LAB 2: SERVER SYNCHRONISATION BEGIN */
	
	double unix_time = timelib_unix_timestamp()/1000;
    double sleep_time = (int)((ceil(unix_time)-unix_time)*1000000);
    usleep(sleep_time);
    
    /* LAB 2: SERVER SYNCHRONISATION END */
	
	scheduler_start(ces);

	while(1)
	{	
		for(int i=0; i<nb_minor_cycle; i++){
			timelib_timer_set(&minor_cycle_start);
			
			if(i == g_config.robot_id - 1)
			{
				scheduler_run_task(ces, s_TASK_COMMUNICATE_ID, &minor_cycle_start);
			}

			scheduler_run_task(ces, s_TASK_MISSION_ID, &minor_cycle_start);			
			
			if(i%4 == 0)
			{
				scheduler_run_task(ces, s_TASK_NAVIGATE_ID, &minor_cycle_start);
				scheduler_run_task(ces, s_TASK_CONTROL_ID, &minor_cycle_start);
			}

			scheduler_run_task(ces, s_TASK_REFINE_ID, &minor_cycle_start);
			scheduler_run_task(ces, s_TASK_REPORT_ID, &minor_cycle_start);
			scheduler_run_task(ces, s_TASK_AVOID_ID, &minor_cycle_start);
			
			scheduler_wait_for_timer(ces);
		}
		stat_executed_major_cycle_add(&g_stat);
		
		print_stat(&g_stat);
	}
}

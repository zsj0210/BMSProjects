#define BMS_DBG_TAG "Comm"

#include <stdio.h>
#include <rtthread.h>

#include "main.h"

#include "bms_comm.h"

#include "bms_hal_monitor.h"
#include "bms_hal_control.h"

#include "bms_monitor.h"
#include "bms_energy.h"
#include "bms_analysis.h"
#include "bms_global.h"
#include "bms_debug.h"




// thread config
#define COMM_TASK_STACK_SIZE	512	
#define COMM_TASK_PRIORITY		13
#define COMM_TASK_TIMESLICE		25

#define COMM_TASK_PERIOD		2000






static void BMS_CommTaskEntry(void *paramter);


void BMS_CommInit(void)
{
	rt_thread_t thread;


	thread = rt_thread_create("Comm", 
							   BMS_CommTaskEntry, 
							   NULL,
							   COMM_TASK_STACK_SIZE,
							   COMM_TASK_PRIORITY,
							   COMM_TASK_TIMESLICE);

	if (thread == NULL)
	{
		BMS_ERROR("Create Task Fail");
	}

	rt_thread_startup(thread);

}


/* 目前未支持，待后续支持 */
static void BMS_CommTaskEntry(void *paramter)
{
	while(1)
	{	
		osDelay(COMM_TASK_PERIOD);
	}
}




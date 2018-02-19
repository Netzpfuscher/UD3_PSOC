#if !defined(tsk_midi_TASK_H)
	#define tsk_midi_TASK_H

/*
 * Add user task definitions, types, includes and other things in the below
 * merge region to customize the task.
 */
/* `#START USER_TYPES_AND_DEFINES` */
#include <device.h>
    
xQueueHandle qMIDI_rx;
xSemaphoreHandle new_midi_data;
    
    
/* `#END` */
	
void tsk_midi_Start( void );

/*
 * Add user function prototypes in the below merge region to add user
 * functionality to the task definition.
 */
/* `#START USER_TASK_PROTOS` */

/* `#END` */

/* ------------------------------------------------------------------------ */
#endif
/* [] END OF FILE */
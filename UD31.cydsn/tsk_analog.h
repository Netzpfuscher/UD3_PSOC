#if !defined(tsk_analog_TASK_H)
	#define tsk_analog_TASK_H

/*
 * Add user task definitions, types, includes and other things in the below
 * merge region to customize the task.
 */
/* `#START USER_TYPES_AND_DEFINES` */
#include <device.h>
    
    
#define CT_PRIMARY 0
#define CT_SECONDARY 1
    
/* `#END` */
	
void tsk_analog_Start( void );
uint16_t CT1_Get_Current(uint8_t channel);
float CT1_Get_Current_f(uint8_t channel);

/*
 * Add user function prototypes in the below merge region to add user
 * functionality to the task definition.
 */
/* `#START USER_TASK_PROTOS` */

/* `#END` */

/* ------------------------------------------------------------------------ */
#endif
/* [] END OF FILE */
#if !defined(tsk_uart_TASK_H)
	#define tsk_uart_TASK_H

/*
 * Add user task definitions, types, includes and other things in the below
 * merge region to customize the task.
 */
/* `#START USER_TYPES_AND_DEFINES` */
#include <device.h>
    

    
/* `#END` */
	
void tsk_uart_Start( void );

xQueueHandle qUart_tx;
xQueueHandle qUart_rx;





/*
 * Add user function prototypes in the below merge region to add user
 * functionality to the task definition.
 */
/* `#START USER_TASK_PROTOS` */

/* `#END` */

/* ------------------------------------------------------------------------ */
#endif
/* [] END OF FILE */
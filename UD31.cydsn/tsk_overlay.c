#include <cytypes.h>
#include "cyapicallbacks.h"   

#include "tsk_overlay.h"

#include "FreeRTOS.h"
#include "FreeRTOS_task.h"
#include "FreeRTOS_semphr.h"


/* ------------------------------------------------------------------------ */
/*
 * Place user included headers, defines and task global data in the
 * below merge region section.
 */
/* `#START USER_INCLUDE SECTION` */
    #include <project.h>
    #include <stdio.h>
    #include "telemetry.h"
    #include "cli_common.h"
    #include "tsk_analog.h"
    #include "tsk_uart.h"
    

/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * User defined task-local code that is used to process commands, control
 * operations, and/or generrally do stuff to make the taks do something
 * meaningful.
 */
/* `#START USER_TASK_LOCAL_CODE` */

void show_overlay(uint8_t port){
    xSemaphoreTake(block_term[port],portMAX_DELAY);

 
    char buffer[50];
    Term_Save_Cursor(port);
    send_string("\033[?25l",port);
    
    uint8_t row_pos = 1;
    uint8_t col_pos = 90;
    //Term_Erase_Screen(port);
    Term_Box(row_pos,col_pos,row_pos+9,col_pos+25,port);
    Term_Move_Cursor(row_pos+1,col_pos+1,port);
    sprintf(buffer,"Bus Voltage:       %4iV",telemetry.bus_v);
    send_string(buffer,port);
        
    Term_Move_Cursor(row_pos+2,col_pos+1,port);
    sprintf(buffer,"Battery Voltage:   %4iV",telemetry.batt_v);
    send_string(buffer,port);
    
    Term_Move_Cursor(row_pos+3,col_pos+1,port);
    sprintf(buffer,"Temp 1:          %4i *C",telemetry.temp1);
    send_string(buffer,port);
    
    Term_Move_Cursor(row_pos+4,col_pos+1,port);
    sprintf(buffer,"Temp 2:          %4i *C",telemetry.temp2);
    send_string(buffer,port);
    
    Term_Move_Cursor(row_pos+5,col_pos+1,port);
    send_string("Bus status: ",port);
    
    switch (telemetry.bus_status){
        case BUS_READY:
            send_string("       Ready",port);
        break;
        case BUS_CHARGING:
            send_string("    Charging",port);
        break;
        case BUS_OFF:
            send_string("         OFF",port);
        break;
    }
    
    Term_Move_Cursor(row_pos+6,col_pos+1,port);
    sprintf(buffer,"Average power:     %4iW",telemetry.avg_power);
    send_string(buffer,port);
    
    Term_Move_Cursor(row_pos+7,col_pos+1,port);
    sprintf(buffer,"Average Current: %4i.%iA",telemetry.batt_i/10,telemetry.batt_i%10);
    send_string(buffer,port);
    
   
    Term_Move_Cursor(row_pos+8,col_pos+1,port);
    sprintf(buffer,"Primary Current:   %4iA",CT1_Get_Current(CT_PRIMARY));
    send_string(buffer,port);
    
    Term_Restore_Cursor(port);
    send_string("\033[?25h",port);
    xSemaphoreGive(block_term[port]);
     
    
    
}


/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * This is the main procedure that comprises the task.  Place the code required
 * to preform the desired function within the merge regions of the task procedure
 * to add functionality to the task.
 */
void tsk_overlay_TaskProc( void *pvParameters )
{
	/*
	 * Add and initialize local variables that are allocated on the Task stack
	 * the the section below.
	 */
	/* `#START TASK_VARIABLES` */
    


	/* `#END` */
	
	/*
	 * Add the task initialzation code in the below merge region to be included
	 * in the task.
	 */
	/* `#START TASK_INIT_CODE` */
    
    
    

	/* `#END` */

	
	for(;;) {
		/* `#START TASK_LOOP_CODE` */
        show_overlay((uint32_t)pvParameters);
        
		/* `#END` */
        
        vTaskDelay( 1000/ portTICK_PERIOD_MS);
		
	}
}

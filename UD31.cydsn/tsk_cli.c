/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#include <cytypes.h>
#include "cyapicallbacks.h"   

#include "tsk_cli.h"

#include "FreeRTOS.h"
#include "FreeRTOS_task.h"
#include "FreeRTOS_semphr.h"

xTaskHandle UART_Terminal_TaskHandle;
xTaskHandle USB_Terminal_TaskHandle;
uint8 tsk_cli_initVar = 0u;

#if (1 == 1)
	xSemaphoreHandle UART_Terminal_Mutex;
    xSemaphoreHandle USB_Terminal_Mutex;
#endif

/* ------------------------------------------------------------------------ */
/*
 * Place user included headers, defines and task global data in the
 * below merge region section.
 */
/* `#START USER_INCLUDE SECTION` */

#include <project.h>
#include "ntshell.h"
#include "cli_common.h"
#include "tsk_priority.h"
#include "tsk_uart.h"
#include "tsk_usb.h"

#define UNUSED_VARIABLE(N)  do { (void)(N); } while (0)
void *extobjt = 0;

static int serial_read(char *buf, int cnt, void *extobj);
static int usb_read(char *buf, int cnt, void *extobj);
static int serial_write(const char *buf, int cnt, void *extobj);
static int usb_write(const char *buf, int cnt, void *extobj);
void initialize_cli(ntshell_t* ptr, uint8_t port);
static int serial_callback(const char *text, void *extobj);
static int usb_callback(const char *text, void *extobj);

/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * User defined task-local code that is used to process commands, control
 * operations, and/or generrally do stuff to make the taks do something
 * meaningful.
 */
/* `#START USER_TASK_LOCAL_CODE` */

void initialize_cli(ntshell_t* ptr, uint8_t port){
    
    switch (port){
    case SERIAL:
        UART_2_Start();
        ntshell_init(ptr, serial_read, serial_write, serial_callback, extobjt);
    break;
    case USB:
        USBMIDI_1_Start(0, USBMIDI_1_5V_OPERATION);
        ntshell_init(ptr, usb_read, usb_write, usb_callback, extobjt);
    break;
    default:
    break;    
    }
    
    ntshell_set_prompt(ptr, ":>");
    ntshell_show_promt(ptr);
  
}

static int serial_read(char *buf, int cnt, void *extobj){
    UNUSED_VARIABLE(extobj);

    return cnt;
}

static int usb_read(char *buf, int cnt, void *extobj){
    UNUSED_VARIABLE(extobj);

    return cnt;
}

static int serial_write(const char *buf, int cnt, void *extobj){
    UNUSED_VARIABLE(extobj);
    //UART_2_PutArray((uint8*)buf,cnt);
    
    for(int i = 0;i<cnt;i++){
        xQueueSend(qUart_tx, &buf[i],portMAX_DELAY);
    }
    
    return cnt;
}

static int usb_write(const char *buf, int cnt, void *extobj){
    UNUSED_VARIABLE(extobj);
    if (0u != USBMIDI_1_GetConfiguration()){
    
//        while (0u == USBMIDI_1_CDCIsReady()){}
//        USBMIDI_1_PutData((uint8*)buf,cnt);
        
        for(int i = 0;i<cnt;i++){
        xQueueSend(qUSB_tx, &buf[i],portMAX_DELAY);
        }
        
    };
   
    return cnt;
}

static int serial_callback(const char *text, void *extobj){
    UNUSED_VARIABLE(extobj);
    /*
     * This is a really simple example codes for the callback function.
     */
    nt_interpret(text,SERIAL);
    return 0;
}

static int usb_callback(const char *text, void *extobj){
    UNUSED_VARIABLE(extobj);
    /*
     * This is a really simple example codes for the callback function.
     */
    nt_interpret(text,USB);
    return 0;
}

uint8_t handle_uart_terminal(ntshell_t* ptr){
    static uint8_t blink = 0;
    char c;
    if(blink) blink--;
    if(blink==1) rx_blink_Control = 1; 
    //if(UART_2_GetRxBufferSize()==0) return 0;

  
        if(xQueueReceive(qUart_rx,&c,portMAX_DELAY)){
            if(xSemaphoreTake(block_term[SERIAL],portMAX_DELAY)){;
        //c = UART_2_GetChar();
            rx_blink_Control = 0;
            blink = 240;
            ntshell_execute_nb(ptr,c); 
            xSemaphoreGive(block_term[SERIAL]);
            }else{
                send_string("cannot\r\n",SERIAL);
            }
        }
        
   
	return 0;
}

uint8_t handle_USB_terminal(ntshell_t* ptr){
    static uint8_t blink = 0;
    char c;
    if(blink) blink--;
    if(blink==1) rx_blink_Control = 1; 
    //if(UART_2_GetRxBufferSize()==0) return 0;
    if(xQueueReceive(qUSB_rx,&c,portMAX_DELAY)){
        xSemaphoreTake(block_term[USB],portMAX_DELAY);
    //c = UART_2_GetChar();
        rx_blink_Control = 0;
        blink = 240;
        ntshell_execute_nb(ptr,c); 
        xSemaphoreGive(block_term[USB]);
    }
   
	return 0;
}

/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * This is the main procedure that comprises the task.  Place the code required
 * to preform the desired function within the merge regions of the task procedure
 * to add functionality to the task.
 */
void tsk_cli_TaskProc( void *pvParameters )
{
	/*
	 * Add and initialize local variables that are allocated on the Task stack
	 * the the section below.
	 */
	/* `#START TASK_VARIABLES` */
    
    ntshell_t ntserial;

	/* `#END` */
	
	/*
	 * Add the task initialzation code in the below merge region to be included
	 * in the task.
	 */
	/* `#START TASK_INIT_CODE` */
    
    initialize_cli(&ntserial, (uint32_t)pvParameters);
 

	/* `#END` */
	
	
	for(;;) {
		/* `#START TASK_LOOP_CODE` */
        
        switch((uint32_t)pvParameters){
            case SERIAL:
                handle_uart_terminal(&ntserial);
            break;
            case USB:
                handle_USB_terminal(&ntserial);
            break; 
            default:
            break;
        }

		/* `#END` */

		//vTaskDelay( 20/ portTICK_PERIOD_MS);
	}
    send_string("CLI-Task died",(uint32_t)pvParameters);
}
/* ------------------------------------------------------------------------ */
void cli_Start( void )
{
	/*
	 * Insert task global memeory initialization here. Since the OS does not
	 * initialize ANY global memory, execute the initialization here to make
	 * sure that your task data is properly 
	 */
	/* `#START TASK_GLOBAL_INIT` */

	/* `#END` */
	#if defined(cli_START_CALLBACK)
		cli_Start_Callback();
	#endif
	
	if (tsk_cli_initVar != 1) {
		#if (1 == 1)
			UART_Terminal_Mutex = xSemaphoreCreateMutex();
            USB_Terminal_Mutex = xSemaphoreCreateMutex();
		#endif
	
		/*
	 	* Create the task and then leave. When FreeRTOS starts up the scheduler
	 	* will call the task procedure and start execution of the task.
	 	*/
		xTaskCreate(tsk_cli_TaskProc,"UART-CLI",1024,(void*)SERIAL,PRIO_TERMINAL,&UART_Terminal_TaskHandle);
        xTaskCreate(tsk_cli_TaskProc,"USB-CLI",1024,(void*)USB,PRIO_TERMINAL,&USB_Terminal_TaskHandle);
		tsk_cli_initVar = 1;
	}
}
/* ------------------------------------------------------------------------ */
/* ======================================================================== */
/* [] END OF FILE */

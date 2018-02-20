/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#include <device.h>
#include "DMA.h"
#include "interrupter.h"
#include "ZCDtoPWM.h"
#include "charging.h"
#include "telemetry.h"

#include "cli_common.h"
#include "tsk_cli.h"
#include "tsk_analog.h"
#include "tsk_thermistor.h"
#include "tsk_uart.h"
#include "tsk_usb.h"
#include "tsk_fault.h"
#include "tsk_midi.h"


void vMainTask( void *pvParameters);

int main()
{
    system_fault_Control = 0;   //this should suppress any start-up sparking until the system is ready
    EEPROM_1_Start();
    eprom_load();
    initialize_DMA();           //sets up all DMA channels
    initialize_interrupter();   //initializes hardware related to the interrupter
    initialize_ZCD_to_PWM();    //initializes hardware related to ZCD to PWM
    initialize_charging();		


    CyGlobalIntEnable;          //enables interrupts

    //calls that must always happen after updating the configuration/settings
    configure_ZCD_to_PWM(); 
	

    rx_blink_Control = 1;    
    block_term[0] = xSemaphoreCreateBinary();  //Blocking semaphore for overlay Display
    block_term[1] = xSemaphoreCreateBinary();  //Blocking semaphore for overlay Display
    xSemaphoreGive(block_term[0]);
    xSemaphoreGive(block_term[1]);
    

    //Starting Tasks
    tsk_uart_Start();       //Handles UART-Hardware and queues
    tsk_usb_Start();        //Handles USB-Hardware and queues
    tsk_midi_Start();
    cli_Start();            //Commandline interface
    tsk_analog_Start();     //Reads bus voltage and currents
    tsk_thermistor_Start(); //Reads thermistors
    tsk_fault_Start();      //Handles fault conditions
    
    FreeRTOS_Start();
    
    for(;;)
    {
        //should never reach this point

    }
}




/* [] END OF FILE */

/*
 * UD3
 *
 * Copyright (c) 2018 Jens Kerrinnes
 * Copyright (c) 2015 Steve Ward
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

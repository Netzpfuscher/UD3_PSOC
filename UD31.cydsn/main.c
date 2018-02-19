/*UD3 USB Code!!!

Code designed on/for PSoC Creator V4.0

Tips for playing with the UD3 without an external controller:

Running test QCW pulses:
1) in "UD3config.h" near the top of the file, set "ext_trig_runs_CW        1" (make sure to undo this later!)
2) Set the "config.max_qcw_pw" to desired value (units are uS*10).
3) engaging the trigger/enable should fire a 10mS or so ramp that holds maximum voltage 
    until either the QCW pulse limiter stops it, or you let go of the switch. 


 * ========================================
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

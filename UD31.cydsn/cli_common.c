/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

/* [] END OF FILE */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <project.h>
#include <math.h>
#include "cli_common.h"
#include "interrupter.h"
#include "charging.h"
#include "ZCDtoPWM.h"
#include "telemetry.h"
#include "ntshell.h"
#include "autotune.h"


#include <cytypes.h>
#include "cyapicallbacks.h"   

#include "FreeRTOS.h"
#include "FreeRTOS_task.h"
#include "FreeRTOS_semphr.h"

#include "tsk_priority.h"
#include "tsk_uart.h"
#include "tsk_usb.h"
#include "tsk_overlay.h"
#include "tsk_analog.h"


#define UNUSED_VARIABLE(N)  do { (void)(N); } while (0)

typedef struct  {
	const char* text;
	uint8_t (*commandFunction)(char *commandline, uint8_t port);
    const char* help;
} command_entry;

uint8_t updateConfDefFunction(int newValue, uint8_t index);
uint8_t updateDefaultFunction(int newValue, uint8_t index);
uint8_t updateConfigFunction(int newValue, uint8_t index);
uint8_t updateTRFunction(int newValue, uint8_t index);
uint8_t updateTuneFunction(int newValue, uint8_t index);
uint8_t updateOfftimeFunction(int newValue, uint8_t index);

uint8_t commandHelp(char *commandline, uint8_t port);
uint8_t commandGet(char *commandline, uint8_t port);
uint8_t commandGetConf(char *commandline, uint8_t port);
uint8_t commandSet(char *commandline, uint8_t port);
uint8_t commandSetConf(char *commandline, uint8_t port);
uint8_t command_tr(char *commandline, uint8_t port);
uint8_t command_udkill(char *commandline, uint8_t port);
uint8_t command_eprom(char *commandline, uint8_t port);
uint8_t command_status(char *commandline, uint8_t port);
uint8_t command_window(char *commandline, uint8_t port);
uint8_t command_cls(char *commandline, uint8_t port);
uint8_t command_tune_p(char *commandline, uint8_t port);
uint8_t command_tune_s(char *commandline, uint8_t port);
uint8_t command_tasks(char *commandline, uint8_t port);
uint8_t command_bootloader(char *commandline, uint8_t port);
uint8_t command_qcw(char *commandline, uint8_t port);
uint8_t command_bus(char *commandline, uint8_t port);

void nt_interpret(const char *text, uint8_t port);

uint8_t kill_msg[3] = {0xb0, 0x77, 0x00};


uint8_t tr_running=0;

parameter_entry volatile tparameters[] = {
    // "Text   "	    ,Value	,Min	,Max	, UpdateFunction        ,Help text
	{ "pw"              , 50	, 0		,800	, updateTRFunction      ,"Pulsewidth"},
	{ "pwd"             , 50000	, 0		,60000	, updateTRFunction      ,"Pulsewidthdelay"},
    { "tune_start"      , 400	, 5		,400	, updateTuneFunction    ,"Start frequency"},
    { "tune_end"        , 1000	, 5		,400	, updateTuneFunction    ,"End frequency"},
    { "tune_pw"         , 50	, 0		,800	, updateTuneFunction    ,"Tune pulsewidth"},
    { "tune_delay"      , 50	, 1		,200	, updateTuneFunction    ,"Tune delay"},
    { "offtime"         , 2  	, 2		,250	, updateOfftimeFunction ,"Offtime for MIDI"},
    { "qcw_ramp"        , 2  	, 1		,10   	, updateDefaultFunction ,"QCW Ramp Inc/93us"},
 	//
};

parameter_entry confparam[] = {
    // "Text   "	        ,Value	,Min	,Max	, UpdateFunction        ,Help text
    { "watchdog"            , 1	    , 0		,1	    , updateConfDefFunction ,"Watchdog Enable"},    
    { "max_tr_pw"           , 1000  , 0		,3000   , updateConfigFunction  ,"Maximum TR PW [uSec]"},
    { "max_tr_prf"          , 800   , 0		,3000   , updateConfigFunction  ,"Maximum TR frequency [Hz]"},
    { "max_qcw_pw"          , 10000 , 0		,30000  , updateConfigFunction  ,"Maximum QCW PW [uSec*10]"},
    { "max_tr_current"      , 400   , 0		,1000	, updateConfigFunction  ,"Maximum TR current [A]"},
    { "min_tr_current"      , 100   , 0		,1000   , updateConfigFunction  ,"Minimum TR current [A]"},
    { "max_qcw_current"     , 300   , 0		,1000	, updateConfigFunction  ,"Maximum QCW current [A]"},
    { "temp1_max"           , 40    , 0		,100    , updateConfigFunction  ,"Max temperature 1 [*C]"},
    { "temp2_max"           , 40    , 0		,100    , updateConfigFunction  ,"Max temperature 2 [*C]"},
    { "ct1_ratio"           , 600   , 1		,2000   , updateConfigFunction  ,"CT1 [N Turns]"},
    { "ct2_ratio"           , 1000  , 1	    ,2000   , updateConfigFunction  ,"CT2 [N Turns]"},
    { "ct3_ratio"           , 30    , 1		,2000   , updateConfigFunction  ,"CT3 [N Turns]"},
    { "ct1_burden"          , 33    , 1		,1000   , updateConfigFunction  ,"CT1 burden [deciOhm]"},   
    { "ct2_burden"          , 500   , 1		,1000   , updateConfigFunction  ,"CT2 burden [deciOhm]"},
    { "ct3_burden"          , 33    , 1		,1000   , updateConfigFunction  ,"CT3 burden [deciOhm]"},
    { "lead_time"           , 200   , 0		,2000   , updateConfigFunction  ,"Lead time [nSec]"},
    { "start_freq"          , 630   , 0		,5000   , updateConfigFunction  ,"Resonant freq [Hz*100]"},
    { "start_cycles"        , 3     , 0		,20     , updateConfigFunction  ,"Start Cyles [N]"},
    { "max_tr_duty"         , 100   , 1		,500    , updateConfigFunction  ,"Max TR duty cycle [0.1% incr]"},
    { "max_qcw_duty"        , 350   , 1		,500    , updateConfigFunction  ,"Max QCW duty cycle [0.1% incr]"},
    { "temp1_setpoint"      , 30    , 0		,100    , updateConfigFunction  ,"Setpoint for fan [*C]"},
    { "ext_trig_enable"     , 0     , 0		,1      , updateConfigFunction  ,"External trigger enable [0-1]"},
    { "batt_lockout_v"      , 360   , 0		,500    , updateConfigFunction  ,"Battery lockout voltage [V]"},
    { "slr_fswitch"         , 500   , 0		,1000   , updateConfigFunction  ,"SLR switch frequency [Hz*100]"},
    { "slr_vbus"            , 200   , 0		,1000   , updateConfigFunction  ,"SLR Vbus [V]"},
    { "ps_scheme"           , 2     , 0		,4      , updateConfigFunction  ,"Power supply sheme"},
    { "autotune_s"          , 1     , 1 	,32     , updateConfDefFunction,"Number of samples for Autotune"},
 	//
};

command_entry commands[] =
	{
        {"help"		        ,commandHelp        ,"this text" },
 		{"get"		        ,commandGet         ,"usage get [param]" },
        {"confget"          ,commandGetConf     ,"usage confget [param]" },
 		{"set"		        ,commandSet         ,"usage set [param] [value]" },
        {"confset"	        ,commandSetConf     ,"usage confset [param] [value]" },
        {"tr"		        ,command_tr         ,"Transient [start/stop]" },
        {"kill"		        ,command_udkill     ,"Kills all UD Coils" },        
        {"eeprom"		    ,command_eprom      ,"Save/Load config [load/save]" },
        {"status"		    ,command_status     ,"Displays coil status" },
        {"window"		    ,command_window     ,"Displays message window" },
        {"cls"		        ,command_cls        ,"Clear screen" },
        {"tune_p"	        ,command_tune_p     ,"Autotune Primary" },
        {"tune_s"	        ,command_tune_s     ,"Autotune Secondary" },
        {"tasks"	        ,command_tasks      ,"Show running Tasks" },
        {"bootloader"       ,command_bootloader ,"Start bootloader" },
        {"qcw"              ,command_qcw        ,"qcw [start/stop]" },
        {"bus"              ,command_bus        ,"bus [on/off]" },
};

uint8_t updateConfDefFunction(int newValue, uint8_t index) {
	if (newValue >= confparam[index].min && newValue <= confparam[index].max)
	{
		confparam[index].value = newValue;
		return 1;
	} else
		return 0;	
}

uint8_t updateDefaultFunction(int newValue, uint8_t index) {
	if (newValue >= tparameters[index].min && newValue <= tparameters[index].max)
	{
		tparameters[index].value = newValue;
		return 1;
	} else
		return 0;	
}

uint8_t updateOfftimeFunction(int newValue, uint8_t index) {
	if (newValue >= tparameters[index].min && newValue <= tparameters[index].max)
	{
		tparameters[index].value = newValue;
        Offtime_WritePeriod(newValue);
		return 1;
	} else
		return 0;	
}


uint8_t updateTuneFunction(int newValue, uint8_t index) {
	if (newValue >= tparameters[index].min && newValue <= tparameters[index].max)
	{
        switch(index){
            case PARAM_TUNE_SFREQ:
                if(tparameters[PARAM_TUNE_EFREQ].value <= newValue) return 0;
            break;
            case PARAM_TUNE_EFREQ:
                if(tparameters[PARAM_TUNE_SFREQ].value >= newValue) return 0;
            break;
            default:
            break;
        }
        
		tparameters[index].value = newValue;
		return 1;
	} else
		return 0;	
}

uint8_t updateTRFunction(int newValue, uint8_t index) {
	if (newValue >= tparameters[index].min && newValue <= tparameters[index].max)
	{
		tparameters[index].value = newValue;
        interrupter.pw = tparameters[PARAM_PW].value;
        interrupter.prd = tparameters[PARAM_PWD].value;
        if(tr_running){
            update_interrupter();
        }
		return 1;
	} else
		return 0;	
}

uint8_t updateConfigFunction(int newValue, uint8_t index) {
	if (newValue >= confparam[index].min && newValue <= confparam[index].max)
	{
        uint8 sfflag = system_fault_Read();
        system_fault_Control = 0;   //halt tesla coil operation during updates!
		confparam[index].value = newValue;
        initialize_interrupter();
		initialize_charging();
        configure_ZCD_to_PWM();
        system_fault_Control = sfflag;
		return 1;
	} else
		return 0;	
}

uint8_t command_bus(char *commandline, uint8_t port){
    if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
     if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        Term_Color_Red(port);
        send_string("Usage: bus [on|off]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
   
    if (strcmp(commandline,"on") == 0)
    {
        bus_command=BUS_COMMAND_ON;
        send_string("BUS ON\r\n",port);
    }
    if (strcmp(commandline,"off") == 0)
    {
        bus_command=BUS_COMMAND_OFF;
        send_string("BUS OFF\r\n",port);
    }
    
    return 1;
}

uint8_t command_status(char *commandline, uint8_t port){
    static xTaskHandle overlay_Serial_TaskHandle;
    static xTaskHandle overlay_USB_TaskHandle;
    if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
    
    if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        Term_Color_Red(port);
        send_string("Usage: status [start|stop]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
   
    if (strcmp(commandline,"start") == 0)
    {
        switch(port){
            case SERIAL:
                if(overlay_Serial_TaskHandle == NULL){
                    xTaskCreate(tsk_overlay_TaskProc,"Overl_S",256,(void*)SERIAL,PRIO_OVERLAY,&overlay_Serial_TaskHandle);
                }
            break;
            case USB:
                if(overlay_USB_TaskHandle == NULL){
                    xTaskCreate(tsk_overlay_TaskProc,"Overl_U",256,(void*)USB,PRIO_OVERLAY,&overlay_USB_TaskHandle);
                }
            break;
        }
        //ntshell_set_scroll_region(&ntserial,20,40);
    }
    if (strcmp(commandline,"stop") == 0)
    {
        switch(port){
            case SERIAL:
                if(overlay_Serial_TaskHandle != NULL){
                    vTaskDelete(overlay_Serial_TaskHandle);
                    overlay_Serial_TaskHandle=NULL;
                }
            break;
            case USB:
                if(overlay_USB_TaskHandle != NULL){
                    vTaskDelete(overlay_USB_TaskHandle);
                    overlay_Serial_TaskHandle=NULL;
                }
            break;
        }
       
    }
    
    return 1;
}

uint8_t command_cls(char *commandline, uint8_t port){
    Term_Erase_Screen(port);
    return 1;
}

uint8_t command_bootloader(char *commandline, uint8_t port){
    Bootloadable_Load();
    return 1;
}


void Term_Save_Cursor(uint8_t port){
    send_string("\033[s",port);
}
void Term_Restore_Cursor(uint8_t port){
    send_string("\033[u",port);
}


void Term_Box(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2,uint8_t port){
    Term_Move_Cursor(row1,col1,port);
    Term_BGColor_Blue(port);
    send_string("\xE2\x95\x94", port); //edge upper left
    int i=0;
    for(i=1;i<(col2-col1);i++){
        send_string("\xE2\x95\x90", port); //=
    }
    send_string("\xE2\x95\x97", port); //edge upper right
    for(i=1;i<(row2-row1);i++){
        Term_Move_Cursor(row1+i,col1,port);
        send_string("\xE2\x95\x91", port);   //left ||
        Term_Move_Cursor(row1+i,col2,port);
        send_string("\xE2\x95\x91", port);  //right ||
    }
    Term_Move_Cursor(row2,col1,port);
    send_string("\xE2\x95\x9A", port);  //edge lower left
    for(i=1;i<(col2-col1);i++){
        send_string("\xE2\x95\x90", port); //=
    }
    send_string("\xE2\x95\x9D", port);  //edge lower right
    Term_Color_White(port);
}

uint8_t command_window(char *commandline, uint8_t port){
    if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        Term_Color_Red(port);
        send_string("Usage: window [string]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
    send_string("\r\n",port);
    Term_Save_Cursor(port);
   
    uint8_t nchars = strlen(commandline);
    Term_Box(5,30,7,30+nchars,port);
    Term_Move_Cursor(6,31,port);
    send_string(commandline+1,port);
    Term_Restore_Cursor(port);
    return 1;
}


uint8_t command_tr(char *commandline, uint8_t port){
    
    if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
    
    if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        Term_Color_Red(port);
        send_string("Usage: tr [start|stop]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
    
    if (strcmp(commandline,"start") == 0)
    {
        interrupter.pw = tparameters[PARAM_PW].value;
        interrupter.prd = tparameters[PARAM_PWD].value;
        update_interrupter();
        tr_running = 1;
        send_string("Transient Enabled\r\n",port);
	    return 0;
    }
    if (strcmp(commandline,"stop") == 0)
    {
        interrupter.pw = 0;
        update_interrupter();
        tr_running = 0;
        send_string("\r\nTransient Disabled\r\n",port);
	    return 0;
    }
    return 1;
    
}

uint8_t command_qcw(char *commandline, uint8_t port){
    
    if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
    
    if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        Term_Color_Red(port);
        send_string("Usage: qcw [start|stop]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
    
    if (strcmp(commandline,"start") == 0)
    {
        qcw_reg=1;
        ramp.modulation_value = 20;
	    ramp_control();
        send_string("QCW Enabled\r\n",port);
	    return 0;
    }
    if (strcmp(commandline,"stop") == 0)
    {
        QCW_enable_Control = 0;
        send_string("\r\nQCW Disabled\r\n",port);
	    return 0;
    }
    return 1;
    
}

uint8_t command_udkill(char *commandline, uint8_t port){
    USBMIDI_1_callbackLocalMidiEvent(0, kill_msg);
    bus_command= BUS_COMMAND_OFF;
    interrupter1_control_Control = 0;
    QCW_enable_Control = 0;
    Term_Color_Green(port);
    send_string("Killed\r\n",port);
    Term_Color_White(port);
	return 0;
}

uint8_t command_tune_p(char *commandline, uint8_t port){
    run_adc_sweep(tparameters[PARAM_TUNE_SFREQ].value,tparameters[PARAM_TUNE_EFREQ].value,tparameters[PARAM_TUNE_PW].value, CT_PRIMARY,tparameters[PARAM_TUNE_DELAY].value, port);
    return 0;
}

uint8_t command_tune_s(char *commandline, uint8_t port){
    run_adc_sweep(tparameters[PARAM_TUNE_SFREQ].value,tparameters[PARAM_TUNE_EFREQ].value,tparameters[PARAM_TUNE_PW].value, CT_SECONDARY,tparameters[PARAM_TUNE_DELAY].value, port);
    return 0;
}

uint8_t command_tasks(char *commandline, uint8_t port){
    static char buff[500];
    send_string("********************************************\n\r", port);
    send_string("Task          State   Prio    Stack    Num\n\r", port); 
    send_string("********************************************\n\r", port);
    vTaskList(buff);
    send_string(buff, port);
    send_string("*********************************************\n\r", port);
    return 0;
}

uint8_t commandGet(char *commandline, uint8_t port) {
    char buffer[100];
	uint8_t current_parameter;
	
	if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
	
	if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        send_string("\tParameter\t\t| Value\t\t| Text\r\n",port);
		for (current_parameter = 0 ; current_parameter < sizeof(tparameters)/sizeof(parameter_entry); current_parameter++)
		{
            if(strlen(tparameters[current_parameter].name) > 7){
                sprintf(buffer,"\t\033[36m%s\033[37m\t\t| \033[32m%i\033[37m\t\t| %s\r\n",tparameters[current_parameter].name,tparameters[current_parameter].value,tparameters[current_parameter].help);
            }else{
                sprintf(buffer,"\t\033[36m%s\033[37m\t\t\t| \033[32m%i\033[37m\t\t| %s\r\n",tparameters[current_parameter].name,tparameters[current_parameter].value,tparameters[current_parameter].help);
            }
            send_string(buffer,port);
		}
		return 1;
	}
	
	for (current_parameter = 0 ; current_parameter < sizeof(tparameters)/sizeof(parameter_entry); current_parameter++)
	{
		if (strcmp(commandline,tparameters[current_parameter].name) == 0)
		{
			//Parameter not found:
			sprintf(buffer,"\t%s=%i\r\n",tparameters[current_parameter].name,tparameters[current_parameter].value);
            send_string(buffer,port);
			return 1;			
		}
	}
    Term_Color_Red(port);
	send_string("E: unknown param\r\n",port);
    Term_Color_White(port);
	return 0;
}

uint8_t commandGetConf(char *commandline, uint8_t port) {
    char buffer[100];
	uint8_t current_parameter;
	
	if (*commandline == 0x20 && commandline!=0) commandline++;	//Leerzeichen überspringen
	
	if(*commandline == 0 || commandline==0)	//Kein Parametername --> Liste anzeigen
	{
        send_string("\tParameter\t\t| Value\t\t| Text\r\n",port);
		for (current_parameter = 0 ; current_parameter < sizeof(confparam)/sizeof(parameter_entry); current_parameter++)
		{
            if(strlen(confparam[current_parameter].name) > 7){
                sprintf(buffer,"\t\t\033[36m%s\033[37m\t\t| \033[32m%i\033[37m\t\t| %s\r\n",confparam[current_parameter].name,confparam[current_parameter].value,confparam[current_parameter].help);
            }else{
                sprintf(buffer,"\t\t\033[36m%s\033[37m\t\t\t| \033[32m%i\033[37m\t\t| %s\r\n",confparam[current_parameter].name,confparam[current_parameter].value,confparam[current_parameter].help);
            }
            send_string(buffer,port);
		}
		return 1;
	}
	
	for (current_parameter = 0 ; current_parameter < sizeof(confparam)/sizeof(parameter_entry); current_parameter++)
	{
		if (strcmp(commandline,confparam[current_parameter].name) == 0)
		{
			//Parameter not found:
			sprintf(buffer,"\t%s=%i\r\n",confparam[current_parameter].name,confparam[current_parameter].value);
            send_string(buffer,port);
			return 1;			
		}
	}
    Term_Color_Red(port);
	send_string("E: unknown param\r\n",port);
    Term_Color_White(port);
	return 0;
}

uint8_t commandSet(char *commandline, uint8_t port) {
    char buffer[44];
	char *param_value;
	
	if (*commandline == ' ') commandline++;


	if (commandline == NULL) {
        if(!port)
        Term_Color_Red(port);
		send_string("E: no name\r\n",port);
        Term_Color_White(port);
		return 0;
	}
	
	param_value = strchr(commandline,' ');
	if ( param_value == NULL) {
        Term_Color_Red(port);
		send_string("E: no value\r\n",port);
        Term_Color_White(port);
		return 0;	
	}
	
	//Trennzeichen gefunden und durch \0 ersetzen, um Strings zu trennen
	*param_value= 0;
	param_value++;

	if (*param_value == '\0') {
        Term_Color_Red(port);
		send_string("E: no val\r\n",port);
        Term_Color_White(port);
		return 0;
	}
	
		
	uint8_t current_parameter;
	for (current_parameter = 0 ; current_parameter < sizeof(tparameters)/sizeof(parameter_entry); current_parameter++)
	{
		if (strcmp(commandline,tparameters[current_parameter].name) == 0)
		{
			//Parameter-Name gefunden:
			int new_value;
			new_value = atoi(param_value);
			
			if (new_value < tparameters[current_parameter].min || new_value > tparameters[current_parameter].max)
			{
                sprintf(buffer,"E:Range %i-%i\r\n",tparameters[current_parameter].min,tparameters[current_parameter].max);
                Term_Color_Red(port);
				send_string(buffer,port);
                Term_Color_White(port);
                return 0;
			}
			
			if (tparameters[current_parameter].updateFunction(new_value,current_parameter)) {
				tparameters[current_parameter].value=new_value; //Wert übernehmen
                Term_Color_Green(port);
				send_string("OK\r\n",port);
                Term_Color_White(port);
				return 1;
			} else {
                Term_Color_Red(port);
				send_string("NOK\r\n",port);
                Term_Color_White(port);
				return 1;
			}
		}
	}
    Term_Color_Red(port);
	send_string("E: unknown param\r\n",port);
    Term_Color_White(port);
	return 0;
}

uint8_t commandSetConf(char *commandline, uint8_t port) {
    char buffer[44];
	char *param_value;
	
	if (*commandline == ' ') commandline++;


	if (commandline == NULL) {
        if(!port)
        Term_Color_Red(port);
		send_string("E: no name\r\n",port);
        Term_Color_White(port);
		return 0;
	}
	
	param_value = strchr(commandline,' ');
	if ( param_value == NULL) {
        Term_Color_Red(port);
		send_string("E: no value\r\n",port);
        Term_Color_White(port);
		return 0;	
	}
	
	//Trennzeichen gefunden und durch \0 ersetzen, um Strings zu trennen
	*param_value= 0;
	param_value++;

	if (*param_value == '\0') {
        Term_Color_Red(port);
		send_string("E: no val\r\n",port);
        Term_Color_White(port);
		return 0;
	}
	
		
	uint8_t current_parameter;
	for (current_parameter = 0 ; current_parameter < sizeof(confparam)/sizeof(parameter_entry); current_parameter++)
	{
		if (strcmp(commandline,confparam[current_parameter].name) == 0)
		{
			//Parameter-Name gefunden:
			int new_value;
			new_value = atoi(param_value);
			
			if (new_value < confparam[current_parameter].min || new_value > confparam[current_parameter].max)
			{
                Term_Color_Red(port);
                sprintf(buffer,"E:Range %i-%i\r\n",confparam[current_parameter].min,confparam[current_parameter].max);
				send_string(buffer,port);
                Term_Color_White(port);
                return 0;
			}
			
			if (confparam[current_parameter].updateFunction(new_value,current_parameter)) {
				confparam[current_parameter].value=new_value; //Wert übernehmen
                Term_Color_Green(port);
				send_string("OK\r\n",port);
                Term_Color_White(port);
				return 1;
			} else {
                Term_Color_Red(port);
				send_string("NOK\r\n",port);
                Term_Color_White(port);
				return 1;
			}
		}
	}
    Term_Color_Red(port);
	send_string("E: unknown param\r\n",port);
    Term_Color_White(port);
	return 0;
}
uint8_t eprom_load(void){
        uint8_t current_parameter;
        uint8 sfflag = system_fault_Read();
        system_fault_Control = 0;   //halt tesla coil operation during updates!
        uint16_t count=0;
        count = (sizeof(confparam)/sizeof(parameter_entry))*2;
        if(EEPROM_1_ReadByte(count)!=0xDE) return 0;
        count++;
        if(EEPROM_1_ReadByte(count)!=0xAD) return 0;
        count++;
        if(EEPROM_1_ReadByte(count)!=0xBE) return 0;
        count++;
        if(EEPROM_1_ReadByte(count)!=0xEF) return 0;
        count = 0;

        for (current_parameter = 0 ; current_parameter < sizeof(confparam)/sizeof(parameter_entry); current_parameter++)
    	{
    		    confparam[current_parameter].value = EEPROM_1_ReadByte(count);
                count++;
                confparam[current_parameter].value |= EEPROM_1_ReadByte(count)<<8;
                count++;
    	}
        initialize_interrupter();
		initialize_charging();
        configure_ZCD_to_PWM();
        system_fault_Control = sfflag;
        return 1;
}

/********************************************
* Saves confparams to eeprom
*********************************************/
uint8_t command_eprom(char *commandline, uint8_t port){
    char buffer[44];
    if (*commandline == 0x20 && commandline!=0) commandline++;	//skip space
    
    if(*commandline == 0 || commandline==0)	//no param show list
	{
        Term_Color_Red(port);
        send_string("Usage: eprom [load|save]\r\n",port);
        Term_Color_White(port);
		return 1;
	}
    uint8_t current_parameter;
    uint16_t count=0;
    uint8_t change_flag=0;
    uint16_t change_count=0;
    uint8 sfflag = system_fault_Read();
    system_fault_Control = 0;   //halt tesla coil operation during updates!
    if (strcmp(commandline,"save") == 0)
    {
        
        EEPROM_1_UpdateTemperature();
        for (current_parameter = 0 ; current_parameter < sizeof(confparam)/sizeof(parameter_entry); current_parameter++)
    	{
                change_flag=0;
                if(EEPROM_1_ReadByte(count) != LO8(confparam[current_parameter].value)){
    		        EEPROM_1_WriteByte(LO8(confparam[current_parameter].value),count);
                    change_flag=1;
                }
                count++;
                if(EEPROM_1_ReadByte(count) != HI8(confparam[current_parameter].value)){
                    EEPROM_1_WriteByte(HI8(confparam[current_parameter].value),count);
                    change_flag=1;
                }
    		    count++;
                if(change_flag){
                    change_count++;
                }
    	}
        EEPROM_1_WriteByte(0xDE,count);
        count++;
        EEPROM_1_WriteByte(0xAD,count);
        count++;
        EEPROM_1_WriteByte(0xBE,count);
        count++;
        EEPROM_1_WriteByte(0xEF,count);
        sprintf(buffer,"%i / %i new config params written\r\n",change_count,sizeof(confparam)/sizeof(parameter_entry));
        Term_Color_Green(port);
		send_string(buffer,port);
        Term_Color_White(port);
        //send_string("\r\nWrite EEPROM OK\r\n", port);
        system_fault_Control = sfflag;
    	return 0;
    }
    if (strcmp(commandline,"load") == 0)
    {
        Term_Color_Green(port);
        if(eprom_load()){
            send_string("Read EEPROM OK\r\n", port);
        }else{
            send_string("Read EEPROM FAILED\r\n", port);
        }
        Term_Color_White(port);
    	return 0;
    }
    return 1;
}
	

/********************************************
* Prints the help text
*********************************************/
uint8_t commandHelp(char *commandline, uint8_t port) {
    UNUSED_VARIABLE(commandline);
	uint8_t current_command;
	send_string("\r\nCommands:\r\n",port);
	for (current_command=0; current_command < (sizeof(commands) / sizeof(command_entry)); current_command++) {
        send_string("\t",port);
        Term_Color_Cyan(port);
        send_string((char*)commands[current_command].text,port);
        Term_Color_White(port);
        if(strlen(commands[current_command].text) > 7){
            send_string("\t-> ",port);
        }else{
            send_string("\t\t-> ",port);
        }
        send_string((char*)commands[current_command].help,port);
		send_string("\r\n",port);
	}
    
	send_string("\r\nParameters:\r\n",port);
	for (current_command = 0 ; current_command < sizeof(tparameters)/sizeof(parameter_entry); current_command++)
	{
        send_string("\t",port);
        Term_Color_Cyan(port);
        send_string((char*)tparameters[current_command].name,port);
        Term_Color_White(port);
        if(strlen(tparameters[current_command].name) > 7){
            send_string("\t-> ",port);
        }else{
            send_string("\t\t-> ",port);
        }
        
        send_string((char*)tparameters[current_command].help,port);
		send_string("\r\n",port);
	}
    
    send_string("\r\nConfiguration:\r\n",port);
	for (current_command = 0 ; current_command < sizeof(confparam)/sizeof(parameter_entry); current_command++)
	{
        send_string("\t",port);
        Term_Color_Cyan(port);
        send_string((char*)confparam[current_command].name,port);
        Term_Color_White(port);
        if(strlen(confparam[current_command].name) > 7){
            send_string("\t-> ",port);
        }else{
            send_string("\t\t-> ",port);
        }
        
        send_string((char*)confparam[current_command].help,port);
		send_string("\r\n",port);
	}
    
    
	return 0;
}

/********************************************
* Sends char to transmit queue
*********************************************/
void send_char(uint8 c, uint8_t port){
    char buf[4];
    switch(port){
    case USB:
        if(qUSB_tx!=NULL) xQueueSend(qUSB_tx,&c,portMAX_DELAY);
    break;
    case SERIAL:
        //UART_2_PutChar(c);
        buf[0]=c;
        buf[1]='\0';
        if(qUart_tx!=NULL) xQueueSend(qUart_tx,buf,portMAX_DELAY);
        
    break;
    }
}

/********************************************
* Sends string to transmit queue
*********************************************/
void send_string(char * data, uint8_t port){

    switch(port){
        case USB:
            if(qUSB_tx!=NULL){

                while((*data)!= '\0'){
                    //if(xQueueSendFromISR(qUSB_tx,data,NULL)) data++;
                    if(xQueueSend(qUSB_tx,data,portMAX_DELAY)) data++;
                }
                
            }
        
        break;
        case SERIAL:
            if(qUart_tx!=NULL){
                while((*data)!= '\0'){
                    //if(xQueueSendFromISR(qUart_tx,data,NULL)) data++;
                    if(xQueueSend(qUart_tx,data,portMAX_DELAY)) data++;
                }
            }
        break;
    }
}
/********************************************
* Sends buffer to transmit queue
*********************************************/
void send_buffer(uint8_t * data,uint16_t len, uint8_t port){

    switch(port){
        case USB:
            if(qUSB_tx!=NULL){
                while(len){
                    if(xQueueSend(qUSB_tx,data,portMAX_DELAY))
                    data++;
                    len--;
                }
            }
           
        break;
        case SERIAL:
            if(qUart_tx!=NULL){
                while(len){
                    if(xQueueSend(qUart_tx,data,portMAX_DELAY))
                    data++;
                    len--;
                }
            }
        break;
    }
}


/********************************************
* Interprets the Input String
*********************************************/
void nt_interpret(const char *text, uint8_t port) {
	uint8_t current_command;
	for (current_command=0; current_command < (sizeof(commands) / sizeof(command_entry)); current_command++) {
		if (memcmp(text, commands[current_command].text, strlen(commands[current_command].text)) == 0) {
			commands[current_command].commandFunction( (char*) strchr(text, ' '),port);

			return;
		}
		
	}
    if(text[0] != 0x00){
        Term_Color_Red(port);
        send_string("Unknown Command: ",port);
        send_string((char*)text,port);
        send_string("\r\n",port);
        Term_Color_White(port);
    }

 }
	
void Term_Erase_to_End_of_Line(void) {
	    UART_2_PutString("\0x1B[K");
}


void Term_Erase_Screen(uint8_t port)
{
    send_string("\033[2J\033[1;1H",port);
}

void Term_Color_Green(uint8_t port){
    send_string("\033[32m",port);
}
void Term_Color_Red(uint8_t port){
    send_string("\033[31m",port);
}
void Term_Color_White(uint8_t port){
    send_string("\033[37m",port);
}
void Term_Color_Cyan(uint8_t port){
    send_string("\033[36m",port);
}
void Term_BGColor_Blue(uint8_t port){
    send_string("\033[44m",port);
}

void Term_Move_Cursor(uint8_t row, uint8_t column,uint8_t port)
{
    char buffer[44];
    sprintf(buffer,"\033[%i;%iH",row,column);
    send_string(buffer,port);
}

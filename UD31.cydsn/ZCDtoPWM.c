/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#include <device.h>
#include <math.h>
#include "interrupter.h"
#include "ZCDtoPWM.h"
#include "cli_common.h"

void initialize_ZCD_to_PWM(void)
{
    //initialize all the timers/counters/PWMs
    PWMA_Start();
    PWMB_Start();
    FB_capture_Start();
    ZCD_counter_Start();
    FB_glitch_detect_Start();
    
    //initialize all comparators
    ZCD_compA_Start();
    ZCD_compB_Start();
    CT1_comp_Start();
    
    //initialize the DACs
    CT1_dac_Start();   //hier
    ZCDref_Start();     //hier

    //start the feedback filter block
    FB_Filter_Start();
    FB_Filter_SetCoherency(FB_Filter_CHANNEL_A, FB_Filter_KEY_MID);
//	FB_Filter_SetDalign(FB_Filter_STAGEA_DALIGN, FB_Filter_ENABLED);
//	FB_Filter_SetDalign(FB_Filter_HOLDA_DALIGN, FB_Filter_ENABLED);

    configure_ZCD_to_PWM();
}

void configure_CT1(void){
    
    float max_tr_cl_dac_val_temp;
    float max_qcw_cl_dac_val_temp;
    float min_tr_cl_dac_val_temp;
    
        //figure out the CT setups
    max_tr_cl_dac_val_temp = (((float)confparam[CONF_MAX_TR_CURRENT].value/(float)confparam[CONF_CT1_RATIO].value)*confparam[CONF_CT1_BURDEN].value)/(DAC_VOLTS_PER_STEP*10);
    if(max_tr_cl_dac_val_temp > 255)
    {
        max_tr_cl_dac_val_temp = 255;
    }
    params.max_tr_cl_dac_val = round(max_tr_cl_dac_val_temp);
    max_qcw_cl_dac_val_temp = (((float)confparam[CONF_MAX_QCW_CURRENT].value/(float)confparam[CONF_CT1_RATIO].value)*confparam[CONF_CT1_BURDEN].value)/(DAC_VOLTS_PER_STEP*10);
    if(max_qcw_cl_dac_val_temp > 255)
    {
        max_qcw_cl_dac_val_temp = 255;
    }
    
    params.max_qcw_cl_dac_val = round(max_qcw_cl_dac_val_temp);
    min_tr_cl_dac_val_temp = (((float)confparam[CONF_MIN_TR_CURRENT].value/(float)confparam[CONF_CT1_RATIO].value)*confparam[CONF_CT1_BURDEN].value)/(DAC_VOLTS_PER_STEP*10);
    if(min_tr_cl_dac_val_temp > 255)
    {
        min_tr_cl_dac_val_temp = 255;
    }
    params.min_tr_cl_dac_val =  round(min_tr_cl_dac_val_temp);
    
    ct1_dac_val[0] = params.max_tr_cl_dac_val;
    ct1_dac_val[1] = params.max_tr_cl_dac_val;
    ct1_dac_val[2] = params.max_qcw_cl_dac_val;
    
    params.diff_tr_cl_dac_val = params.max_tr_cl_dac_val - params.min_tr_cl_dac_val;
}

void configure_CT2(void){
    //DC CT full scale
    params.Idc_fs = (50*(float)confparam[CONF_CT2_RATIO].value)/(confparam[CONF_CT2_BURDEN].value*4096);
    
    //DC CT mA_Count
    params.idc_ma_count = (uint32_t)((confparam[CONF_CT2_RATIO].value*50*1000)/confparam[CONF_CT2_BURDEN].value)/4096;
}


void configure_ZCD_to_PWM(void)
{
    //this function calculates the variables used at run-time for the ZCD to PWM hardware
    //it also initializes that hardware so its ready for interrupter control
    
    float lead_time_temp;
    float pwm_start_prd_temp;
    
   
    configure_CT1();
    configure_CT2();
    
    
    //initialize the ZCD counter
	if(confparam[CONF_START_CYCLES].value == 0)
    {
        ZCD_counter_WritePeriod(1);
        ZCD_counter_WriteCompare(1);
    }
	else
	{
    	ZCD_counter_WritePeriod(confparam[CONF_START_CYCLES].value*2);
    	ZCD_counter_WriteCompare(4);
	}
    
    //calculate lead time
    lead_time_temp = (float)confparam[CONF_LEAD_TIME].value/CPU_CLK_PERIOD;
    params.lead_time = round(lead_time_temp);
    
    //calculate starting period
    pwm_start_prd_temp = CPU_CLK_FREQ/(confparam[CONF_START_FREQ].value*200); //why 200? well 2 because its half-periods, and 100 because frequency is in hz*100
	params.pwm_top = round(pwm_start_prd_temp*2);  //top value for FB capture and pwm generators to avoid ULF crap
    params.pwma_start_prd = params.pwm_top - params.lead_time;
	params.pwma_start_cmp = params.pwm_top - pwm_start_prd_temp + 4;	//untested, was just 4;
    params.pwma_run_prd = params.pwm_top - params.lead_time;
	params.pwmb_start_prd = round(pwm_start_prd_temp);  //experimental start up mode, 2 cycles of high frequency to start things up but stay ahead of the game.
    params.pwmb_start_cmp = 4;
	params.pwmb_start_psb_val = 15;//config.psb_start_val;// params.pwmb_start_prd>>3;
	//Set up FB_GLITCH PWM
    params.fb_glitch_cmp = pwm_start_prd_temp/4;  //set the lock out period to be 1/4 of a cycle long
    if(params.fb_glitch_cmp > 255)
    {
        params.fb_glitch_cmp = 255;
    }
    FB_glitch_detect_WritePeriod(255);
    FB_glitch_detect_WriteCompare1(255-params.fb_glitch_cmp);
    FB_glitch_detect_WriteCompare2(255);   

    //initialize PWM generator
    PWMA_WritePeriod(params.pwma_start_prd);
    PWMA_WriteCompare(params.pwma_start_cmp);
    
    //start up oscillator
    PWMB_WritePeriod(params.pwmb_start_prd);
    PWMB_WriteCompare(params.pwmb_start_cmp);
    //initialize FB Capture period to account for 10 count offset in timing due to reset delay.
    FB_capture_WritePeriod(params.pwm_top - 10);
    //i think that initializing these might fix a bug that QCW mode always seems to miss the first pulse when the UD3
    //is first started.
    ramp.modulation_value = 0;
    ramp.modulation_value_previous = 0;
    
	//prime the feedback filter with fake values so that the first time we use it, its not absolutely garbage
	uint8 z;
	for(z = 0; z < 5; z++)
	{
		FB_Filter_Write24(FB_Filter_CHANNEL_A,params.pwm_top - round(pwm_start_prd_temp));
		uint16 wait = 0;
		while(wait < 255)	wait++;
	}
	
	//set reference voltage for zero current detector comparators
    ZCDref_Data = 25;
	ZCDref_Data = 25;
}


/* [] END OF FILE */

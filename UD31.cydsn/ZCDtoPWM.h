/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef ZCDTOPWM_H
#define ZCDTOPWM_H

#define CPU_CLK_PERIOD 15.6325
#define CPU_CLK_FREQ 64000000
#define DAC_VOLTS_PER_STEP 0.016
#define R_BURDEN 3.3

typedef struct
{
    uint8 max_tr_cl_dac_val;
    uint16 lead_time;
    uint16 pwma_start_prd;
	uint16 pwma_start_cmp;
	uint16 pwma_run_prd;
    uint16 pwmb_start_prd;
    uint16 pwmb_start_cmp;
    uint16 pwm_top;
    uint16 fb_glitch_cmp;
    uint16 pwmb_psb_prd;
    uint16 pwmb_psb_val;
    uint16 pwmb_start_psb_val;
    uint8 max_qcw_cl_dac_val;
    uint16 min_tr_prd;
    uint8 min_tr_cl_dac_val;
    uint8 diff_tr_cl_dac_val;
    uint16_t idc_ma_count;
    float Idc_fs;
} parameters;
volatile parameters params;

//variables read by DMA for PWM stuffs
uint16 pwma_init_val; //holds the initial PWMA compare2 value, DMA transfers it to the PWMA module
uint16 fb_filter_in;
uint16 fb_filter_out;
uint16 fb_fake;
uint8 ct1_dac_val[3];


void initialize_ZCD_to_PWM(void);
void configure_ZCD_to_PWM(void);
void configure_CT1(void);
void configure_CT2(void);

#endif

//[] END OF FILE

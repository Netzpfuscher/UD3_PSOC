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

#ifndef AUTOTUNE_H
#define AUTOTUNE_H

#define AT_WD_PRD 50    //mS between test bursts in auto-tune sweep mode

#include <device.h>

uint16 freq_response[128][2];  //[frequency value, amplitude]

void run_adc_sweep(uint16_t F_min, uint16_t F_max, uint16_t pulsewidth, uint8_t channel,uint8_t delay, uint8_t port);


#endif

//[] END OF FILE
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

#ifndef TELEMETRY_H
#define TELEMETRY_H

#define BUS_OFF         0
#define BUS_CHARGING    1
#define BUS_READY       2
#define BUS_TEMP1_FAULT 3
#define BUS_TEMP2_FAULT 4
#define BUS_TEMP3_FAULT 5
#define BUS_BATT_OV_FLT 6
#define BUS_BATT_UV_FLT 7

typedef struct
{
    uint16 bus_v;           //in volts
    uint16 batt_v;          //in volts
    uint16 aux_batt_v;                  //not used
    uint8 temp1;            //in *C
    uint8 temp2;            //in *C
    uint8 temp3;                        //not used
    uint8 bus_status;       //0 = charging, 1 = ready
	uint16 avg_power;       //Average power in watts
	uint16 bang_energy;     //bang energy in joules
	uint16 batt_i;          //battery current in centiamps 
    uint8 uvlo_stat;        //uvlo status
    uint16_t primary_i;
} telemetry_struct;
telemetry_struct telemetry;



#endif

//[] END OF FILE

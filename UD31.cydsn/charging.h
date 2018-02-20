/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#ifndef CHARGING_H
#define CHARGING_H
	
#define LOW_BATTERY_TIMEOUT			20000	//8khz ticks, 40000 = 5sec
#define CHARGE_TEST_TIMEOUT			80000	//10 sec
#define AC_PRECHARGE_TIMEOUT		100	//

#define BAT_PRECHARGE_BUS_SCHEME	0	//battery "directly" supplies bus voltage
#define BAT_BOOST_BUS_SCHEME	 	1	//battery is boosted via SLR converter
#define AC_PRECHARGE_BUS_SCHEME		2	//no batteries, mains powered 
#define AC_OR_BATBOOST_SCHEME		3	//might be battery powered, if not, then mains powered

#define BATTERY     0
#define BUS         1
#define RELAY_CHARGE      1
#define RELAY_OFF         0
#define RELAY_ON          3

#define BUS_COMMAND_OFF     0
#define BUS_COMMAND_ON      1
#define BUS_COMMAND_FAULT   2

volatile uint8 bus_command;	
	
void initialize_charging(void);
void run_battery_charging(void);
void control_precharge(void);


#endif

//[] END OF FILE

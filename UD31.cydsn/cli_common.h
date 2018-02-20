/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */


#include <device.h>


struct parameter_entry_struct {
	const char *name;
	uint16_t value;
	uint16_t min;
	uint16_t max;
	uint8_t (*updateFunction)(int newValue, uint8_t index);
    const char *help;
};

typedef struct parameter_entry_struct parameter_entry;

#define PARAM_PW 0
#define PARAM_PWD 1
#define PARAM_TUNE_SFREQ 2
#define PARAM_TUNE_EFREQ 3
#define PARAM_TUNE_PW 4
#define PARAM_TUNE_DELAY 5
#define PARAM_OFFTIME 6
#define PARAM_QCW_RAMP 7

#define CONF_WD 0
#define CONF_MAX_TR_PW 1
#define CONF_MAX_TR_PRF 2
#define CONF_MAX_QCW_PW 3
#define CONF_MAX_TR_CURRENT 4
#define CONF_MIN_TR_CURRENT 5
#define CONF_MAX_QCW_CURRENT 6
#define CONF_TEMP1_MAX 7
#define CONF_TEMP2_MAX 8
#define CONF_CT1_RATIO 9
#define CONF_CT2_RATIO 10
#define CONF_CT3_RATIO 11
#define CONF_CT1_BURDEN 12
#define CONF_CT2_BURDEN 13
#define CONF_CT3_BURDEN 14
#define CONF_LEAD_TIME 15
#define CONF_START_FREQ 16
#define CONF_START_CYCLES 17
#define CONF_MAX_TR_DUTY 18
#define CONF_MAX_QCW_DUTY 19
#define CONF_TEMP1_SETPOINT 20
#define CONF_EXT_TRIG_ENABLE 21
#define CONF_BATT_LOCKOUT_V 22
#define CONF_SLR_FSWITCH 23
#define CONF_SLR_VBUS 24
#define CONF_PS_SCHEME 25
#define CONF_AUTOTUNE_SAMPLES 26

#define MODE_MIDI 0
#define MODE_CLASSIC 1
#define MODE_UDCLASSIC 2

#define fan_controller          1   //enables fan controller
#define auto_charge_bus         0   //enables auto charging of DC bus on start up (no wait for command)
#define auto_charge_battery     0
#define ext_trig_runs_CW        0   //special test mode where holding the trigger runs the coil in CW mode



#define SERIAL 0
#define USB 1


uint8_t input_handle();
void input_interpret(uint8_t port);
void input_restart(void);
void Term_Move_Cursor(uint8_t row, uint8_t column,uint8_t port);
void Term_Erase_Screen(uint8_t port);
void Term_Color_Green(uint8_t port);
void Term_Color_Red(uint8_t port);
void Term_Color_White(uint8_t port);
void Term_Color_Cyan(uint8_t port);
void Term_BGColor_Blue(uint8_t port);
void Term_Save_Cursor(uint8_t port);
void Term_Restore_Cursor(uint8_t port);
void Term_Box(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2,uint8_t port);
void send_char(uint8_t c, uint8_t port);
void send_string(char * data, uint8_t port);
void send_buffer(uint8_t * data,uint16_t len, uint8_t port);
uint8_t term_config_changed(void);
void nt_interpret(const char *text, uint8_t port);
uint8_t eprom_load(void);

void initialize_term(void);
void task_terminal_overlay(void);

///Help
void task_terminal();

extern volatile parameter_entry tparameters[];
volatile uint8_t qcw_reg;
extern parameter_entry confparam[];

xSemaphoreHandle block_term[2];


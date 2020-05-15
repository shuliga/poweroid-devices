#define ID "PWR-PMS-32"

#ifdef MINI
#error This device cannot be run on MINI board versions
#endif

#include <Poweroid10.h>
#include <ultrasonic.h>
#include <MultiClick/MultiClick.h>
#include "poweroid_pump_station_3x2_state.h"
#include "poweroid_pump_station_3x2_prop.h"

#define IND_A IND_2
#define IND_B IND_3

#define PRESSURE_RAW_MIN 198
#define PRESSURE_MIN 0
#define PRESSURE_MAX 600

Timings timings = {0, 0, 0, 0};

TimingState FLASH(750L);
TimingState FLASH_ALARM(250L);

MultiClick btn = MultiClick(IN3_PIN);

Context CTX = Context(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

static uint16_t distance;
static uint16_t pressure;

void processSensors(){
    if (test_timer(TIMER_2HZ)){
        distance = ULTRASONIC.getDistance();
        pressure = PWR.SENS->getNormalizedSensor(SEN_2, PRESSURE_MIN, PRESSURE_MAX, PRESSURE_RAW_MIN);
    }
}

void applyTimings() {
    timings.alarm_pump.interval = (unsigned long) PROPS.FACTORY[3].runtime;
    timings.countdown_pump.interval = (unsigned long) PROPS.FACTORY[4].runtime;
    timings.countdown_lost_power.interval = (unsigned long) PROPS.FACTORY[5].runtime;
    timings.countdown_pre_power.interval = (unsigned long) PROPS.FACTORY[6].runtime;
}

void fillOutput() {
    processSensors();
    if (state_info == SI_ALARM) {
        BANNER.mode = 0;
        sprintf(BANNER.data.text, "%s", "ALARM");
    } else {
        BANNER.mode = 2;

        BANNER.data.gauges[0].val = pressure;
        BANNER.data.gauges[0].min = PROPS.FACTORY[1].runtime / PROPS.FACTORY[1].scale;
        BANNER.data.gauges[0].max = PROPS.FACTORY[2].runtime / PROPS.FACTORY[2].scale;
        BANNER.data.gauges[0].g_min = 0;
        BANNER.data.gauges[0].g_max = PROPS.FACTORY[2].maxv/ PROPS.FACTORY[2].scale;
        BANNER.data.gauges[0].measure = PROPS.FACTORY[1].measure;

        BANNER.data.gauges[1].val = distance;
        BANNER.data.gauges[1].min = PROPS.FACTORY[0].minv / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].max = PROPS.FACTORY[0].runtime / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].g_min = 0;
        BANNER.data.gauges[1].g_max = PROPS.FACTORY[0].maxv / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].measure = PROPS.FACTORY[0].measure;
    }
}

bool testLevel(){
    return distance > PROPS.FACTORY[0].runtime;
}

bool testPressure(){
    return pressure < PROPS.FACTORY[0].runtime;
}

void run_state_power(McEvent event) {
    switch (state_power) {
        case SP_OFF: {
            if (event == HOLD) {
                state_power = SP_PRE_POWER;
            }
            break;
        }
        case SP_PRE_POWER: {
            if (timings.countdown_pre_power.isTimeAfter(true)) {
                timings.countdown_pre_power.reset();
                if(!testPressure()){
                    gotoStateInfo(SI_ALARM);
                    break;
                }
                gotoStatePower(SP_POWER);
            }
            if(!testLevel()){
                gotoStatePower(SP_LOW_LEVEL);
                break;
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }
        case SP_POWER: {
            if (prev_state_power == SP_OFF) {
                timings.countdown_pump.reset();
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
                break;
            }
            if(!testPressure()){
                gotoStatePower(SP_LOST_POWER);
                break;
            }
            if(!testLevel()){
                gotoStatePower(SP_LOW_LEVEL);
                break;
            }
            break;
        }
        case SP_LOST_POWER: {
            if (timings.countdown_lost_power.isTimeAfter(true)) {
                gotoStatePower(SP_OFF);
                timings.countdown_lost_power.reset();
            }
            if(testPressure()){
                gotoStatePower(SP_POWER);
                break;
            }
            if(!testLevel()){
                gotoStatePower(SP_LOW_LEVEL);
                break;
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }
        case SP_LOW_LEVEL: {
            if(testLevel()){
                gotoStatePower(SP_PRE_POWER);
                break;
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }
    }
}


void run_state_pump(McEvent event) {
    switch (state_pump) {
        case SPM_PUMP_1: {
            prev_state_pump = SPM_PUMP_BOTH;
            if (event == CLICK ||  timings.countdown_pump.isTimeAfter(true) || state_info == SI_ALARM) {
                state_pump = SPM_PUMP_2;
            }
            break;
        }
        case SPM_PUMP_2: {
            if (prev_state_pump == SPM_PUMP_1) {
                timings.countdown_pump.reset();
            }

            if (state_info == SI_ALARM && timings.alarm_pump.countdown(true, false, false)) {
                gotoStatePump(SPM_PUMP_1);
                break;
            }
            if (event == HOLD) {
                gotoStatePump(SPM_PUMP_BOTH);
                break;
            }

            break;
        }
        case SPM_PUMP_1_ONLY: {
            if (event == CLICK ||  timings.countdown_pump.isTimeAfter(true) || state_info == SI_ALARM) {
                gotoStatePump(SPM_PUMP_2);
            }
            break;
        }
        case SPM_PUMP_BOTH: {
            if (timings.countdown_pre_power.countdown(true, true, false)) {
  //              state_pump
            }
            if (event == CLICK) {
                state_power = prev_state_power;
                break;
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }
    }
}

void runPowerStates() {
    McEvent event = btn.checkButton();
    run_state_power(event);
    run_state_pump(event);
}

void setup() {
    PWR.begin();
    ULTRASONIC.begin(SEN_1);
}

void loop() {

    PWR.run();

    bool powerWarning = state_power == SP_PRE_POWER || state_power == SP_LOST_POWER;
    bool power = state_power == SP_POWER || powerWarning;
    bool powerA = power && (state_pump == SPM_PUMP_1 || state_pump == SPM_PUMP_BOTH);
    bool powerB = power && (state_pump == SPM_PUMP_2 || state_pump == SPM_PUMP_BOTH);

    PWR.power(REL_A, powerA);
    PWR.power(REL_B, powerB);

    INDICATORS.set(IND_A, powerA);
    INDICATORS.set(IND_B, powerB);

    if (state_info == SI_ALARM) {
        INDICATORS.flash(IND_1, &FLASH_ALARM, true);
    } else if (powerWarning){
        INDICATORS.flash(IND_1, &FLASH, true);
    } else {
        INDICATORS.set(IND_1, false);
    }
}
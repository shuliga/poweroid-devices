#define ID "PWR-PMS-32"

#ifdef MINI
#error This device cannot be run on MINI board versions
#endif

#include <Poweroid10.h>
#include <ultrasonic.h>
#include <MultiClick/MultiClick.h>
#include "poweroid_pump_station_3x2_state.h"
#include "poweroid_pump_station_3x2_prop.h"

#define PRESSURE_RAW_MIN 198
#define PRESSURE_MIN 0
#define PRESSURE_MAX 600

#define IND_BUTTON IND_1
#define IND_WARN IND_2
#define IND_ALARM IND_3

Timings timings = {0, 0, 0, 0, 0};

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

bool testPressureWorking(){
    return pressure >= PROPS.FACTORY[1].runtime;
}

bool testPressureMax(){
    return pressure >= PROPS.FACTORY[2].runtime;
}

void run_state_basin() {
    switch (state_basin) {
        case SB_INTAKE: {
            if(!testLevel()){
                gotoStateBasin(SB_LOW_WATER);
                break;
            }
            if (distance <= 3){
                gotoStateBasin(SB_SENSOR_FAILED);
                break;
            }
            break;
        }
        case SB_LOW_WATER: {
            if( timings.low_water.isTimeAfter(testLevel())){
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_SENSOR_FAILED: {
            if(testLevel()){
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        default:;
    }
}

void run_state_info() {
    switch (state_info) {
        case SI_ALARM: {
            if(state_basin != SB_SENSOR_FAILED){
                gotoStateInfo(SI_WARNING);
                break;
            }
            break;
        }
        case SI_WARNING: {
            if(state_basin == SB_SENSOR_FAILED){
                gotoStateInfo(SI_ALARM);
                break;
            }
            if (
                    state_basin != SB_LOW_WATER ||
                    state_power != SP_FAILED
            ){
                gotoStateInfo(SI_DISARM);
            }
            break;
        }
        case SI_DISARM: {
            if(
                    state_basin == SB_SENSOR_FAILED ||
                    state_basin == SB_LOW_WATER ||
                    state_power == SP_FAILED
             ){
                gotoStateInfo(SI_WARNING);
                break;
            }
            break;
        }
        default:;
    }
}

void run_state_power(McEvent event) {
    switch (state_power) {
        case SP_OFF: {
            if (event == CLICK) {
                gotoStatePower(SP_PRE_POWER);
            }
            break;
        }
        case SP_SUSPEND: {
            if (event == CLICK || state_basin == SB_INTAKE) {
                gotoStatePower(SP_PRE_POWER);
            }
            break;
        }
        case SP_PRE_POWER: {
            if (state_basin == SB_LOW_WATER){
                gotoStatePower(SP_SUSPEND);
            }
            if (timings.countdown_pre_power.isTimeAfter(true)) {
                timings.countdown_pre_power.reset();
                if(!testPressureWorking()){
                    gotoStatePower(SP_OFF);
                    break;
                }
                gotoStatePower(SP_POWER);
            }
            if (event == CLICK) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }
        case SP_POWER: {
            if (prev_state_power == SP_OFF) {
                timings.countdown_pump.reset();
            }
            if (event == CLICK) {
                gotoStatePower(SP_OFF);
                break;
            }
            if(!testPressureWorking()){
                gotoStatePower(SP_FAILED);
                break;
            }
            if(testPressureMax()){
                gotoStatePower(SP_DISCHARGE);
                break;
            }
            break;
        }
        case SP_DISCHARGE: {
            if (event == CLICK) {
                gotoStatePower(SP_POWER);
                break;
            }
            if(!testPressureWorking()){
                gotoStatePower(SP_POWER);
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
            if (event == HOLD ||  timings.countdown_pump.isTimeAfter(true) || state_info == SI_ALARM) {
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
    run_state_basin();
    run_state_info();
    run_state_pump(event);
    run_state_power(event);
}

void setup() {
    PWR.begin();
    ULTRASONIC.begin(SEN_1);
}

void loop() {

    PWR.run();
    bool warning = state_info == SI_WARNING;
    bool alarm = state_info == SI_ALARM;

    bool power = state_power == SP_POWER || SP_PRE_POWER;
    bool powerA = power && (state_pump == SPM_PUMP_1 || state_pump == SPM_PUMP_BOTH);
    bool powerB = power && (state_pump == SPM_PUMP_2 || state_pump == SPM_PUMP_BOTH);

    PWR.power(REL_A, powerA);
    PWR.power(REL_B, powerB);

    INDICATORS.set(IND_WARN, warning);
    INDICATORS.set(IND_ALARM, alarm);

    if (alarm) {
        INDICATORS.flash(IND_BUTTON, &FLASH_ALARM, true);
    } else if (warning){
        INDICATORS.flash(IND_BUTTON, &FLASH, true);
    } else {
        INDICATORS.set(IND_BUTTON, false);
    }
}

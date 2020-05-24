#define ID "PWR-PMS-32"

#ifdef MINI
#error This device cannot be run on MINI board versions
#endif

#include <Poweroid10.h>
#include <ultrasonic.h>
#include <MultiClick/MultiClick.h>
#include "poweroid_pump_station_3x2_state.h"
#include "poweroid_pump_station_3x2_prop.h"
#include "smoother.h"

#define PRESSURE_MIN 0
#define PRESSURE_MAX 600
#define PRESSURE_RAW_0 198

#define IND_BUTTON IND_2
#define IND_WARN IND_1
#define IND_ALARM IND_3

Timings timings = {0, 0, 0, 0};

TimingState FLASH(750L);
TimingState FLASH_SUSPEND(500L);
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

static Smoother DIST_SMTH;
static Smoother PRESS_SMTH;

static int16_t distance;
static int16_t pressure;

void processSensors() {
    if (test_timer(TIMER_2HZ)) {
        distance = DIST_SMTH.feedAndGet(ULTRASONIC.getDistance());
        pressure = PRESS_SMTH.feedAndGet(
                PWR.SENS->getNormalizedSensor(SEN_2, PRESSURE_MIN, PRESSURE_MAX, PRESSURE_RAW_0));
    }
}

void applyTimings() {
    timings.countdown_pre_power.interval = (unsigned long) PROPS.FACTORY[3].runtime;
    timings.countdown_pump_switch.interval = (unsigned long) PROPS.FACTORY[4].runtime;
    timings.alarm_pump.interval = (unsigned long) PROPS.FACTORY[5].runtime;
    timings.intake_level_delay.interval = (unsigned long) PROPS.FACTORY[6].runtime;
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
        BANNER.data.gauges[0].g_max = PROPS.FACTORY[2].maxv / PROPS.FACTORY[2].scale;
        BANNER.data.gauges[0].measure = PROPS.FACTORY[1].measure;

        BANNER.data.gauges[1].val = distance;
        BANNER.data.gauges[1].min = PROPS.FACTORY[0].minv / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].max = PROPS.FACTORY[0].runtime / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].g_min = 0;
        BANNER.data.gauges[1].g_max = PROPS.FACTORY[0].maxv / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[1].measure = PROPS.FACTORY[0].measure;
    }
}

bool testIntakeLevel() {
    return distance < PROPS.FACTORY[0].runtime;
}

bool testWorkingPressure() {
    return pressure >= PROPS.FACTORY[1].runtime;
}

bool testPressureMax() {
    return pressure >= PROPS.FACTORY[2].runtime;
}

void run_state_basin() {
    switch (state_basin) {
        case SB_INTAKE: {
            if (!testIntakeLevel()) {
                gotoStateBasin(SB_LOW_WATER);
                break;
            }
            if (distance <= 3) {
                gotoStateBasin(SB_SENSOR_FAILED);
                break;
            }
            break;
        }
        case SB_LOW_WATER: {
            if (timings.intake_level_delay.isTimeAfter(testIntakeLevel())) {
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_SENSOR_FAILED: {
            if (testIntakeLevel()) {
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        default:;
    }
}

void run_state_info(McEvent event) {
    switch (state_info) {
        case SI_ALARM: {
            if (prev_state_info != SI_ALARM){
                prev_state_info = SI_ALARM;
                strncpy(CUSTOM_HEADER, "(!)", 3);
                CTX.refreshState = true;
            }
            if (event == DOUBLE_CLICK) {
                gotoStateInfo(SI_DISARM);
                break;
            }
            if (state_basin != SB_SENSOR_FAILED && state_power != SP_FAILED) {
                gotoStateInfo(SI_WARNING);
                break;
            }
            break;
        }
        case SI_WARNING: {
            if (prev_state_info != SI_WARNING){
                prev_state_info = SI_WARNING;
//                strncpy(CUSTOM_HEADER, "  ", 3);
            }
            if (event == DOUBLE_CLICK) {
                gotoStateInfo(SI_DISARM);
                break;
            }
            if (
                    state_basin == SB_SENSOR_FAILED ||
                    state_power == SP_FAILED
                    ) {
                gotoStateInfo(SI_ALARM);
                break;
            }
            if (
                    state_basin != SB_LOW_WATER &&
                    state_power != SP_FAILED
                    ) {
                gotoStateInfo(SI_DISARM);
            }
            break;
        }
        case SI_DISARM: {
            if (prev_state_info != SI_DISARM){
                prev_state_info = SI_DISARM;
                strncpy(CUSTOM_HEADER, "   ", 3);
                CTX.refreshState = true;
            }
            if (
                    state_basin == SB_SENSOR_FAILED ||
                    state_basin == SB_LOW_WATER ||
                    state_power == SP_FAILED
                    ) {
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
            if (event == CLICK || state_timer == ST_ENGAGE) {
                gotoStatePower(SP_PRE_POWER);
                timings.countdown_pre_power.reset();
            }
            break;
        }

        case SP_PRE_POWER: {
            if (state_basin == SB_LOW_WATER) {
                gotoStatePower(SP_SUSPEND);
                break;
            }
            if (timings.countdown_pre_power.isTimeAfter(true)) {
                Serial.println(timings.countdown_pre_power.interval);
                timings.countdown_pre_power.reset();
                if (!testWorkingPressure()) {
                    gotoStatePower(SP_FAILED);
                    break;
                }
                gotoStatePower(SP_POWER);
                break;
            }
            if (testPressureMax()) {
                gotoStatePower(SP_DISCHARGE);
                break;
            }
            if (event == CLICK) {
                gotoStatePower(SP_OFF);
                break;
            }
            break;
        }

        case SP_SUSPEND: {
            if (event == CLICK || state_basin == SB_INTAKE) {
                gotoStatePower(SP_PRE_POWER);
                timings.countdown_pre_power.reset();
            }
            break;
        }

        case SP_POWER: {
            if (prev_state_power == SP_OFF) {
                timings.countdown_pump_switch.reset();
            }
            if (event == CLICK) {
                gotoStatePower(SP_OFF);
                break;
            }
            if (!testWorkingPressure()) {
                gotoStatePower(SP_FAILED);
                break;
            }
            if (testPressureMax()) {
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
            if (!testWorkingPressure()) {
                gotoStatePower(SP_PRE_POWER);
                break;
            }
            break;
        }
        case SP_FAILED: {
            if (state_info == SI_DISARM) {
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
            if (prev_state_pump != SPM_PUMP_1) {
                timings.countdown_pump_switch.reset();
                prev_state_pump = SPM_PUMP_2;
            }

            if (event == HOLD || timings.countdown_pump_switch.isTimeAfter(true)) {
                state_pump = SPM_PUMP_2;
            }

            break;
        }

        case SPM_PUMP_2: {
            if (prev_state_pump != SPM_PUMP_2) {
                timings.countdown_pump_switch.reset();
                prev_state_pump = SPM_PUMP_2;
            }

            if (event == HOLD || timings.countdown_pump_switch.isTimeAfter(true)) {
                gotoStatePump(SPM_PUMP_1);
                break;
            }

            break;
        }

    }
}

bool isInTimeSpan(uint8_t hrs_start, uint8_t min_start, uint8_t hrs_stop, uint8_t min_stop, uint8_t hrs, uint8_t min){
    uint16_t start = hrs_start * 60  + min_start;
    uint16_t stop = hrs_stop * 60  + min_stop;
    uint16_t actual = hrs * 60  + min;
    return start < stop ? actual >= start && actual < stop : actual >= start || actual < stop;
}

void run_state_timer() {
    switch (state_timer) {

        case ST_STAND_BY: {
            if (prev_state_timer != ST_STAND_BY || (changedState[3] && state_info == SI_DISARM)) {
                prev_state_timer = ST_STAND_BY;
                strncpy(CUSTOM_HEADER, " T ", 3);
            }
            if (isInTimeSpan(PROPS.FACTORY[7].runtime, PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime, PROPS.FACTORY[10].runtime, hrs, min)) {
                gotoStateTimer(ST_ENGAGE);
            }
            break;
        }

        case ST_ENGAGE: {
            if (prev_state_timer != ST_ENGAGE || (changedState[3] && state_info == SI_DISARM) ) {
                prev_state_timer = ST_ENGAGE;
                strncpy(CUSTOM_HEADER, ">T<", 3);
            }
            if (!isInTimeSpan(PROPS.FACTORY[7].runtime, PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime, PROPS.FACTORY[10].runtime, hrs, min)) {
                gotoStateTimer(ST_STAND_BY);
            }
            break;
        }

        case ST_DISARM: {
            if (prev_state_timer != ST_DISARM || (changedState[3] && state_info == SI_DISARM) ) {
                prev_state_timer = ST_DISARM;
                strncpy(CUSTOM_HEADER, "   ", 3);
            }

            break;
        }

    }
}

void runPowerStates() {
    McEvent event = btn.checkButton();
    run_state_basin();
    run_state_info(event);
    run_state_timer();
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

    bool power = state_power == SP_POWER || state_power == SP_PRE_POWER;
    bool powerA = power && (state_pump == SPM_PUMP_1 || state_pump == SPM_PUMP_BOTH);
    bool powerB = power && (state_pump == SPM_PUMP_2 || state_pump == SPM_PUMP_BOTH);

    PWR.power(REL_A, powerA);
    PWR.power(REL_B, powerB);

    INDICATORS.set(IND_WARN, warning);
    INDICATORS.set(IND_ALARM, alarm);

    if (alarm || warning) {
        INDICATORS.flash(IND_BUTTON, alarm ? FLASH_ALARM.flash() : FLASH.flash(), true);
    } else {
        INDICATORS.set(IND_BUTTON, power);
    }
}

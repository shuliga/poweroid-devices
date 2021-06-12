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

#define POWER_NO_CUT_MILS 2500L

Timings timings = {0, POWER_NO_CUT_MILS, 0, 0, 0};

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

static char *failedState;

void processSensors() {
    if (test_timer(TIMER_2HZ)) {
        distance = DIST_SMTH.feedAndGet(ULTRASONIC.getDistance());
        pressure = PRESS_SMTH.feedAndGet(
                PWR.SENS->getNormalizedSensor(SEN_2, PRESSURE_MIN, PRESSURE_MAX, PRESSURE_RAW_0));
    }
}

void applyTimings() {
    timings.pre_power_timeout.interval = (unsigned long) PROPS.FACTORY[3].runtime;
    timings.countdown_pump_switch.interval = (unsigned long) PROPS.FACTORY[4].runtime;
    timings.alarm_pump.interval = (unsigned long) PROPS.FACTORY[5].runtime;
    timings.intake_level_delay.interval = (unsigned long) PROPS.FACTORY[6].runtime;
}

void fillOutput() {
    if (stateHolderInfo.state == SI_ALARM) {
        BANNER.mode = 0;
        sprintf(BANNER.data.text, "%s", "ALARM");
    } else {
        BANNER.mode = 4;

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

        static const char *_fmt = "%s: %s";

        strcpy(EXTRA_BUFF[0], stateHolderPower.getState());
        if (failedState == NULL){
            strcpy(EXTRA_BUFF[1], stateHolderPump.getState());
        } else {
            sprintf(EXTRA_BUFF[1], _fmt, stateHolderInfo.getState(), failedState);
        }

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

bool testPressureLow() {
    return pressure < PROPS.FACTORY[1].runtime * (1.0 + PROPS.FACTORY[6].runtime / 100.0);
}

void run_state_basin(McEvent event) {
    switch (stateHolderBasin.state) {
        case SB_FULL: {
            stateHolderBasin.firstState(SB_FULL);
            if (!testIntakeLevel()) {
                stateHolderBasin.gotoState(SB_LOW_WATER);
                break;
            }
            if (distance <= 3) {
                stateHolderBasin.gotoState(SB_SENSOR_FAILED);
                break;
            }
            break;
        }
        case SB_LOW_WATER: {
            if (stateHolderBasin.firstState(SB_LOW_WATER)){
                timings.intake_level_delay.reset();
            }
            if (timings.intake_level_delay.isTimeAfter(testIntakeLevel()) ||
                (testIntakeLevel() && event == DOUBLE_CLICK)) {
                stateHolderBasin.gotoState(SB_FULL);
                break;
            }
            break;
        }
        case SB_SENSOR_FAILED: {
            stateHolderBasin.firstState(SB_SENSOR_FAILED);
            if (testIntakeLevel()) {
                stateHolderBasin.gotoState(SB_FULL);
                break;
            }
            break;
        }
        default:;
    }
}

void run_state_info(McEvent event) {
    switch (stateHolderInfo.state) {
        case SI_ALARM: {
            if (stateHolderInfo.firstState(SI_ALARM)) {
                strncpy(CUSTOM_HEADER, "(!)", 3);
                CTX.refreshState = true;
            }
            if (event == DOUBLE_CLICK) {
                stateHolderInfo.gotoState(SI_DISARM);
                break;
            }
            break;
        }
        case SI_WARNING: {
            if (stateHolderInfo.firstState(SI_WARNING)) {
//                strncpy(CUSTOM_HEADER, "  ", 3);
            }
            if (
                    stateHolderBasin.state == SB_SENSOR_FAILED ||
                            stateHolderPower.state == SP_FAILED
                    ) {
                stateHolderInfo.gotoState(SI_ALARM);
                break;
            }
            if (
                    stateHolderBasin.state != SB_LOW_WATER &&
                            !(stateHolderPower.state == SP_POWER && testPressureLow())
                    ) {
                stateHolderInfo.gotoState(SI_DISARM);
            }
            break;
        }
        case SI_DISARM: {
            if (stateHolderInfo.firstState(SI_DISARM)) {
                strncpy(CUSTOM_HEADER, "   ", 3);
                CTX.refreshState = true;
                failedState = NULL;
            }
            if (
                    stateHolderBasin.state == SB_SENSOR_FAILED ||
                            stateHolderPower.state == SP_FAILED
                    ) {
                strcpy(failedState, stateHolderBasin.getState());
                stateHolderInfo.gotoState(SI_ALARM);
                break;
            }
            if (
                    stateHolderPower.state == SB_LOW_WATER ||
                    (stateHolderPower.state == SP_POWER && testPressureLow())
                    ) {
                stateHolderInfo.gotoState(SI_WARNING);
                break;
            }
            break;
        }
        default:;
    }
}

void run_state_power(McEvent event) {
    switch (stateHolderPower.state) {
        case SP_OFF: {
            stateHolderPower.firstState(SP_OFF);
            if (event == CLICK || stateHolderTimer.state == ST_ENGAGE) {
                stateHolderPower.gotoState(SP_PRE_POWER);
            }
            break;
        }

        case SP_PRE_POWER: {
            if (stateHolderPower.firstState(SP_PRE_POWER)) {
                timings.pre_power_timeout.reset();
            }

            if (event == CLICK) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }

            if (stateHolderBasin.state == SB_LOW_WATER) {
                stateHolderPower.gotoState(SP_SUSPEND);
                break;
            }

            if (testWorkingPressure()) {
                stateHolderPower.gotoState(SP_POWER);
                break;
            }

            if (timings.pre_power_timeout.isTimeAfter(true)) {
                stateHolderPower.gotoState(SP_FAILED);
                break;
            }

            break;
        }

        case SP_SUSPEND: {
            stateHolderPower.firstState(SP_SUSPEND);
            if (event == CLICK || stateHolderBasin.state == SB_FULL) {
                stateHolderPower.gotoState(SP_PRE_POWER);
            }
            break;
        }

        case SP_POWER: {

            if (stateHolderPower.firstState(SP_POWER)) {
//TODO this prev state seem not appear in this place
                if (stateHolderPower.prev_state == SP_OFF) {
                    timings.countdown_pump_switch.reset();
                }

                timings.power_fail_delay.reset();
            }

            if (event == CLICK) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }

            if (stateHolderBasin.state == SB_LOW_WATER) {
                stateHolderPower.gotoState(SP_SUSPEND);
                break;
            }

            if (timings.power_fail_delay.isTimeAfter(!testWorkingPressure())) {
                stateHolderPower.gotoState(SP_FAILED);
                break;
            }

            if (testPressureMax()) {
                stateHolderPower.gotoState(SP_DISCHARGE);
                break;
            }
            break;
        }

        case SP_DISCHARGE: {
            stateHolderPower.firstState(SP_DISCHARGE);
            if (event == CLICK) {
                stateHolderPower.gotoState(SP_POWER);
                break;
            }
            if (event == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            if (!testWorkingPressure()) {
                stateHolderPower.gotoState(SP_PRE_POWER);
                break;
            }
            break;
        }
        case SP_FAILED: {
            stateHolderPower.firstState(SP_FAILED);
            if (stateHolderInfo.state == SI_DISARM) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            break;
        }
    }

}

void run_state_pump(McEvent event) {
    switch (stateHolderPump.state) {

        case SPM_PUMP_1_ONLY: {
            stateHolderPump.firstState(SPM_PUMP_2_ONLY);
            if (event == HOLD) {
                stateHolderPump.gotoState(SPM_PUMP_2_ONLY);
            }
            break;
        }

        case SPM_PUMP_2_ONLY: {
            stateHolderPump.firstState(SPM_PUMP_1_ONLY);
            if (event == HOLD) {
                stateHolderPump.gotoState(SPM_PUMP_1);
            }
            break;
        }

        case SPM_PUMP_1: {
            if (stateHolderPump.firstState(SPM_PUMP_1)) {
                timings.countdown_pump_switch.reset();
            }

            if (event == HOLD || timings.countdown_pump_switch.isTimeAfter(true)) {
                stateHolderPump.gotoState(SPM_PUMP_2);
            }

            break;
        }

        case SPM_PUMP_2: {
            if (stateHolderPump.firstState(SPM_PUMP_2)) {
                timings.countdown_pump_switch.reset();
            }

            if (timings.countdown_pump_switch.isTimeAfter(true)) {
                stateHolderPump.gotoState(SPM_PUMP_1);
                break;
            }

            if (event == HOLD) {
                stateHolderPump.gotoState(SPM_PUMP_1_ONLY);
                break;
            }

            break;
        }

    }
}

bool isInTimeSpan(uint8_t hrs_start, uint8_t min_start, uint8_t hrs_stop, uint8_t min_stop, uint8_t hrs, uint8_t min) {
    uint16_t start = hrs_start * 60 + min_start;
    uint16_t stop = hrs_stop * 60 + min_stop;
    uint16_t actual = hrs * 60 + min;
    return start < stop ? actual >= start && actual < stop : actual >= start || actual < stop;
}

#ifdef RTCM
void run_state_timer() {
    switch (stateHolderTimer.state) {

        case ST_STAND_BY: {
            if (stateHolderTimer.firstState(ST_STAND_BY) || (stateHolderInfo.wasChanged() && stateHolderInfo.state == SI_DISARM)) {
                strncpy(CUSTOM_HEADER, " T ", 3);
            }
            if (isInTimeSpan(PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime, PROPS.FACTORY[10].runtime,
                             PROPS.FACTORY[11].runtime, hrs, min)) {
                stateHolderTimer.gotoState(ST_ENGAGE);
            }
            break;
        }

        case ST_ENGAGE: {
            if (stateHolderTimer.firstState(ST_ENGAGE) || (stateHolderInfo.wasChanged() && stateHolderInfo.state == SI_DISARM)) {
                strncpy(CUSTOM_HEADER, ">T<", 3);
            }
            if (!isInTimeSpan(PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime, PROPS.FACTORY[10].runtime,
                              PROPS.FACTORY[11].runtime, hrs, min)) {
                stateHolderTimer.gotoState(ST_STAND_BY);
            }
            break;
        }

        case ST_DISARM: {
            if (stateHolderTimer.firstState(ST_DISARM) || (stateHolderInfo.wasChanged() && stateHolderInfo.state == SI_DISARM)) {
                strncpy(CUSTOM_HEADER, "   ", 3);
            }

            break;
        }

    }
}
#endif

void runPowerStates() {
    McEvent event = btn.checkButton();
    run_state_basin(event);
    run_state_info(event);
#ifdef RTCM
    run_state_timer();
#endif
    run_state_pump(event);
    run_state_power(event);
}

void setup() {
    PWR.begin();
    ULTRASONIC.begin(SEN_1);
}

void loop() {

    PWR.run();
    bool warning = stateHolderInfo.state == SI_WARNING;
    bool alarm = stateHolderInfo.state == SI_ALARM;

    bool power = stateHolderPower.state == SP_POWER || stateHolderPower.state == SP_PRE_POWER;
    bool powerA = power && (stateHolderPump.state == SPM_PUMP_1 || stateHolderPump.state == SPM_PUMP_1_ONLY || stateHolderPump.state == SPM_PUMP_BOTH);
    bool powerB = power && (stateHolderPump.state == SPM_PUMP_2 || stateHolderPump.state == SPM_PUMP_2_ONLY || stateHolderPump.state == SPM_PUMP_BOTH);

    PWR.power(REL_A, powerA);
    PWR.power(REL_B, powerB);

    INDICATORS.set(IND_WARN, warning && flash_symm(timerCounter_1Hz));
    INDICATORS.set(IND_ALARM, alarm && flash_symm(timerCounter_1Hz));

    if (alarm || warning) {
        INDICATORS.flash(IND_BUTTON, alarm ? flash_symm(timerCounter_4Hz) : flash_symm(timerCounter_1Hz), true);
    } else {
        INDICATORS.set(IND_BUTTON, power);
    }
}

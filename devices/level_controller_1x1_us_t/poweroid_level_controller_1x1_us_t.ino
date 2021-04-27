#define ID "PWR-PMS-32"

#include <Poweroid10.h>
#include <ultrasonic.h>
#include <MultiClick/MultiClick.h>
#include "poweroid_level_controller_1x1_us_t_state.h"
#include "poweroid_level_controller_1x1_us_t_prop.h"
#include "smoother.h"

#define IND_POWER IND_1
#define BUTT_POWER IN_1

#define IND_WARN IND_1
#define IND_ALARM IND_3

Context CTX(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

static Smoother DIST_SMTH;

static int16_t distance;

static char *failedState;

void processSensors() {
    if (test_timer(TIMER_2HZ)) {
        distance = DIST_SMTH.feedAndGet(ULTRASONIC.getDistance());
    }
}

void applyTimings() {
//    timings.alarm_pump.interval = (unsigned long) PROPS.FACTORY[5].runtime;
//    timings.intake_level_delay.interval = (unsigned long) PROPS.FACTORY[6].runtime;
}

void fillOutput() {
    if (state_info == SI_ALARM) {
        BANNER.mode = 0;
        sprintf(BANNER.data.text, "%s", "ALARM");
    } else {
        BANNER.mode = 3;

        BANNER.data.gauges[0].val = distance;
        BANNER.data.gauges[0].min = PROPS.FACTORY[1].runtime / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[0].max = PROPS.FACTORY[0].runtime / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[0].g_min = 0;
        BANNER.data.gauges[0].g_max = PROPS.FACTORY[0].maxv / PROPS.FACTORY[0].scale;
        BANNER.data.gauges[0].measure = PROPS.FACTORY[0].measure;

        static const char *_fmt = "%s: %s";

        RunState *power_state = getState(S_POWER);
        strcpy(EXTRA_BUFF[0], power_state->state);
    }
}

bool testIntakeLevel() {
    return distance < PROPS.FACTORY[0].runtime / PROPS.FACTORY[0].scale;
}

bool testOverflow() {
    return distance < PROPS.FACTORY[1].runtime / PROPS.FACTORY[1].scale;
}

void run_state_basin(McEvent event) {

    if (distance <= 3) {
        gotoStateBasin(SB_SENSOR_FAILED);
    }

    switch (state_basin) {
        case SB_FULL: {
            if (!testOverflow()) {
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_INTAKE: {
            if (!testIntakeLevel()) {
                gotoStateBasin(SB_LOW_WATER);
                break;
            }
            if (testOverflow()) {
                gotoStateBasin(SB_FULL);
                break;
            }
        }
        case SB_LOW_WATER: {
            if (testIntakeLevel()) {
                gotoStateBasin(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_SENSOR_FAILED: {
            if (testIntakeLevel()) {
                gotoStateBasin(SB_INTAKE);
            }
            break;
        }
        default:;
    }
}

void run_state_info(McEvent event) {
    switch (state_info) {
        case SI_ALARM: {
            if (prev_state_info != SI_ALARM) {
                prev_state_info = SI_ALARM;
                strncpy(CUSTOM_HEADER, "(!)", 3);
                CTX.refreshState = true;
            }
            if (event == DOUBLE_CLICK) {
                gotoStateInfo(SI_DISARM);
                break;
            }
            break;
        }
        case SI_WARNING: {
            if (prev_state_info != SI_WARNING) {
                prev_state_info = SI_WARNING;
                strncpy(CUSTOM_HEADER, "  ", 3);
            }
            if (event == DOUBLE_CLICK) {
                gotoStateInfo(SI_DISARM);
                break;
            }
            if (
                    state_basin == SB_SENSOR_FAILED
                    ) {
                gotoStateInfo(SI_ALARM);
                break;
            }
            if (
                    state_basin != SB_LOW_WATER
                    ) {
                gotoStateInfo(SI_DISARM);
            }
            break;
        }
        case SI_DISARM: {
            if (prev_state_info != SI_DISARM) {
                prev_state_info = SI_DISARM;
                strncpy(CUSTOM_HEADER, "   ", 3);
                CTX.refreshState = true;
                failedState = NULL;
            }
            if (
                state_power != SP_OFF && (
                     state_basin == SB_SENSOR_FAILED ||
                     state_basin == SB_LOW_WATER
                    )
                ) {
                RunState * run_state_basin = getState(S_BASIN);
                failedState = run_state_basin->state;
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
            if (event == HOLD || state_timer == ST_ENGAGE) {
                gotoStatePower(SP_AUTO);
            }
            break;
        }

        case SP_AUTO: {

            if (event == CLICK || state_timer == ST_ENGAGE) {
                gotoStatePower(SP_FILL_IN);
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
            }
            break;
        }

        case SP_FILL_IN: {
            if (event == CLICK) {
                gotoStatePower(SP_AUTO);
                break;
            }
            if (event == HOLD) {
                gotoStatePower(SP_OFF);
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
    switch (state_timer) {

        case ST_SHUT_DOWN: {
            if (prev_state_timer != ST_SHUT_DOWN || (changedState[3] && state_info == SI_DISARM)) {
                prev_state_timer = ST_SHUT_DOWN;
                strncpy(CUSTOM_HEADER, " T ", 3);
            }
            if (isInTimeSpan(PROPS.FACTORY[7].runtime, PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime,
                             PROPS.FACTORY[10].runtime, hrs, min)) {
                gotoStateTimer(ST_ENGAGE);
            }
            break;
        }

        case ST_ENGAGE: {
            if (prev_state_timer != ST_ENGAGE || (changedState[3] && state_info == SI_DISARM)) {
                prev_state_timer = ST_ENGAGE;
                strncpy(CUSTOM_HEADER, ">T<", 3);
            }
            if (!isInTimeSpan(PROPS.FACTORY[7].runtime, PROPS.FACTORY[8].runtime, PROPS.FACTORY[9].runtime,
                              PROPS.FACTORY[10].runtime, hrs, min)) {
                gotoStateTimer(ST_SHUT_DOWN);
            }
            break;
        }

        case ST_DISARM: {
            if (prev_state_timer != ST_DISARM || (changedState[3] && state_info == SI_DISARM)) {
                prev_state_timer = ST_DISARM;
                strncpy(CUSTOM_HEADER, "   ", 3);
            }

            break;
        }

    }
}
#endif

void runPowerStates() {
    McEvent event = CTRL.BUTTONS[0];
    run_state_basin(event);
    run_state_info(event);
#ifdef RTCM
    run_state_timer();
#endif
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

    bool power = state_power == SP_FILL_IN && (state_basin == SB_INTAKE || state_basin == SB_LOW_WATER) || (state_power == SP_AUTO && state_basin == SB_LOW_WATER);
    PWR.power(REL_A, power);

    INDICATORS.set(IND_WARN, warning && flash_symm(timerCounter_1Hz));
    INDICATORS.set(IND_ALARM, alarm && flash_symm(timerCounter_4Hz));

    if (alarm || warning) {
        INDICATORS.flash(IND_POWER, alarm ? flash_symm(timerCounter_4Hz) : flash_symm(timerCounter_1Hz), true);
    } else {
        INDICATORS.set(IND_POWER, power);
    }
}

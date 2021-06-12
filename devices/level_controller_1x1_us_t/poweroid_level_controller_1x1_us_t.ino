#define ID "PWR-PLC-32"

#include <Poweroid10.h>
#include <ultrasonic.h>
#include <MultiClick/MultiClick.h>
#include <smoother.h>
#include <timings.h>
#include "poweroid_level_controller_1x1_us_t_state.h"
#include "poweroid_level_controller_1x1_us_t_prop.h"

#define IND_POWER       IND_1
#define IND_POWER_MAN   IND_2

#define BUTT_POWER_AUTO SEN_2
#define BUTT_POWER_MAN  SEN_3

#define ULTRASONIC_SEN  SEN_1

#define IND_WARN        IND_1
#define IND_ALARM       IND_3

Context CTX(SIGNATURE, ID, PROPS.FACTORY, PROPS.props_size, PROPS.DEFAULT_PROPERTY);

Commander CMD(CTX);
Bt BT(CTX.id);

#ifndef NO_CONTROLLER
Controller CTRL(CTX, CMD);
Pwr PWR(CTX, &CMD, &CTRL, &BT);
#else
Pwr PWR(CTX, &CMD, NULL, &BT);
#endif

TimingState timer(0);
TimingState levelCheck(0);
TimingState levelCheckOverflow(0);
TimeSplit toGo;

static Smoother DIST_SMTH;
static int16_t distance;

void processSensors() {
    if (test_timer(TIMER_2HZ)) {
        distance = DIST_SMTH.feedAndGet(ULTRASONIC.getDistance());
    }
}

void applyTimings() {
    timer.interval = (PROPS.FACTORY[2].runtime * 60 + PROPS.FACTORY[3].runtime) * 1000 * 60;
    levelCheck.interval = PROPS.FACTORY[4].runtime;
    levelCheckOverflow.interval = PROPS.FACTORY[4].runtime;
}

void fillOutput() {
    if (stateHolderInfo.state == SI_ALARM) {
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

        if (stateHolderTimer.state == ST_ENGAGE ){
            splitTime(timer.millsToGo(), toGo);
            sprintf(EXTRA_BUFF[0],"-%d:%d:%d", toGo.hrs,toGo.mins, toGo.sec );
        } else {
            strcpy(EXTRA_BUFF[0], stateHolderPower.getState());
        }
    }
}

bool testIntakeLevel() {
    return distance < PROPS.FACTORY[0].runtime / PROPS.FACTORY[0].scale;
}

bool testOverflow() {
    return distance < PROPS.FACTORY[1].runtime / PROPS.FACTORY[1].scale;
}

void run_state_power(McEvent eventA, McEvent eventB) {

    if (eventB == HOLD && stateHolderPower.state != SP_MAN_ON) {
        stateHolderPower.gotoState(SP_MAN_ON);
        eventB = NOTHING;
    }

    switch (stateHolderPower.state) {

        case SP_DISARM: {
            stateHolderPower.firstState(SP_DISARM);
            break;
        }

        case SP_OFF: {
            stateHolderPower.firstState(SP_OFF);
            if (eventA == HOLD) {
                stateHolderPower.gotoState(SP_AUTO);
            }
            break;
        }

        case SP_AUTO: {
            stateHolderPower.firstState(SP_AUTO);
            if (eventA == CLICK ) {
                stateHolderPower.gotoState(SP_FILL_IN);
            }
            if (eventA == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
            }
            break;
        }

        case SP_FILL_IN: {
            stateHolderPower.firstState(SP_FILL_IN);
            if (eventA == CLICK | stateHolderBasin.state == SB_FULL) {
                stateHolderPower.gotoState(SP_AUTO);
                break;
            }
            if (eventA == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
            }
            break;
        }

        case SP_MAN_ON: {
            stateHolderPower.firstState(SP_MAN_ON);
            if (eventB == CLICK) {
                stateHolderPower.gotoState(SP_AUTO);
                break;
            }
            if (eventB == HOLD) {
                stateHolderPower.gotoState(SP_OFF);
                break;
            }
            break;
        }

    }

}

void run_state_basin(McEvent event) {

    if (distance <= 3) {
        stateHolderBasin.gotoState(SB_SENSOR_FAILED);
    }

    switch (stateHolderBasin.state) {
        case SB_DISARM: {
            stateHolderBasin.firstState(SB_DISARM);
            break;
        }
        case SB_FULL: {
            stateHolderBasin.firstState(SB_FULL);
            if (levelCheckOverflow.isTimeAfter(!testOverflow())) {
                levelCheckOverflow.reset();
                stateHolderBasin.gotoState(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_INTAKE: {
            if (stateHolderBasin.firstState(SB_INTAKE)){
                levelCheck.reset();
                levelCheckOverflow.reset();
            }
            if (levelCheck.isTimeAfter(!testIntakeLevel())) {
                levelCheck.reset();
                stateHolderBasin.gotoState(SB_LOW_WATER);
                break;
            }
            if (levelCheckOverflow.isTimeAfter(testOverflow())) {
                levelCheckOverflow.reset();
                stateHolderBasin.gotoState(SB_FULL);
                break;
            }
            break;
        }
        case SB_LOW_WATER: {
            stateHolderBasin.firstState(SB_LOW_WATER);
            if (levelCheck.isTimeAfter(testIntakeLevel())) {
                levelCheck.reset();
                stateHolderBasin.gotoState(SB_INTAKE);
                break;
            }
            break;
        }
        case SB_SENSOR_FAILED: {
            stateHolderBasin.firstState(SB_SENSOR_FAILED);
            if (testIntakeLevel()) {
                stateHolderBasin.gotoState(SB_INTAKE);
            }
            break;
        }
        default:;
    }
}

#ifdef RTCM
void run_state_timer(McEvent eventB) {
    switch (stateHolderTimer.state) {

        case ST_SHUT_DOWN: {
            if (stateHolderTimer.firstState(ST_SHUT_DOWN)) {
                strncpy(CUSTOM_HEADER, " T ", 3);
                CTX.refreshState = true;
                timer.isTimeAfter(false);
            }
            if (eventB == DOUBLE_CLICK) {
                stateHolderTimer.gotoState(ST_ENGAGE);
            }
            break;
        }

        case ST_ENGAGE: {
            if (stateHolderTimer.firstState(ST_ENGAGE)) {
                strncpy(CUSTOM_HEADER, ">T<", 3);
                CTX.refreshState = true;
            }
            if (timer.isTimeAfter(true) || eventB == CLICK) {
                stateHolderTimer.gotoState(ST_SHUT_DOWN);
            }
            break;
        }

        case ST_DISARM: {
            if (stateHolderTimer.firstState(ST_DISARM)) {
                strncpy(CUSTOM_HEADER, "   ", 3);
                CTX.refreshState = true;
            }
            break;
        }

    }
}
#endif

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
                strncpy(CUSTOM_HEADER, "   ", 3);
            }
            if (event == DOUBLE_CLICK) {
                stateHolderInfo.gotoState(SI_DISARM);
                break;
            }
            if (
                    stateHolderBasin.state == SB_SENSOR_FAILED
                    ) {
                stateHolderInfo.gotoState(SI_ALARM);
                break;
            }
            if (
                    stateHolderBasin.state != SB_LOW_WATER
                    ) {
                stateHolderInfo.gotoState(SI_DISARM);
            }
            break;
        }
        case SI_DISARM: {
            if (stateHolderInfo.firstState(SI_DISARM)) {
                strncpy(CUSTOM_HEADER, "   ", 3);
                CTX.refreshState = true;
            }
            if (
                    stateHolderPower.state != SP_OFF && (
                            stateHolderBasin.state == SB_SENSOR_FAILED ||
                            stateHolderBasin.state == SB_LOW_WATER
                    )
                    ) {
                stateHolderInfo.gotoState(SI_WARNING);
                break;
            }
            break;
        }
        default:;
    }
}

void runPowerStates() {
#ifndef NO_CONTROLLER
    McEvent eventA = CTRL.BUTTONS[BUTT_POWER_AUTO];
    McEvent eventB = CTRL.BUTTONS[BUTT_POWER_MAN];
#else
    McEvent eventA = NOTHING;
    McEvent eventB = NOTHING;
#endif
    run_state_basin(eventA);
    run_state_info(eventA);
#ifdef RTCM
    run_state_timer(eventB);
#endif
    run_state_power(eventA, eventB);
}

void setup() {
    PWR.begin();
    ULTRASONIC.begin(ULTRASONIC_SEN);
}

void loop() {

    PWR.run();

    bool warning = stateHolderInfo.state == SI_WARNING;
    bool alarm = stateHolderInfo.state == SI_ALARM;

    bool power = stateHolderPower.state == SP_FILL_IN && (stateHolderBasin.state == SB_INTAKE || stateHolderBasin.state == SB_LOW_WATER) || (stateHolderPower.state == SP_AUTO && stateHolderBasin.state == SB_LOW_WATER);
    bool power_man = stateHolderPower.state == SP_MAN_ON;
    bool power_timer = stateHolderTimer.state == ST_ENGAGE && stateHolderBasin.state != SB_FULL && stateHolderInfo.state != SI_ALARM;
    PWR.power(REL_A, power || power_man || power_timer);

    INDICATORS.set(IND_WARN, warning && flash_symm(timerCounter_1Hz));
    INDICATORS.set(IND_ALARM, alarm && flash_symm(timerCounter_4Hz));

    INDICATORS.set(IND_POWER_MAN, power_timer ? !flash_accent(timerCounter_1Hz) : power_man);

    if (power && (alarm || warning)) {
        INDICATORS.flash(IND_POWER, alarm ? flash_symm(timerCounter_4Hz) : flash_symm(timerCounter_1Hz), true);
    } else {
        INDICATORS.set(IND_POWER, power);
    }
}

#include <avr/wdt.h>
#include "Poweroid10.h"

volatile uint8_t timerCounter = 0;
uint8_t pastTimerCounter = 0;

uint8_t timerFlags;
uint8_t timerCounter_1Hz;
uint8_t timerCounter_4Hz;

void initTimer1() {
    cli();

    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = CONST_4_HZ_TIMER;

    TCCR1B |= (1 << WGM12);                 // turn on CTC mode. clear timer on compare match
    TCCR1B |= TIMER_PRESCALER;              // pre-scaler
    TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt

    sei();

}

void setTimerFlags(){
    if (timerCounter != pastTimerCounter){
        for(uint8_t i = 0; i < TIMERS_COUNT; i++){
            timerFlags = timerCounter % (1U << i ) == 0 ? timerFlags | 1U << i : timerFlags;
        }
        pastTimerCounter = timerCounter;
    }
}

void setFlashCounters(){
    if (test_timer(TIMER_1HZ)){
        timerCounter_1Hz++;
        timerCounter_1Hz = timerCounter_1Hz >= TIMERS_FLASH_COUNTS ? 0 : timerCounter_1Hz;
    }

    if (test_timer(TIMER_4HZ)){
        timerCounter_4Hz++;
        timerCounter_4Hz = timerCounter_4Hz >= TIMERS_FLASH_COUNTS ? 0 : timerCounter_4Hz;
    }

}

Pwr::Pwr(Context &ctx, Commander *_cmd, Controller *_ctrl, Bt *_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {
#ifdef WATCH_DOG
    wdt_disable();
#endif

    Serial.begin(DEFAULT_BAUD);

    CTX->PERS.begin();

    if (BT) {
        BT->begin();
        CTX->passive = !BT->host;
        CTX->remoteMode = BT->remote_on;
    }

    initPins();

#ifdef SSERIAL
    SSerial.begin(DEFAULT_BAUD);
    SSerial.println("SSerial-started");
#endif

    printVersion();

#ifdef WATCH_DOG
    wdt_enable(WDTO_8S);
#endif

    SENS->initSensors(!CTX->passive);

    loadDisarmedStates();

    REL->mapped = !CTX->passive;

#ifdef DEBUG
    writeLog('I', "PWR", 200 + CTX->passive, (unsigned long)0);
#endif

    #ifdef WATCH_DOG
    wdt_enable(WDTO_2S);
#endif

#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->begin();
    }
#endif

    Serial.setTimeout(SERIAL_READ_TIMEOUT);

    initTimer1();
}

void Pwr::run() {

    setTimerFlags();
    setFlashCounters();
    applyTimings();

#ifdef WATCH_DOG
    wdt_reset();
#endif

    bool newConnected = false;
    bool updateConnected = false;

    SENS->process();

    processSensors();

    if (CMD) {
        CMD->listen();
    }

    if (BT && CTX->remoteMode) {
        newConnected = CMD->isConnected();
        updateConnected = newConnected != CTX->connected;
        CTX->refreshState = CTX->refreshState || updateConnected;
        CTX->connected = newConnected;
    }

    if (CTX->canAccessLocally() && test_timer(TIMER_1HZ)){
        fillOutput();
    }

#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->process();
    }
#endif

    if (updateConnected && newConnected && CTX->canInteract()) {
        REL->castMappedRelays();
    }
    if (firstRun) {
#ifdef DEBUG
        writeLog('I', SIGNATURE, 100 + CTX->passive, (unsigned long)0);
#endif
        firstRun = false;
    }

#ifndef CONTROLLER_ONLY
    runPowerStates();
#endif

    printChangedStates();

    timerFlags = 0;
}

void Pwr::printVersion() {
    if (CTX->canInteract()){
        Serial.println(CTX->version);
    }
}

void Pwr::initPins() {

#ifndef SPI
    pinMode(LED_PIN, OUTPUT);
#endif

    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
    REL->reset();

    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }

    INDICATORS.init();
}

void Pwr::loadDisarmedStates() {
    for (uint8_t i = 0; i < state_count; i++) {
        bool disarm = CTX->PERS.loadState(i);
        disarmState(i, disarm);
#ifdef DEBUG
        if (disarm) {
            Serial.println(printState(i));
        }
#endif
    }
}

void Pwr::power(uint8_t i, bool power) {
    if (CTX->canAccessLocally()) {
        CTX->refreshState = power != REL->isPowered(i);
        REL->power(i, power);
    }
}

void Pwr::printChangedStates() {
    for(uint8_t i = 0; i < state_count; i++){
        if (changedState[i]) {
            CMD->printState(i);
            changedState[i] = false;
        }
    }
}

ISR(TIMER1_COMPA_vect) {
    timerCounter++;
    timerCounter = timerCounter > TIMER_COUNTER_MAX ? 0 : timerCounter;
}

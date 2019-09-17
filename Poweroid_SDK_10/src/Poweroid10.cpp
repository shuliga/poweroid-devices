#include <avr/wdt.h>
#include <DS1307/DS1307.h>
#include "Poweroid10.h"

volatile uint16_t oneHzCounter = 0;
uint16_t currentCounter = 0;
bool oneHzFlag = false;

void initOneHzTimer() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = HZ_TIMER_CONST;

    TCCR1B |= (1 << WGM12);                 // turn on CTC mode. clear timer on compare match
    TCCR1B |= (1 << CS12) | (1 << CS10);    // 1024 pre-scaler
    TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt

#ifdef DEBUG
    TCCR1B &= ~((1 << CS12) | (1 << CS10));
    TIMSK1 &= ~(1 << OCIE1A); // disable timer overflow interrupt
#endif

    sei();

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
        CTX->remote = BT->remote_on;
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

}

void Pwr::run() {

    oneHzFlag = oneHzCounter != currentCounter;
    currentCounter = oneHzFlag ? oneHzCounter : currentCounter;

    applyTimings();

#ifdef WATCH_DOG
    wdt_reset();
#endif
    initOneHzTimer();

    bool newConnected = false;
    bool updateConnected = false;

    SENS->process();

    processSensors();

    if (CMD) {
        CMD->listen();
    }

    if (BT && CTX->remote) {
        newConnected = CMD->isConnected();
        updateConnected = newConnected != CTX->connected;
        CTX->refreshState = CTX->refreshState || updateConnected;
        CTX->connected = newConnected;
    }

    if (CTX->canAccessLocally() && oneHzFlag){
        fillOutput();
    }

#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->process();
    }
#endif

    if (updateConnected && newConnected) {
        REL->castMappedRelays();
    }
    if (firstRun) {
#ifdef DEBUG
        writeLog('I', SIGNATURE, 100 + CTX->passive, (unsigned long)0);
#endif
        firstRun = false;
    }

    runPowerStates();

    printChangedStates();

    oneHzFlag = false;
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
    oneHzCounter ++;
}

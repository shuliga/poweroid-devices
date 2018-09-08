#include "global.h"
#include <avr/wdt.h>
#include "Poweroid10.h"

volatile uint8_t semaphor = 0;

Pwr::Pwr(Context &ctx, Commands *_cmd, Controller *_ctrl, Bt *_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {
#ifdef WATCH_DOG
    wdt_disable();
#endif
#ifdef DEBUG
    cli();
    TCCR1B &= ~((1 << CS12) | (1 << CS10));
    TIMSK1 &= ~(1 << OCIE1A); // disable timer overflow interrupt
    sei();
#endif
    Serial.begin(DEFAULT_BAUD);
#ifdef SSERIAL
    SSerial.begin(DEFAULT_BAUD);
    SSerial.println("SSerial-started");
#endif
    printVersion();

    init_pins();

    CTX->PERS.begin();

    #ifdef WATCH_DOG
    wdt_enable(WDTO_8S);
#endif
    if (BT){
        BT->begin();
        CTX->passive = !BT->server;
    }

    SENS->initSensors(!CTX->passive);

    loadDisarmedStates();

    REL->mapped = !CTX->passive;
    writeLog('I', "PWR", 200 + CTX->passive);
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
#ifdef WATCH_DOG
    wdt_reset();
#endif
#ifdef DEBUG
    initTimer();
#endif
    bool newConnected = false;
    bool updateConnected = false;

    semaphor = 1;
    SENS->process();

    semaphor = 2;
    if (CMD) {
        CMD->listen();
    }

    semaphor = 3;
    if (BT){
        newConnected = CMD->isConnected();
        updateConnected = newConnected != CTX->connected;
        CTX->refreshState = CTX->refreshState || updateConnected;
        CTX->connected = newConnected;
    }

    semaphor = 4;
#ifndef NO_CONTROLLER
    if (CTRL) {
        CTRL->process();
    }
#endif

    semaphor = 5;
    if (updateConnected && newConnected) {
        REL->castMappedRelays();
    }
    if (firstRun) {
        writeLog('I', SIGNATURE, 100 + CTX->passive);
        firstRun = false;
    }
#ifdef DEBUG
    cli();
    TCCR1B &= ~((1 << CS12) | (1 << CS10));
    TIMSK1 &= ~(1 << OCIE1A); // disable timer overflow interrupt
    sei();
#endif
}

void Pwr::printVersion() {
    Serial.println(CTX->version);
}

void Pwr::init_pins() {
    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
    REL->reset();

    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
}

void Pwr::loadDisarmedStates() {
    for (uint8_t i = 0; i < state_count; i++) {
        bool disarm = CTX->PERS.loadState(i);
        disarmState(i, disarm);
        if (disarm) {
            Serial.println(printState(i));
        }
    }
}

void Pwr::power(uint8_t i, bool power) {
    if (!CTX->passive) {
        REL->power(i, power);
        CTX->refreshState = true;
    }
}

#ifdef DEBUG
void initTimer() {
// initialize Timer1
    cli();
    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = 15624; // 65536 - 15624; // 16000000L / 1024 / OVERTIME - 1; // set timer value 16MHz/1024/1Hz-1

    TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match
    TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    sei();
}

ISR(TIMER1_COMPA_vect) {
    writeLog('E', "OVERTIME", semaphor);
}
#endif


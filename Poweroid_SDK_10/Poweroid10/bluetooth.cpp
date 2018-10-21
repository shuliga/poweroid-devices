#include <avr/wdt.h>
#include "bluetooth.h"
#include "commons.h"

/*
 * Log codes
 *
 * 100 Command echo
 * 101 Command echo, passive mode
 * 200 Server mode
 * 210 Client mode (speed)
 * 211 Connected to server
 * 410 Disconnected
*/
static const char *ORIGIN = "BT";

Bt::Bt(const char *id) {
    name = id;
}

void Bt::begin() {

    delay(2500);

    server = true;
    Serial.begin(HC_06_BAUD);
    String ver = getVerHC06();

    if (ver.startsWith(BT_VER_06)) {
        Serial.print(F("AT+NAME"));
        Serial.print(name);
    } else {
        if (!checkPeerType(MODE_CLIENT)) {
            server = false;
            Serial.end();
            Serial.begin(HC_05_AT_BAUD_FAST);

            writeLog('I', ORIGIN, 210, HC_05_AT_BAUD_FAST);

            Serial.println("AT");
            delay(200);
            cleanSerial();

            ver = execBtAtCommand(F("AT+VERSION"));
#ifdef WATCH_DOG
            wdt_reset();
#endif
            if (ver.startsWith(BT_VER_05)) {
                applyBt05();
            } else {
                if (checkPeerType(MODE_SERVER)) {
                    writeLog('I', ORIGIN, 211, (unsigned long)0);
                }
            }
        }
    }
}

void Bt::cleanSerial() const {
    while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

}

String Bt::getVerHC06() const {
    Serial.print(F("AT+VERSION"));
    Serial.flush();
    return Serial.readString();
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd) {
    return execBtAtCommand(cmd, 0, 0);
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout) {
#ifdef SSERIAL
    SSerial.print("CMD->");
    SSerial.println(cmd);
#endif
    String s;
    Serial.print(cmd);
    if (cmd2 != 0) {
        Serial.print(cmd2);
    }
    Serial.println();

    long active_timeout = (timeout > 0) ? timeout : Serial.getTimeout();
    uint16_t _delay = 0;
    while (!Serial.available() && active_timeout > _delay) {
        _delay += TIMEOUT_STEP;
        delay(TIMEOUT_STEP);
    }
    while ((Serial.available() || active_timeout > _delay) && !s.endsWith("OK\r\n") && !s.endsWith("ERROR")) {
        char c = (char) Serial.read();
        _delay += 5;
        delay(5);
        if (c < 0) continue;
//        c >= ' ' && c <= 'z' || c == '\r' || c == '\n' ? s += c : s += ' ';
        s += c;
    }
#if defined(SSERIAL) and defined(DEBUG)
    SSerial.print(s);
#endif
    return s;
}

void Bt::applyBt05() {
#ifdef WATCH_DOG
        wdt_reset();
#endif
    execBtAtCommand(F("AT+ORGL"));
    execBtAtCommand(F("AT+NAME="), name, 200);
    execBtAtCommand(F("AT+RMAAD"));
    execBtAtCommand(F("AT+ROLE=1")); // ser role to master
    execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output low level indicates successful connection
//    execBtAtCommand(F(HC_05_AT_BAUD_AT)); // Set baud rate
    execReset();
    execBtAtCommand(F("AT+CMODE=0")); // connect only to predefined address
    execBtAtCommand(F("AT+CLASS=1F00")); // iquire only devices with type 1F00
    execBtAtCommand(F("AT+INQM=1,2,3")); // inquire rssi mode, 2 items, timeout 1.28 * 3 sec

    String devices = execBtAtCommand(F("AT+INQ"), 0, 4000); // Inquire devices, until OK

    int dp1 = devices.indexOf('\r');
    if (dp1 < 0)
        return;

    String device = devices.substring(0, (unsigned int) dp1);
#ifdef SSERIAL
    SSerial.println();
    SSerial.print("Device found: ");
    SSerial.println(device);
#endif
    if (device.startsWith("+INQ:")) {
#ifdef WATCH_DOG
        wdt_reset();
#endif
        String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), (unsigned int) device.indexOf(','));
        peer_address.replace(':', ',');

        execBtAtCommand(F("AT+PSWD="), PASSWD, 0);
        execBtAtCommand(F("AT+BIND="), peer_address.c_str(), 0);
        execBtAtCommand(F("AT+POLAR=1,1")); // PIN09 output HIGH level indicates successful connection
        execReset();
        execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output LOW level indicates successful connection
    }

}

void Bt::execReset() {
    execBtAtCommand(F("AT+RESET"));
    delay(1000);
    execBtAtCommand(F("AT+INIT"));
}

bool Bt::checkPeerType(const char *conn_type) {
    delay(500);
    cleanSerial();
    Serial.println(F("mode ask"));
    Serial.flush();
    return Serial.readStringUntil('\n').indexOf(conn_type) >= 0;
}

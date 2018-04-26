#include "bluetooth.h"

/*
 * Log codes
 *
 * 100 Command echo
 * 200 Server mode
 * 210 Client mode (speed)
 * 211 Connected to server
 * 410 Disconnected
*/
static TimingState connection_check(CONNECTION_CHECK);

const char *ORIGIN = "BT";

Bt::Bt(const char *id) {
    name = id;
}

void Bt::begin() {
    delay(1000);
    String ver = getVerHC06();
    server = true;
    if (ver.startsWith(BT_VER_06)) {
        Serial.print(F("AT+BAUD8"));
    }
    cleanSerial();
    Serial.end();
    delay(1000);
    Serial.begin(HC_06_BAUD);
    ver = getVerHC06();
    if (ver.startsWith(BT_VER_06)) {
        Serial.print(F("AT+NAME"));
        Serial.print(name);
    } else {
        if (!checkPeerType(ASK_CLIENT)) {
            server = false;
            Serial.end();
            delay(3000);
            Serial.begin(HC_05_AT_BAUD);

            writeLog('I', ORIGIN, 210, HC_05_AT_BAUD);

            Serial.println("AT");
            delay(200);
            cleanSerial();

            ver = execBtAtCommand(F("AT+VERSION"));

            if (ver.startsWith(BT_VER_05)) {
                applyBt05();
                connected = true;
            } else {
                if (checkPeerType(ASK_SERVER)) {
                    writeLog('I', ORIGIN, 211);
                    connected = true;
                }
            }
        }
    }

    firstRun = false;
    if (server) {
        writeLog('I', ORIGIN, 200);
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, HIGH);
        Serial.setTimeout(100);
    }

}

void Bt::cleanSerial() const {
    while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

}

String Bt::getVerHC06() const {
    Serial.print(F("AT+VERSION"));
    String ver = Serial.readString();
    return ver;
}

bool Bt::isConnected() {
    if (firstRun || connection_check.isTimeAfter(true)) {
        connected = server ? checkPeerType(ASK_CLIENT) : checkPeerType(ASK_SERVER);
        connection_check.reset();
#ifdef DEBUG
        if (!server && !connected) {
            writeLog('W', ORIGIN, 410);
        }
#endif
    }
    return connected;
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
        c >= ' ' && c <= 'z' || c == '\r' || c == '\n' ? s += c : s += ' ';
        _delay += 20;
        delay(20);
    }
#if defined(SSERIAL) and defined(DEBUG)
    SSerial.print(s);
#endif
    return s;
}

bool Bt::getConnected() {
    return isConnected();
}

void Bt::applyBt05() {
    execBtAtCommand(F("AT+ORGL"));
    execBtAtCommand(F("AT+NAME="), name, 200);
    execBtAtCommand(F("AT+RMAAD"));
    execBtAtCommand(F("AT+ROLE=1")); // ser role to master
    execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output low level indicates successful connection
    execReset();
    execBtAtCommand(F("AT+CMODE=0")); // connect only to predefined address
    execBtAtCommand(F("AT+CLASS=1F00")); // iquire only devices with type 1F00
    execBtAtCommand(F("AT+INQM=1,2,4")); // inquire rssi mode, 2 items, timeout 5 sec

    String devices = execBtAtCommand(F("AT+INQ"), 0, 5100); // Inquire devices, until OK

    int dp0 = 0;
    int dp1 = devices.indexOf('\r');
    if (dp1 < 0)
        return;

    String device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

    while (device.startsWith("+INQ:")) {
        uint8_t dio_c = (unsigned int) device.indexOf(',');
        String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), dio_c);
        peer_address.replace(':', ',');

        dp0 = devices.indexOf('+', (unsigned int) dp1);
        dp1 = devices.indexOf('\n', (unsigned int) dp0);
        device = devices.substring((unsigned int) dp0, (unsigned int) dp1);
//
//        String peer_name = execBtAtCommand(F("AT+RNAME?"), peer_address.c_str(),
//                                           5000);// get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6
//
//        if (peer_name.startsWith("+RNAME:PWR-")) {
        execBtAtCommand(F("AT+PSWD="), PASSWD, 0);
        execBtAtCommand(F("AT+BIND="), peer_address.c_str(), 0);
        execBtAtCommand(F("AT+POLAR=1,1")); // PIN09 output HIGH level indicates successful connection
        execReset();
        execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output LOW level indicates successful connection
        break;
//        }
    }

}

void Bt::execReset() {
    execBtAtCommand(F("AT+RESET"));
    delay(1000);
    execBtAtCommand(F("AT+INIT"));
}

bool Bt::checkPeerType(const char *conn_type) {
    Serial.println(F("ask"));
    return Serial.readStringUntil('\n').indexOf(conn_type) >= 0;
}

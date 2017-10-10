#include "bluetooth.h"


Bt::Bt(const char *id) {
    name = id;
}

void Bt::begin() {
#ifdef SSERIAL
    SSerial.begin(9600);
#endif
    if (!on) {
        delay(500);
        Serial.print(F("AT+VERSION"));
        String ver = Serial.readString();
        if (ver.startsWith(BT_VER_06)) {
            Serial.print(F("AT+NAME"));
            Serial.print(name);
            delay(500);
            on = true;
        } else {
            Serial.end();
            Serial.begin(38400);
            passive = true;
            if (!isConnected()) {
                Serial.println("AT");
                delay(200);
                while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

                ver = execBtAtCommand(F("AT+VERSION"));

                if (ver.startsWith(BT_VER_05)) {
                    applyBt05();
                    on = true;
                }
// Keep working on 38400 for HC-05
//                Serial.end();
//                Serial.begin(9600);
                Serial.println();
            }
        }
    }
}

bool Bt::isConnected() {
    connected = execBtAtCommand(F("get_ver"), 0, 500).startsWith("get_ver");
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
        _delay += 100;
        delay(100);
    }
    while ((Serial.available() || active_timeout > _delay) && !s.endsWith("OK\r\n") && !s.endsWith("ERROR")) {
        char c = (char) Serial.read();
        c >= ' ' && c <= 'z' || c == '\r' || c == '\n' ? s += c : s += ' ';
        _delay += 20;
        delay(20);
    }
#ifdef SSERIAL
    SSerial.println(s);
#endif
    return s.substring(0, (unsigned int) s.lastIndexOf('\r'));
}

bool Bt::getPassive() {
    return passive;
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
    execBtAtCommand(F("AT+INQM=1,3,4")); // inquire rssi mode, 2 items, timeout 13 sec

    String devices = execBtAtCommand(F("AT+INQ"), 0, 5100); // Inquire devices, until OK

    long dp0 = 0;
    long dp1 = (unsigned int) devices.indexOf('\r');
    String device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

    while (device.startsWith("+INQ:")) {
        uint8_t dio_c = (unsigned int) device.indexOf(',');
        String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), dio_c);
        peer_address.replace(':', ',');

        dp0 = devices.indexOf('+', (unsigned int) dp1);
        dp1 = devices.indexOf('\r', (unsigned int) dp0);
        device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

        String peer_name = execBtAtCommand(F("AT+RNAME?"), peer_address.c_str(),
                                           5000);// get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6

        if (peer_name.startsWith("+RNAME:PWR-")) {
            execBtAtCommand(F("AT+PSWD="), PASSWD, 0);
            execBtAtCommand(F("AT+BIND="), peer_address.c_str(), 0);
            execBtAtCommand(F("AT+POLAR=1,1")); // PIN09 output HIGH level indicates successful connection
            execReset();
            execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output LOW level indicates successful connection
            connected = true;
            passive = true;
            break;
        }
    }

}

void Bt::execReset() {
    execBtAtCommand(F("AT+RESET"));
    delay(1000);
    execBtAtCommand(F("AT+INIT"));
}

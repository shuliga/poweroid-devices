#include "bluetooth.h"

Bt::Bt(const char *id) {
    name = id;
}

void Bt::begin() {
    if (!on) {
        delay(600);
        Serial.print(F("AT+VERSION"));
        String ver = Serial.readString();
        if (ver.startsWith(BT_VER_06)) {
            sprintf(result, "BT-ver: %s", ver.c_str());
            Serial.print(F("AT+NAME"));
            Serial.print(name);
            on = true;
            digitalWrite(10,HIGH);
        } else {
            Serial.end();
            Serial.begin(38400);

            delay(400);

            Serial.println("AT");
            delay(400);
            while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

            ver = execBtAtCommand(F("AT+VERSION"));

            if (ver.startsWith(BT_VER_05)) {
                sprintf(result, "BT-ver: %s", ver.c_str());
                applyBt05();
                on = true;
            } else {
                sprintf(result, "BT-no_ver");
            }
            Serial.end();
            Serial.begin(9600);
            Serial.println();
        }
    }
}

void Bt::getResult() {
    Serial.println(result);
}

bool Bt::getConnected() {
    return connected;
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd) {
    return execBtAtCommand(cmd, 0, 0);
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout) {
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
        s += (char) Serial.read();
        _delay += 20;
        delay(20);
    }

    return s.substring(0, (unsigned int) s.lastIndexOf('\r'));
}

bool Bt::getPassive() {
    return passive;
}

void Bt::applyBt05() {
    passive = true;
    execBtAtCommand(F("AT+ORGL"));
    execBtAtCommand(F("AT+RMAAD"));
    execBtAtCommand(F("AT+ROLE=1")); // ser role to master
    execBtAtCommand(F("AT+RESET"));
    delay(500);
    execBtAtCommand(F("AT+INIT"));
    execBtAtCommand(F("AT+CMODE=0")); // connect only to predefined address
    execBtAtCommand(F("AT+INQM=1,5,3")); // inquire rssi mode, 5 items, timeout 13 sec

    String devices = execBtAtCommand(F("AT+INQ"), 0, 4000); // Inquire devices, until OK

    long dp0 = 0;
    long dp1 = (unsigned int) devices.indexOf('\r');
    String device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

    while (device.startsWith("+INQ:")) {
        uint8_t dio_c = (unsigned int) device.indexOf(',');
        String peer_type = device.substring(dio_c + 1, (unsigned int) device.lastIndexOf(','));
        String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), dio_c);
        peer_address.replace(':', ',');

        dp0 = devices.indexOf('+', (unsigned int) dp1);
        dp1 = devices.indexOf('\r', (unsigned int) dp0);
        device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

        if (!peer_type.startsWith("1F00")) // Expected HC-06 device type is 1F00
            continue;

        String peer_name = execBtAtCommand(F("AT+RNAME?"), peer_address.c_str(),
                                           3000);// get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6

        if (peer_name.startsWith("+RNAME:PWR-")) {
            execBtAtCommand(F("AT+PSWD="), PASSWD, 0);
            execBtAtCommand(F("AT+BIND="), peer_address.c_str(), 0);
            delay(200);
            execBtAtCommand(F("AT+POLAR=1,0"));
            connected = true;
            sprintf(result, "BT-passive");
            break;
        }
    }
    if (!connected) {
        execBtAtCommand(F("AT+INQC"));
    }

}

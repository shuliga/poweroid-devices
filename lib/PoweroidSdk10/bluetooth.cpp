#include "bluetooth.h"

Bt::Bt(const char *id) {
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);

    if (!on) {
        name = id;
        delay(600);
        Serial.print(F("AT+VERSION"));
        delay(600);
        uint8_t cnt = (uint8_t) Serial.available();
        char *ver = new char[cnt + 1]();
        ver[cnt] = 0;
        Serial.readBytes(ver, cnt);
        if ((cnt == 12 && strncmp(ver, BT_VER_06, 12) == 0)) {
            sprintf(result, "BT-ver: %s", ver);
            Serial.print(F("AT+NAME"));
            Serial.print(name);
            on = true;
        } else {
            Serial.end();
            Serial.begin(38400);

            delay(400);

            Serial.println("AT");
            delay(400);
            while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

            Serial.println(F("AT+VERSION"));
            delay(400);
            cnt = (uint8_t) Serial.available();
            delete ver;
            ver = new char[cnt + 1]();
            ver[cnt] = 0;
            Serial.readBytes(ver, cnt);

            if ((cnt >= 21 && strncmp(ver, BT_VER_05, 21) == 0)) {
                passive = true;
                sprintf(result, "BT-ver: %s", ver);
                execBtAtCommand(F("AT+ORGL"),0,0);
                execBtAtCommand(F("AT+RMAAD"),0,0);
                execBtAtCommand(F("AT+ROLE=1"),0,0); // ser role to master
                execBtAtCommand(F("AT+RESET"),0,0);
                delay(500);
                execBtAtCommand(F("AT+INIT"),0,0);
                execBtAtCommand(F("AT+CMODE=0"),0,0); // connect only to predefined address
                execBtAtCommand(F("AT+INQM=1,3,3"),0,0); // inquire rssi mode, 5 items, timeout 13 sec

                String devices = execBtAtCommand(F("AT+INQ"), 0, 4000); // Inquire devices, until OK

                long dp0 = 0;
                long dp1 = (unsigned int) devices.indexOf('\r');
                String device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

                while(device.startsWith("+INQ:")){
                    uint8_t dio_c = (unsigned int) device.indexOf(',');
                    String peer_type = device.substring( dio_c + 1,(unsigned int) device.lastIndexOf(','));
                    String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), dio_c);
                    peer_address.replace(':', ',');

                    dp0 = devices.indexOf('+', (unsigned int) dp1);
                    dp1 = devices.indexOf('\r', (unsigned int) dp0);
                    device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

                    if (!peer_type.startsWith("1F00"))
                        continue;

                    String peer_name = execBtAtCommand(F("AT+RNAME?"), peer_address.c_str(),
                                                       3000);// get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6

                    if (peer_name.startsWith("+RNAME:PWR-")) {
                        execBtAtCommand(F("AT+PSWD="), PASSWD,0);
                        execBtAtCommand(F("AT+BIND="), peer_address.c_str(),0);
                        delay(200);
                        execBtAtCommand(F("AT+POLAR=1,0"),0,0);
                        connected = true;
                        sprintf(result, "BT-passive");
                        break;
                    }
                }
                if (!connected){
                    execBtAtCommand(F("AT+INQC"),0,0);
                }
                Serial.end();
                Serial.begin(9600);
                on = true;
            } else {
                sprintf(result, "BT-no_ver");
            }
        }
        delete ver;
    }
}

void Bt::getResult() {
    Serial.println(result);
}

bool Bt::getConnected() {
    return connected;
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout) {
    String s;
    Serial.print(cmd);
    if (cmd2 != 0) {
        Serial.print(cmd2);
    }
    Serial.println();

    if (timeout > 0) {
        uint16_t _delay = 0;
        while (!Serial.available() && timeout > _delay) {
            _delay += 100;
            delay(100);
        }
        while ((Serial.available() || timeout > _delay) && !s.endsWith("OK\r\n") && !s.endsWith("ERROR")) {
            s += (char) Serial.read();
            _delay += 20;
            delay(20);
        }
    } else {
        uint16_t _delay = 0;
        while (!Serial.available() && Serial.getTimeout() > _delay) {
            _delay += 100;
            delay(100);
        }
        while (Serial.available()) {
            s += (char) Serial.read();
            delay(25);
        }
    }

    String res = s.substring(0, (unsigned int) s.lastIndexOf('\r'));
    return res;
}

bool Bt::getPassive() {
    return passive;
}

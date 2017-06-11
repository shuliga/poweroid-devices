#include <ACROBOTIC_SSD1306.h>
#include "bluetooth.h"

//static long getRate(uint8_t recpin)  // function to return valid received baud rate
//// Note that the serial monitor has no 600 baud option and 300 baud
//// doesn't seem to work with version 22 hardware serial library
//{
//    long baud, rate = 10000, x;
//    for (int i = 0; i < 10; i++) {
//        x = pulseIn(recpin,LOW);   // measure the next zero bit width
//        rate = x < rate ? x : rate;
//    }
//
//    if (rate < 12)
//        baud = 115200;
//    else if (rate < 20)
//        baud = 57600;
//    else if (rate < 29)
//        baud = 38400;
//    else if (rate < 40)
//        baud = 28800;
//    else if (rate < 60)
//        baud = 19200;
//    else if (rate < 80)
//        baud = 14400;
//    else if (rate < 150)
//        baud = 9600;
//    else if (rate < 300)
//        baud = 4800;
//    else if (rate < 600)
//        baud = 2400;
//    else if (rate < 1200)
//        baud = 1200;
//    else
//        baud = 0;
//    return baud;
//}

Bt::Bt(const char *id) {
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);

    if (oled.checkAndInit(1)) {
        oled.clearDisplay();
        oled.setBrightness(0);
        oled.setTextXY(0, 0);
    };


    if (!on) {
        name = id;
        delay(600);
        Serial.print(F("AT+VERSION"));
        delay(600);
        uint8_t cnt = (uint8_t) Serial.available();
        char *ver = new char[cnt + 1]();
        ver[cnt] = 0;
        Serial.readBytes(ver, cnt);
        oled.putNumber(cnt);
        oled.putString(":");
        oled.putString(ver);
        oled.putString("//");
        if ((cnt == 12 && strncmp(ver, BT_VER_06, 12) == 0)) {
            sprintf(result, "BT version: %s", ver);
            Serial.print(F("AT+NAME"));
            Serial.print(name);
            on = true;
            sprintf(result, "BT name set to: %s", name);
            delay(600);
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
                sprintf(result, "BT version: %s", ver);
//                oled.setTextXY(1, 0);
                execBtAtCommand(F("AT+ORGL"));
                execBtAtCommand(F("AT+RMAAD"));
//                delay(300);
                execBtAtCommand(F("AT+ROLE=1")); // ser role to master
                execBtAtCommand(F("AT+RESET"));
                delay(500);
                execBtAtCommand(F("AT+INIT"));
                execBtAtCommand(F("AT+CMODE=0")); // connect only to predefined address
                execBtAtCommand(F("AT+INQM=1,3,3")); // inquire rssi mode, 5 items, timeout 13 sec

                String devices = execBtAtCommand(F("AT+INQ"), 0, 4000); // Inquire devices, until OK

                oled.setTextXY(1, 0);
                long dp0 = 0;
                long dp1 = (unsigned int) devices.indexOf('\r');
                String device = devices.substring((unsigned int) dp0, (unsigned int) dp1);
                while(device.startsWith("+INQ:")){
                    String peer_type = device.substring((unsigned int) device.indexOf(',') + 1,(unsigned int) device.lastIndexOf(','));
                    String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1),
                                                            (unsigned int) device.indexOf(','));
                    peer_address.replace(':', ',');

                    dp0 = devices.indexOf('+', (unsigned int) dp1);
                    dp1 = devices.indexOf('\r', (unsigned int) dp0);
//                    if (dp0 < 0 || dp1 < 0) {
//                        break;
//                    }
                    device = devices.substring((unsigned int) dp0, (unsigned int) dp1);

                    if (!peer_type.startsWith("1F00"))
                        continue;

                    oled.putString(peer_address);
                    oled.putString("_");
                    String peer_name = execBtAtCommand(F("AT+RNAME?"), peer_address.c_str(),
                                                       3000);// get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6

                    if (peer_name.startsWith("+RNAME:PWR-")) {
                        oled.putString(peer_name);
                        execBtAtCommand(F("AT+PSWD="), PASSWD);
                        execBtAtCommand(F("AT+BIND="), peer_address.c_str());
                        delay(200);
                        execBtAtCommand(F("AT+POLAR=1,0"));
                        connected = true;
                        sprintf(result, "BT connected in passive mode");
                        break;
                    }
                }
                if (!connected){
                    execBtAtCommand(F("AT+INQC"));
                }
                Serial.end();
                Serial.begin(9600);
                on = true;
            } else {
                sprintf(result, "No BT version data.");
            }
        }
        delete ver;
    }
    digitalWrite(10, connected ? HIGH : LOW);
    digitalWrite(9, passive ? HIGH : LOW);
}

void Bt::getResult() {
    Serial.println(result);
}

bool Bt::getConnected() {
    return connected;
}

bool Bt::isConnected() {
    return execBtAtCommand(F("AT+STATE")).startsWith(F("+STATE:CONNECTED"));
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

String Bt::execBtAtCommand(const __FlashStringHelper *cmd) {
    return execBtAtCommand(cmd, 0, 0);
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2) {
    return execBtAtCommand(cmd, cmd2, 0);
}

void Bt::setPin(char *pin) {
    if (Serial && on) {
        Serial.print(F("AT+PIN"));
        Serial.print(pin);
        delay(600);
    }
}

bool Bt::getPassive() {
    return passive;
}

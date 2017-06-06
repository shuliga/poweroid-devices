#include "bluetooth.h"
#include <ACROBOTIC_SSD1306.h>

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
            digitalWrite(9, HIGH);
            sprintf(result, "BT version: %s", ver);
            Serial.print(F("AT+NAME"));
            Serial.print(name);
            on = true;
            sprintf(result, "BT name set to: %s", name);
            delay(600);
        } else {
            Serial.end();
            Serial.begin(38400);

            delay(600);

            Serial.println("AT");
            delay(600);
            while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

            Serial.println(F("AT+VERSION"));
            delay(600);
            cnt = (uint8_t) Serial.available();
            delete ver;
            ver = new char[cnt + 1]();
            ver[cnt] = 0;
            Serial.readBytes(ver, cnt);
            oled.setTextXY(4, 0);
            oled.putString(ver);

            if ((cnt >= 21 && strncmp(ver, BT_VER_05, 21) == 0)) {
                digitalWrite(10, HIGH);
                char res[] = "12345678902334444444444";
                char peer_name[] = "000000000000000";
                sprintf(result, "BT version: %s", ver);
                on = true;
//                    execBtAtCommand(F("AT+STATE"), res);
//                    if (strncmp(res, "+STATE:CONNECTED", 16) == 0) {
//                        connected = true;
//                        return;
//                    }
//                    execBtAtCommand(F("AT+ORGL"), res);
//                    execBtAtCommand(F("AT+RMAAD"), res);
//                    execBtAtCommand(F("AT+ROLE=1"), res); // ser role to master
//                    execBtAtCommand(F("AT+RESET"), res);
//                    delay(600);
//                    execBtAtCommand(F("AT+INIT"), res);
//                    execBtAtCommand(F("AT+CMODE=0"), res); // connect only to predefined address
//                    execBtAtCommand(F("AT+INQM=1,5,5"), res); // inquire rssi mode, 5 items, timeout 13 sec
//                    execBtAtCommand(F("AT+INQ"), 0, res, 1200); // Inquire devices, until OK
//                    execBtAtCommand(F("AT+RNAME?"), "2016,10,202848",
//                                    res); // get pairing candidate name, sample address +INQ:2016:10:202848,1F00,FFA6
//
//                    if (strncmp(peer_name, "PWR-", 4) == 0) {
//                        execBtAtCommand(F("AT+PSWD=1234"), res);
//                        execBtAtCommand(F("AT+BIND="), "2016,10,202848", res);
//                        connected = true;
//                    }
//                    delay(200);
                Serial.end();
                Serial.begin(9600);
            } else {
                sprintf(result, "No BT version data.");
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

void Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, char *res, uint16_t timeout) {
    Serial.print(cmd);
    if (cmd2 != 0) {
        Serial.print(cmd2);
    }
    Serial.println();
    delay(timeout);
    uint16_t size_t = (uint16_t) Serial.available();
    Serial.readBytes(res, size_t);
    res[size_t] = 0;
}

void Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, char *res) {
    execBtAtCommand(cmd, 0, res, 200);
}

void Bt::execBtAtCommand(const __FlashStringHelper *cmd, char *res) {
    execBtAtCommand(cmd, 0, res);
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


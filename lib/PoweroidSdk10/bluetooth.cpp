#include "bluetooth.h"

Bt::Bt(const char *n): name(n){
    if (!on){
        delay(600);
        Serial.print(F("AT+VERSION"));
        delay(600);
        if (Serial.available() > 0){
            String ver = Serial.readString();
            sprintf(result, "BT version: %s", ver.c_str());
            if (ver.startsWith(BT_VER) ){
                Serial.print(F("AT+NAME"));
                Serial.print(name);
                on = true;
                sprintf(result, "BT name set to: %s", name);
                delay(600);
            }
        } else {
            Serial.println("");
            sprintf(result, "No BT version data.");
        }
    }
}

void Bt::getResult(){
    Serial.println(result);
}

void Bt::setPin(char * pin){
    if (Serial && on){
        Serial.print(F("AT+PIN"));
        Serial.print(pin);
        delay(600);
    }
}

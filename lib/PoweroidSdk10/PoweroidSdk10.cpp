#include "PoweroidSdk10.h"

#include <Arduino.h>

void init_display();

void init_outputs(){
  for(int i=0; i < sizeof(OUT_PINS) / sizeof(OUT_PINS[i]); i++){
    pinMode(OUT_PINS[i],OUTPUT);
  }
}

void init_inputs(){
  for(int i=0; i < sizeof(IN_PINS) / sizeof(IN_PINS[i]); i++){
    pinMode(IN_PINS[i],INPUT_PULLUP);
  }
}

void init_system(){
  flash_333.interval = del;
  init_outputs();
  init_inputs();
}

bool checkInstalled(int pin, bool inst){
  bool sign = readPinLow(pin);
  if (!inst && sign) {
    Serial.print(F("Sensor installed on pin "));
	Serial.println(pin);
  }
  return inst || sign;
}

bool checkInstalledWithDelay(int pin, bool inst, TimingState *hold_on){
  bool sign = isTimeAfter(hold_on, readPinLow(pin));
  if (!inst && sign) {
    Serial.print(F("Sensor installed on pin "));
	Serial.println(pin);
  }
  return inst || sign;
}

void init_sensors(){
  for(int i=0; i < 3; i++){
    hold_on[i].interval = INST_DELAY;
  }
}

void check_installed(){
  for(int i = 0; i < 3; i++){
    installed[i] = checkInstalledWithDelay(IN_PINS[i], installed[i], &hold_on[i]);
  }
}

bool is_sensor_on(int index){
  return readPinLow(IN_PINS[index]) && installed[index];
}

int get_sensor_val(int index){
  if (installed[index]) {
    return readPinVal(INA_PINS[index]);
  } else {
    return -1;
  }
}

bool is_sensor_val(int index, int val){
  return get_sensor_val(index) == val;
}

void indicate(){
  for(int i = 0; i < 3; i++){
    led(LED_PINS[0], installed[i]);
  }
}

void printVersion(){
  Serial.println(F(VERSION));
}

void displayProperty(int i){

}
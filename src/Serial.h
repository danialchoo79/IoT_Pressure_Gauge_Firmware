#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

void ReserveMemoryForSerial(int mem);
void serialEvent();
void HandleSerialCommand(String cmd);
void ServiceSerialCommand();

extern bool stringComplete;
extern String inputString;

#endif
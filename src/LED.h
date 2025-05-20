#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     3
#define NUM_LEDS    16 

void SetupLEDPin();
void TurnONAllColorLED1By1();
void TurnONAllColorLED();
void TurnONAllColorLED1By1()

#endif
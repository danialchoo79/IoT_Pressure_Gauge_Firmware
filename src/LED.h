#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     25
#define NUM_LEDS    16
#define BRIGHTNESS  130

void SetupLEDPin();
void TurnONAllColorLED1By1();
void TurnONAllColorLED();
void TurnOFFAllColorLED1By1();
void TurnOFFAllColorLED();

// Test
void LEDDataTest();

#endif
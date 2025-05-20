#include "LED.h"

Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void SetupLEDPin()
{
    ring.begin();       // Initialize the ring
    ring.show();        // Turn off all LEDs
}

void TurnONAllColorLED1By1()
{
    // Light up LEDs one by one using Color Set By RGB Values
    for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, ring.Color(255, 255, 255)); // White
        ring.show();
        delay(1000);
    }
}

void TurnONAllColorLED()
{
    // Light up All LEDs using Color Set By RGB Values
    for(int i=0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, ring.Color(255, 255, 255)); // White
    }
    ring.show();
}

void TurnOFFAllColorLED1By1()
{
    // Turn Off LEDs one by one using RGB Value of 0,0,0
    for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, ring.Color(255, 255, 255)); // White
        ring.show();
        delay(1000);
    }
}

void TurnOFFAllColorLED()
{
    // Turn OFF LEDs one by one using RGB Value of 0,0,0
    for(int i=0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, ring.Color(0, 0, 0)); // OFF
    }
    ring.show();
}
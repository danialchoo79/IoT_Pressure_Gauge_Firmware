#include "LED.h"

Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void SetupLEDPin()
{
    ring.begin();       // Initialize the ring
    ring.show();        // Turn off all LEDs
}

void TurnAllColorLED()
{
    // Light up LEDs one by one with red color
    for (int i = 0; i < NUM_LEDS; i++) {
        ring.setPixelColor(i, ring.Color(255, 0, 0)); // Red
        ring.show();
        delay(1000);
    }

    delay(1000);
    ring.clear();
    ring.show();
    delay(1000);
}
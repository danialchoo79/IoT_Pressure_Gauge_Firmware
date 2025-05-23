#include "Serial.h"
#include "HTTPCamNonBlocking.h"
#include "LED.h"

String inputString = "";
bool stringComplete = false;

void ReserveMemoryForSerial(int mem)
{
    inputString.reserve(mem); // Reserve "mem" Bytes
}

void serialEvent()
{
    while(Serial.available())
    {
        char inChar = (char)Serial.read();  // Read a Character

        if(inChar == '\n')          
            stringComplete = true;
        else
            inputString += inChar;
    }
}

void HandleSerialCommand(String cmd)
{
    cmd.trim(); // Removes trailing newline / whitespace

    if(cmd == "TakeScreenshot")
    {
        Serial.println("Turning On LED Ring...");
        vTaskDelay(500/portTICK_PERIOD_MS);

        TurnONAllColorLED();
        vTaskDelay(500/portTICK_PERIOD_MS);

        ImageCaptureHTTP();
        vTaskDelay(500/portTICK_PERIOD_MS);

        TurnOFFAllColorLED();
        vTaskDelay(500/portTICK_PERIOD_MS);
        
        Serial.println("Image Taken.");
    }
}

void ServiceSerialCommand()
{
    if(stringComplete)
    {
      HandleSerialCommand(inputString); // Function call
      inputString = "";
      stringComplete = false;
    }
}


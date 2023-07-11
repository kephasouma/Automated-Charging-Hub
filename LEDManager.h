# ifndef LEDManager
# define LEDManager

#include "PinConfig.h"

// TODO: THESE ARE ALL BLOCKING, MUST CHANGE TO MILLIS OR INTERRUPTS

void RGBColour(uint8_t red, uint8_t green, uint8_t blue){
    analogWrite(LEDRed, red);
    analogWrite(LEDGreen, green);
    analogWrite(LEDBlue, blue);
}

void signalKeyPressed(){
    RGBColour(255,255,0);
    delay(50);
    RGBColour(0,0,0);
}

void signalRejectedPassword()
{
    for (int i=0; i<3; i++){
        RGBColour(255,0,0);
        delay(150);
        RGBColour(0,0,0);
        delay(150);
    }
}

void signalEntryAccepted(){
    RGBColour(0,255,0);
    delay(200);
    RGBColour(0,0,0);
}

void signalClearBuffer(){
    RGBColour(255,0,0);
    delay(250);
    RGBColour(0,0,0);
}

void signalStartup(){
  RGBColour(235, 185, 50);
  delay(500);
  RGBColour(0,255,0);
  delay(250);
  RGBColour(0,0,0);
  delay(100);
  RGBColour(0,255,0);
  delay(250);
  RGBColour(0,0,0);
}

# endif

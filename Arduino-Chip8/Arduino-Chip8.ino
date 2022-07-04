//#include <iostream>
//#include <time.h>
//#include <stdlib.h>
#include <Arduino.h>
//define WINDOWS

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h> // The graphics library
#include <Adafruit_SH110X.h>  // The driver for the specific OLED display

#include "Emulator.hpp"

Emulator* emu;

void setup(){
    Serial.begin(115200);
    delay(1000);
    emu = new Emulator("/PONG");
    //emu.PrintMem();
}

void loop(){
    emu->Run();
}

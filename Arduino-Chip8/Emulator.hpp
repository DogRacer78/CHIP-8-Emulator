//#include <stdint.h>
//#include <vector>
#ifndef EMU
#define EMU
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h> // The graphics library
#include <Adafruit_SH110X.h>  // The driver for the specific OLED display

#define TOTAL_REMOTE_KEYS 16
// code goes here

class Emulator{
    public:
        Emulator(const char *fname);
        void Execute();
        void Cycle();
        bool SetPixel(int x, int y);
        void Render();
        void Draw(uint8_t x, uint8_t y);
        void LoadFile();
        void Run();
        void PrintMem();
        void PrintPixels();
        void ReadArduinoFile();
        void GetAllRomNames();

    private:
        int width = 64, height = 32;
        int widthScaleFactor = 2, heightScaleFactor = 2;

        bool pixels[32 * 64];

        //need to do screen stuff

        uint8_t mem[4096]{};
        uint16_t stack[16];
        uint8_t sp = 0x00;
        uint16_t indexReg = 0x0000;
        uint8_t variables[16]{};
        uint16_t pc = 0x200;
        uint16_t opcode;
        const char *rom;

        //keys later

        uint8_t delayTimer = 0x0;
        uint8_t soundTimer = 0x0;

        //screen related
        int fps = 60;
        float interval = 1000.0f / (float)fps;
        unsigned long timer = 0.0;
        unsigned long start = 0.0;
        unsigned long current;

        Adafruit_SH1106G* display;

        //keys
        uint8_t keys[TOTAL_REMOTE_KEYS][3] = { {0x44, 0, 0xF}, {0x7, 0, 0xC}, {0x15, 0, 0xD}, {0x9, 0, 0xE}, {0x16, 0, 0x0}, {0x19, 0, 0xA}, 
        {0xD, 0, 0xB}, {0xC, 0, 0x1}, {0x18, 0, 0x2}, {0x5E, 0, 0x3}, {0x8, 0, 0x4}, {0x1C, 0, 0x5}, {0x5A, 0, 0x6}, {0x42, 0, 0x7}, {0x52, 0, 0x8}, {0x4A, 0, 0x9} };

        const char* romsInMemory[4];


        uint8_t font[80] = 
        {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0, 
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80 
        };



};
#endif

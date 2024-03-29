//#include <stdint.h>
//#include <vector>
#ifndef MyClass_h
#define MyClass_h
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

    private:
        int width = 64, height = 32;
        #ifdef WINDOWS
        int widthScaleFactor = 10, heightScaleFactor = 10;
        #endif

        #ifdef ARDUINO
        int widthScaleFactor = 2, heightScaleFactor = 2;
        #endif

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
        float interval = 1.0f / (float)fps;
        float timer = 0.0f;

        #ifdef ARDUINO
        Adafruit_SH1106G display = Adafruit_SH1106G(width * widthScaleFactor, height * heightScaleFactor, &Wire, -1);
        #endif

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
//#include <math.h>
#include "Emulator.hpp"
#include <Fs.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h> // The graphics library
#include <Adafruit_SH110X.h>  // The driver for the specific OLED display

#include <IRremote.hpp>

Emulator::Emulator(const char *fname){
    //placing font in memory

    for (int i = 0; i < 4096; i++){
        mem[i] = 0x00;   
    }

    for (int i = 0; i < 80; i++){
        mem[i] = font[i];
    }

    for (int i = 0; i < 32 * 64; i++){
        pixels[i] = false;
    }

    for (int i = 0; i < 16; i++){
        variables[i] = 0x0;
    }

    display = new Adafruit_SH1106G((uint16_t)128, (uint16_t)64, &Wire, -1);

    IrReceiver.begin(2, true);
    Serial.print("Ready to receive IR signals of protocols: ");
    printActiveIRProtocols(&Serial);
    Serial.print("at pin ");
    Serial.print(2);
    Serial.println();

    rom = fname;
    Serial.println("BEFORE FILE READ");
    SPIFFS.begin();

    //File file = SPIFFS.open("/IBM Logo.ch8", "r");
    //Serial.println(file.size());
    ReadArduinoFile();
    Serial.println("Finished reading program");
    PrintMem();

    display->begin(0x3C, true);
    start = millis();
    
}

void Emulator::Cycle(){
    int speed = 10;
    //Serial.println("IN CYCLE");
    for (int i = 0; i < speed; i++){
        opcode = ((mem[pc] << 8u) | mem[pc + (uint16_t)1]);
        //Serial.print("PC IS: ");
        //Serial.println(pc, HEX);
        pc += (uint16_t)2;

        Execute();
    }

    if (delayTimer > 0){
        delayTimer -= 1;
    }

    if (soundTimer > 0){
        soundTimer -= 1;
    }
    //Serial.println("AFTER CYCLE");
}

void Emulator::Execute(){
    uint16_t first = (opcode & 0xF000u);
    uint16_t nnn = opcode & 0x0FFFu;
    uint8_t nn = opcode & 0x00FFu;
    uint8_t n = opcode & 0x000Fu;
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;

    //std::cout << std::dec << "PC ******************************************* " << pc << std::endl;
    //std::cout << std::hex << "FIRST " << first << std::endl; 
    //Serial.print("****************************** ");
    //Serial.println(pc, HEX);
//
    //Serial.print("OPCODE :");
    //Serial.println(opcode, HEX);
//
    //Serial.print("FIRST: ");
    //Serial.println(first, HEX);
//
    //Serial.print("nnn: ");
    //Serial.println(nnn, HEX);
//
    //Serial.print("nn: ");
    //Serial.println(nn, HEX);
//
    //Serial.print("n: ");
    //Serial.println(n, HEX);
//
    //Serial.print("x: ");
    //Serial.println(x, HEX);
//
    //Serial.print("y: ");
    //Serial.println(y, HEX);

    if (opcode == 0xE0){
        for (int i = 0; i < 2048; i++){
            pixels[i] = false;
        }
        display->clearDisplay();
        display->display();
        //std::cout << "Clear screen" << std::endl;
        //Serial.println("Clear Screen");
    }
    else if (first == 0x1000){
        pc = nnn;
        //Serial.println("1000");
    }

    else if (first == 0x6000){
        variables[x] = nn;
        //Serial.println("6000");
    }
    
    else if (first == 0xA000){
        indexReg = nnn;
        //Serial.println("A000");
    }

    else if (first == 0xD000){
        //draw instructio
        Draw(x, y);
        //Serial.println("D000");
    }

    else if (first == 0x2000){
        stack[sp] = pc;
        ++sp;
        pc = nnn;
        //Serial.println("2000");
    }

    else if (first == 0x0000){
        --sp;
        pc = stack[sp];
        //Serial.println("00EE");
    }

    else if (first == 0x3000){
        if (variables[x] == nn){
            pc += 2;
        }
        //Serial.println("3000");
    }

    else if (first == 0x4000){
        if (variables[x] != nn){
            pc += 2;
        }
        //Serial.println("4000");
    }

    else if (first == 0x5000){
        if (variables[x] == variables[y]){
            pc += 2;
        }
        //Serial.println("5000");
    }

    else if (first == 0x9000){
        if (variables[x] != variables[y]){
            pc += 2;
        }
        //Serial.println("9000");
    }

    else if (first == 0x7000){
        variables[x] += nn;
        //variables[x] &= 0xFF;
        //Serial.println("7000");
    }

    else if (first == 0x8000){
        switch (n)
        {
        case 0x0:
            variables[x] = variables[y];
            //std::cout << "8000" << std::endl;
            break;

        case 0x1:
            variables[x] |= variables[y];
            //std::cout << "8001" << std::endl;
            break;

        case 0x2:
            variables[x] &= variables[y];
            //std::cout << "8002" << std::endl;
            break;

        case 0x3:
            variables[x] ^= variables[y];
            //std::cout << "8003" << std::endl;
            break;

        case 0x4:
        {
            uint16_t sum = variables[x] + variables[y];
            if (sum > 0xFF){
                variables[0xF] = 1;
            }
            else{
                variables[0xF] = 0;
            }

            variables[x] = sum & 0xFF;
            //std::cout << "8004" << std::endl;
        }
            break;

        case 0x5:
            if (variables[x] > variables[y]){
                variables[0xF] = 1;
            }
            else if (variables[x] < variables[y]){
                variables[0xF] = 0;
            }

            variables[x] = variables[x] - variables[y];
            //std::cout << "8005" << std::endl;
            break;

        case 0x7:
            if (variables[y] > variables[x]){
                variables[0xF] = 1;
            }
            else if (variables[y] < variables[x]){
                variables[0xF] = 0;
            }

            variables[x] = variables[y] - variables[x];
            //std::cout << "8007" << std::endl;
            break;

        case 0x6:
            variables[0xF] = (variables[x] & 0x1);
            variables[x] >>= 1;
            //std::cout << "8006" << std::endl;
            break;
        
        case 0xE:
            variables[0xF] = ((variables[x] & 0x80) >> 7);
            variables[x] <<= 1;
            //std::cout << "800E" << std::endl;
            break;
        
        default:
            break;
        }
    }

    else if (first == 0xB000){
        pc = nnn + variables[0];
        //std::cout << "B000" << std::endl;
    }

    else if (first == 0xC000){
        uint8_t num = random(0, 256);
        variables[x] = num & nn;
        //std::cout << "C000" << std::endl;
    }

    else if (first == 0xE000){
        switch (nn)
        {
        case 0x9E:
            for (int i = 0; i < TOTAL_REMOTE_KEYS; i++){
                if (keys[i][2] == variables[x]){
                    if (keys[i][1] == 0x01){
                        pc += 2;
                    }
                    break;
                }
            }
            break;

        case 0xA1:
            for (int i = 0; i < TOTAL_REMOTE_KEYS; i++){
                if (keys[i][2] == variables[x]){
                    if (keys[i][1] == 0x00){
                        pc += 2;
                    }
                    break;
                }
            }
            break;
        
        default:
            break;
        }
    }

    else if (first == 0xF000){
        switch (nn)
        {
        case 0x07:
            variables[x] = delayTimer;
            //std::cout << "F007" << std::endl;
            break;

        case 0x15:
            delayTimer = variables[x];
            //std::cout << "F015" << std::endl;
            break;

        case 0x18:
            soundTimer = variables[x];
            //std::cout << "F018" << std::endl;
            break;

        case 0x1E:
            indexReg += variables[x];
            //std::cout << "F01E" << std::endl;
            break;

        case 0x0A:
        {
            bool pressed = false;
            for (int i = 0; i < TOTAL_REMOTE_KEYS; i++){
                if (keys[i][1] == 0x01){
                    variables[x] = keys[i][2];
                    pressed = true;
                    break;
                }
            }

            if (!pressed){
                pc -= 2;
            }
        }
            break;

        case 0x29:
            //font
            indexReg = variables[x] + (variables[x] * 4);
            //std::cout << "F029" << std::endl;
            break;

        case 0x33:
        {
            uint8_t num = variables[x];
            uint8_t ones = (uint8_t)(num % 10);
            uint8_t tenths = (uint8_t)(((num - ones) / 10) % 10);
            uint8_t hundreths = (uint8_t)((num - ones - (tenths * 10)) / 100);

            //mem[indexReg] = hundreths;
            //mem[indexReg + 1] = tenths;
            //mem[indexReg + 2] = ones;

            mem[indexReg + 2] = num % 10;
            num /= 10;

            mem[indexReg + 1] = num % 10;
            num /= 10;

            mem[indexReg] = num % 10;
            //std::cout << "F033" << std::endl;
        }
            break;

        case 0x55:
            for (int i = 0; i <= x; i++){
                mem[indexReg + i] = variables[i];
            }
            //std::cout << "F055" << std::endl;
            break;

        case 0x65:
            for (int i = 0; i <= x; i++){
                variables[i] = mem[indexReg + i];
            }
            //std::cout << "F065" << std::endl;
            break;
        
        default:
            break;
        }
    }

}

bool Emulator::SetPixel(int x, int y){
    if (x > (width - 1)){
        x = 0;
    }
    else if (x < 0){
        x = width - 1;
    }

    if (y > (height - 1)){
        y = 0;
    }
    else if (y < 0){
        y = height - 1;
    }

    int pixelLoc = x + (y * width);
    pixels[pixelLoc] ^= 1;

    return (bool)pixels[pixelLoc];
}

void Emulator::Render(){
    //fill screen with black
    //Serial.println("IN RENDER");
    display->clearDisplay();
    //Serial.println("HERE");

    for (unsigned int i = 0; i < 2048; i++){
        int x = i % width;
        int y = floor(i / width);

        if (pixels[i] == 1){
            //draw pixels to the screen
            display->drawRect(x * widthScaleFactor, y * heightScaleFactor, 1 * widthScaleFactor, 1 * heightScaleFactor, 1);
        }
    }
    display->display();
    //Serial.println("AFTER RENDER");
}

void Emulator::Draw(uint8_t x, uint8_t y){
    int spriteWidth = 8;
    uint8_t spriteHeight = (opcode & 0xF);
    variables[0xF] = 0;

    for (int row = 0; row < spriteHeight; row++){
        uint8_t sprite = mem[indexReg + row];

        for (int col = 0; col < spriteWidth; col++){
            if ((sprite & 0x80) > 0){
                if (SetPixel(variables[x] + col, variables[y] + row) == 0){
                    variables[0xF] = 1;
                }
            }
            sprite <<= 1;
        }
    }
}

//#ifdef MYARDUINO
void Emulator::Run(){
    
    //update the timer
    //Serial.println("IN RUN FUNCTION");
    current = millis();
    timer = current - start;
    //Serial.println("AFTER TIME RUN FUNCTION");

    if (IrReceiver.decode()){
        IrReceiver.resume();
        int remoteKeyPressed = -1;

        //check if power button pressed to change program

        

        for (int i = 0; i < TOTAL_REMOTE_KEYS; i++){
            keys[i][1] = 0;

            if (remoteKeyPressed == -1){
                if (IrReceiver.decodedIRData.command == keys[i][0]){
                    Serial.print("Key pressed ");
                    Serial.println(keys[i][0], HEX);
                    remoteKeyPressed = i;
                    break;
                }
            }
        }

        if (remoteKeyPressed > -1){
            keys[remoteKeyPressed][1] = 0x01;
        }
    }

    if (timer > interval){
        //Serial.println("BEFORE CYCLE");
        timer = 0UL;
        start = current;
        Cycle();
        Render();
    }
}
//#endif

void Emulator::ReadArduinoFile(){
    File file = SPIFFS.open(rom, "r");
    if (!file){
        Serial.println("Cannot open file");
    }


    Serial.println("FILE OPEN");
    int size = file.size();
    Serial.print("SIZE IS : ");
    Serial.println(size);
    
    char* buffer = new char[size];
    Serial.print("CONTENTS: ");
    file.readBytes(buffer, (size_t)size);
    for (int i = 0; i < size; i++){
        Serial.println(buffer[i], HEX);
    }
    file.close();
    

    //Serial.println(buffer, HEX);

    for (int i = 0; i < size; i++){
        mem[0x200 + i] = buffer[i];
    }
    delete[] buffer;
    //Serial.println("READ FILE");
}

void Emulator::PrintMem(){
    for (int i = 0; i < 4096; i++){
        Serial.print(mem[i], HEX);
        Serial.print(", ");
    }
    Serial.println();
}

void Emulator::PrintPixels(){
    for (int i = 0; i < 64 * 32; i++){
        //std::cout << pixels[i] << ", ";
    }
}

void Emulator::GetAllRomNames(){
    
}

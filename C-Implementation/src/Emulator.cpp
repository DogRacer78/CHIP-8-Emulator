#include <Emulator.hpp>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <raylib.h>
#include <iostream>
#include <vector>

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

    rom = fname;
    InitWindow(width * widthScaleFactor, height * heightScaleFactor, "CHIP-8");
    LoadFile();
    PrintMem();
}

void Emulator::Cycle(){
    int speed = 10;
    for (int i = 0; i < speed; i++){
        opcode = ((mem[pc] << 8u) | mem[pc + 1]);
        pc += 2;
        Execute();
    }

    if (delayTimer > 0){
        delayTimer -= 1;
    }

    if (soundTimer > 0){
        soundTimer -= 1;
    }
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

    if (first == 0xE0){
        for (int i = 0; i < 2048; i++){
            pixels[i] = false;
        }
        ClearBackground(BLACK);
        std::cout << "Clear screen" << std::endl;
    }
    else if (first == 0x1000){
        pc = nnn;
        std::cout << "1NNN" << std::endl;
    }

    else if (first == 0x6000){
        variables[x] = nn;
        std::cout << "6NNN" << std::endl;
    }
    
    else if (first == 0xA000){
        indexReg = nnn;
        std::cout << "AX" << std::endl;
    }

    else if (first == 0xD000){
        //draw instructio
        Draw(x, y);
        std::cout << "DxyN" << std::endl;
    }

    else if (first == 0x2000){
        stack[sp] = pc;
        ++sp;
        pc = nnn;
        std::cout << "2NNN" << std::endl;
        //std::cout << "PC ******************************************* " << pc << std::endl; 
    }

    else if (first == 0x0000){
        --sp;
        pc = stack[sp];
        std::cout << "00EE" << std::endl;
    }

    else if (first == 0x3000){
        if (variables[x] == nn){
            pc += 2;
        }
        std::cout << "3NNN" << std::endl;
    }

    else if (first == 0x4000){
        if (variables[x] != nn){
            pc += 2;
        }
        std::cout << "4NNN" << std::endl;
    }

    else if (first == 0x5000){
        if (variables[x] == variables[y]){
            pc += 2;
        }
        std::cout << "5NNN" << std::endl;
    }

    else if (first == 0x9000){
        if (variables[x] != variables[y]){
            pc += 2;
        }
        std::cout << "9NNN" << std::endl;
    }

    else if (first == 0x7000){
        variables[x] += nn;
        //variables[x] &= 0xFF;
        std::cout << "7NNN" << std::endl;
    }

    else if (first == 0x8000){
        switch (n)
        {
        case 0x0:
            variables[x] = variables[y];
            std::cout << "8000" << std::endl;
            break;

        case 0x1:
            variables[x] |= variables[y];
            std::cout << "8001" << std::endl;
            break;

        case 0x2:
            variables[x] &= variables[y];
            std::cout << "8002" << std::endl;
            break;

        case 0x3:
            variables[x] ^= variables[y];
            std::cout << "8003" << std::endl;
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
            std::cout << "8004" << std::endl;
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
            std::cout << "8005" << std::endl;
            break;

        case 0x7:
            if (variables[y] > variables[x]){
                variables[0xF] = 1;
            }
            else if (variables[y] < variables[x]){
                variables[0xF] = 0;
            }

            variables[x] = variables[y] - variables[x];
            std::cout << "8007" << std::endl;
            break;

        case 0x6:
            variables[0xF] = (variables[x] & 0x1);
            variables[x] >>= 1;
            std::cout << "8006" << std::endl;
            break;
        
        case 0xE:
            variables[0xF] = ((variables[x] & 0x80) >> 7);
            variables[x] <<= 1;
            std::cout << "800E" << std::endl;
            break;
        
        default:
            break;
        }
    }

    else if (first == 0xB000){
        pc = nnn + variables[0];
        std::cout << "B000" << std::endl;
    }

    else if (first == 0xC000){
        uint8_t num = (rand() % 256);
        variables[x] = num & nn;
        std::cout << "C000" << std::endl;
    }

    else if (first == 0xE000){
        switch (nn)
        {
        case 0x9E:
            //key pressed code
            break;

        case 0xA1:
            //keys pressed code
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
            std::cout << "F007" << std::endl;
            break;

        case 0x15:
            delayTimer = variables[x];
            std::cout << "F015" << std::endl;
            break;

        case 0x18:
            soundTimer = variables[x];
            std::cout << "F018" << std::endl;
            break;

        case 0x1E:
            indexReg += variables[x];
            std::cout << "F01E" << std::endl;
            break;

        case 0x0A:
            //key code
            break;

        case 0x29:
            //font
            indexReg = variables[x] + (variables[x] * 4);
            std::cout << "F029" << std::endl;
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
            std::cout << "F033" << std::endl;
        }
            break;

        case 0x55:
            for (int i = 0; i <= x; i++){
                mem[indexReg + i] = variables[i];
            }
            std::cout << "F055" << std::endl;
            break;

        case 0x65:
            for (int i = 0; i <= x; i++){
                variables[i] = mem[indexReg + i];
            }
            std::cout << "F065" << std::endl;
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
    ClearBackground(BLACK);

    for (unsigned int i = 0; i < 2048; i++){
        int x = i % width;
        int y = floor(i / width);

        if (pixels[i] == 1){
            //draw pixels to the screen
            DrawRectangle(x * widthScaleFactor, y * heightScaleFactor, 1 * widthScaleFactor, 1 * heightScaleFactor, WHITE);
        }
    }
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

void Emulator::LoadFile(){
    //for loading the file will need to be different for the microcontroller flash

    std::ifstream file(rom, std::ios::binary | std::ios::ate);

    if (file.is_open()){
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; i++){
            mem[0x200 + i] =  buffer[i];
        }

        delete[] buffer;
    }
}

void Emulator::Run(){
    float interval = 1.0f / (float)fps;
    float timer = 0.0;

    SetTargetFPS(60);

    while (!WindowShouldClose()){
        timer += GetFrameTime();

        BeginDrawing();

        if (timer > interval){

            Cycle();
            Render();

            timer = 0.0f;
        }

        EndDrawing();
    }
}

void Emulator::PrintMem(){
    for (int i = 0; i < 4096; i++){
        std::cout << (int)mem[i] << ", ";
    }
}

void Emulator::PrintPixels(){
    for (int i = 0; i < 64 * 32; i++){
        std::cout << pixels[i] << ", ";
    }
}
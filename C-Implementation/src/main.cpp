#include <iostream>
#include <time.h>
#include <stdlib.h>
#define ARDUINO
//define WINDOWS

#include <Emulator.hpp>

int main(){

    srand(time(NULL));
    Emulator emu = Emulator("../../ROMS/TETRIS");
    emu.Run();

    return 0;
}
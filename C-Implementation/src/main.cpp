#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <Emulator.hpp>

int main(){

    srand(time(NULL));
    Emulator emu = Emulator("");
    emu.Run();

    return 0;
}


#include "chip8.hpp"
#include <chrono>
#include "Platform.hpp"
#include <iostream>

using namespace std;

int main(int argc,char** argv){

    if(argc != 4){
        cout << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        exit(EXIT_FAILURE);
    }

    int videoscale = stoi(argv[1]);
    int delay = stoi(argv[2]);
    char const* rom = argv[3];

    Platform platform("CHIP-8 Emulator",VIDEO_WIDTH * videoscale,VIDEO_HEIGHT * videoscale, VIDEO_WIDTH,VIDEO_HEIGHT);

    chip8 CHIP8;
    CHIP8.load_rom(rom);
   if(rom){
    cout<<"rom is loaded"<<endl;
   } 
    int videoPitch = sizeof(CHIP8.display[0]) *  VIDEO_WIDTH;
    auto lastCycle = chrono::high_resolution_clock::now();
    bool quit = false;

    while(!quit){
        quit = platform.ProcessInput(CHIP8.keyboard);
        auto currentTime = chrono::high_resolution_clock::now();
        float dt = chrono::duration<float, chrono::milliseconds::period>(currentTime - lastCycle).count();

        if( dt > delay ){
            lastCycle = currentTime;
            cout<<"pc value : "<<hex<<CHIP8.pc<<" index : "<<hex<<CHIP8.index<<endl;
            CHIP8.cycle();
            platform.Update(CHIP8.display, videoPitch);
            // cout << "Cycle completed, dt: " << dt << endl;
        }
    }

    return 0;
}
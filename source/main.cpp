#include <chrono>
#include <iostream>
#include "platform.hpp"
#include "chip8.hpp"

int main(int argc, char** argv){
    // if the argument is not 4, error
    if (argc != 4){
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>"<<std::endl; 
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const* romName = argv[3];

    Platform platform("CHIP-8 Emulator by Peter Lee", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadROM(romName);

    // pitch of video is size of a row
    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    // store last Cycletime to calculate cycle
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    // while program is not quitting
    while (!quit){
        // if ProcessInput returns 1, keypress is done
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();

        //calculate delaytime (current cycle)
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay){
            lastCycleTime = currentTime;

            chip8.Cycle();

            platform.Update(chip8.video, videoPitch);
        }
    }
    return 0;
}
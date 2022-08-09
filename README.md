# CHIP-8 Emulator

CHIP-8 emulator based on [Austin Morlan's CHIP-8 emulator](https://austinmorlan.com/posts/chip8_emulator/).

This is my first emulator project, and I am hoping to develop a gameboy emulator for my next project.

Here are some takeaways from this project:

- Always check datatypes and how much data they can store
  
  - ```uint8_t``` only stores 8 bits, so when I did
  
    ``` c++
    uint8_t x = opcode & 0x0F00u;
    x = x  >> 8u;
    ```

    x variable ends up only storing 0 since it can only save the first byte. Hence, the variable's datatype should be bigger than 8 bits or do all of operation in one line like below

    ``` c++
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    ```

![Tetris on my CHIP-8 emulator](/images/tetris.png)

## Usage

Required library: SDL2

Use makefile within source directory to build the emulator

``` command
cd source
make
```

Run the emulator

``` command
./chip8 <Scale> <Delay> <ROM>
```

- Scale: Multiplier for the window. Scale of 1 is 64 x 32 pixles
- Delay: Delay between each instruction. Use this to control speed of a program execution
- ROM: ROM file name. I recommend downloading from [this](https://github.com/dmatlack/chip8/tree/master/roms/games) repo
#include "chip8.hpp"
#include <fstream>
#include <chrono>
#include <random>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

void Chip8::LoadROM(char const* filename){
    // Open file and point file pointer at the end of the file
    // | std::ios::ate sets file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()){

        // Get the size of the input ROM and allocate it in runtime stack
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        //  move file pointer to the beginning of the file and copy the bits over
        file.seekg(0, std::ios::beg);
        file.read(buffer,size);
        file.close();

        // LD ROM contents into CHIP-8's memory, starting at 0x200
        for (long i = 0; i < size; ++i){ //why is this ++i, not i++? ++i does not create copies. So optimization?
            memory[START_ADDRESS + i]= buffer[i];
        }

        // free the runtime stack after copying the rom contents over
        delete[] buffer;
    }
}

// Font pixel data. Source: austin/Chip8-emulator
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Create a constructor where PC is initialized to 0x200
Chip8::Chip8() 
    // sets a seed of randome engine using current time. Since data member randbyte is uint8_t, its from 0 to 255
    : randGen(std::chrono::system_clock::now().time_since_epoch().count())
    {
    // Initialize PC
    pc = START_ADDRESS;

    // Load font data into buffer
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i){
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Initialize rand num generator of range (0, 255) 
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

/*
Opcode: 
Functionality: 
Implementation: 
*/

/* 
Opcode: 00E0 (CLS)
Functionality: Clear the display
Implementation: Clear the video array's buffer to zero
*/
void Chip8::OP_00E0(){
    memset(video, 0, sizeof(video));
}

/*
Opcode: 00EE (RET)
Functionality: Return from a subroutine
Implementation: Pop 1 from SP and apply it to PC; SP is TOS
*/
void Chip8::OP_00EE(){
    --sp;
    pc = stack[sp];
}

/*
Opcode: 1nnn (JP)
Functionality: Jump to memory location nnn
Implementation: bitmask opcode to extract nnn and aply it to PC
*/
void Chip8::OP_1nnn(){
    // bitmask with 0x0FFF will only leave the last three HEX codes
    uint16_t address = opcode & 0x0FFFu;

    pc = address;
}

/*
Opcode: 2nnn (CALL)
Functionality: Call subroutine at nnn
Implementation: Push PC into stack and change PC to nnn
*/
void Chip8::OP_2nnn(){

    // Push PC into stack and increment sp
    stack[sp] = pc;
    ++sp;

    // Bitmask with 0x0FFF
    uint16_t address = opcode & 0x0FFFu;

    pc = address;
}

/*
Opcode: 3xkk (SE Vx, byte)
Functionality: Skip next instruction if register vx has value kk
Implementation: Bitmask opcode with 0x0F00 and bit right shift 8 times to find x.
Then bitmask opcode again with 0x00FF to extract kk. Add 2 to PC if kk == v[x]
*/
void Chip8::OP_3xkk(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t kk = opcode & 0x00FFu;

    if(registers[x] == kk){
        pc += 2;
    }
}

/*
Opcode: 4xkk (SNE Vx, byte)
Functionality: Skip next instruction if register vx does not have value kk
Implementation: Same as 3xkk, but if statement is != instead of ==
*/
void Chip8::OP_4xkk(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t kk = opcode & 0x00FFu;

    if(registers[x] != kk){
        pc += 2;
    }
}

/*
Opcode: 5xy0 (SE Vx, Vy)
Functionality: Skip next instruction if register vx == register vy
Implementation: Netmask x and y and right bit shift accordingly, compare them using conditionals
*/
void Chip8::OP_5xy0(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    if(registers[x] == registers[y]){
        pc += 2;
    }
}

/*
Opcode: 6xkk (LD Vx, byte)
Functionality: Load kk into register Vx
Implementation: Netmask kk and load it into registers[x]
*/
void Chip8::OP_6xkk(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t kk = opcode & 0x00FFu;

    registers[x] = kk;
}

/*
Opcode: 7xkk (ADD Vx, byte)
Functionality: Add immediate value to register x
Implementation: Bitmask x and kk individually and add kk's value into register[x]
*/
void Chip8::OP_7xkk(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t kk = opcode & 0x00FFu;

    registers[x] += kk;
}

/*
Opcode: 8xy0 (LD Vx, Vy)
Functionality: Load register Vy's value into Vx
Implementation: Bitmask x and y individually and set v[x] = v[y]
*/
void Chip8::OP_8xy0(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    registers[x] = registers[y];
}

/*
Opcode: 8xy1 (OR Vx, Vy)
Functionality: Set Vx = Vx | Vy
Implementation: Bitmask x and y individually and set v[x] = v[x] | v[y]
*/
void Chip8::OP_8xy1(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    registers[x] = registers[x] | registers[y];
}

/*
Opcode: 8xy2 (AND Vx, Vy)
Functionality: Set Vx = Vx & Vy
Implementation: Bitmask x and y individually and set v[x] = v[x] & v[y]
*/
void Chip8::OP_8xy2(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    registers[x] = registers[x] & registers[y];
}

/*
Opcode: 8xy3 (XOR Vx, Vy)
Functionality: Set Vx = Vx ^ Vy
Implementation: Bitmask x and y individually and set v[x] = v[x] ^ v[y]
*/
void Chip8::OP_8xy3(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    registers[x] = registers[x] ^ registers[y];
}

/*
Opcode: 8xy4 (ADD Vx, Vy)
Functionality: Set Vx = Vx ^ Vy
Implementation: Bitmask x and y individually and set v[x] = v[x] + v[y]
if total is bigger than 255, set VF to 1 and subtract/bitmask carry.
*/
void Chip8::OP_8xy4(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    uint32_t total = registers[x] + registers[y];

    if (total > 255u){
        registers[0xF] = 1u;
    }
    else{
        registers[0xF] - 0u;
    }
    /*registers[x] = total - 256u;*/ //this is also possible probably, but bitwise is faster
    registers[x] = total & 0xFFu;
}

/*
Opcode: 8xy5 (SUB Vx, Vy)
Functionality: Bitmask x and y individually and set v[x] = v[x] - v[y]
Implementation: Bitmask x and y individually and set v[x] = v[x] - v[y]
if Vx is bigger than Vy, set V[16] to 1 and 0 otherwise.
*/
void Chip8::OP_8xy5(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    if (registers[x] > registers[y]){
        registers[0xF] = 1u;
    }
    else{
        registers[0xF] - 0u;
    }

    registers[x] = registers[x] - registers[y];

}

/*
Opcode: 8xy6 (SHR Vx)
Functionality: Vx >> 1; VF is set to 1 when Vx's LSB is 1. Div by 2.
Implementation: use >> operator after bitmasking to get Vx
*/
void Chip8::OP_8xy6(){

    uint8_t x = opcode & 0x0F00u;
    // save LSB to VF
    registers[0xF] = registers[x] & 0x1;

    registers[x] = registers[x] >> 1;
}

/*
Opcode: 8xy7 (SUBN Vx, Vy)
Functionality: Vx = Vy - Vx
Implementation: same as 8xy5 but swap Vx and Vy
*/
void Chip8::OP_8xy7(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;
    
    uint8_t y = opcode & 0x00F0u;
    y = y>>4u;

    if (registers[x] < registers[y]){
        registers[0xF] = 1u;
    }
    else{
        registers[0xF] - 0u;
    }

    registers[x] = registers[y] - registers[x];
}

/*
Opcode: 8xyE (SHL Vx)
Functionality: Vx << 1; VF is set to 1 when Vx's LSB is 1. Mult by 2.
Implementation: 
*/
void Chip8::OP_8xyE(){

    uint8_t x = opcode & 0x0F00u;
    x = x>>8u;

    // save LSB to VF
    registers[0xF] = registers[x] & 0x80;
    registers[0xF] = registers[0xF] >> 7u;

    registers[x] = registers[x] << 1;
}

/*
Opcode: 9xy0 (SNE, Vx, Vy)
Functionality: Skip next instruction if Vx != Vy
Implementation: Extract Vx and Vy using bitmasking and conditionally check them
*/
void Chip8::OP_9xy0(){

    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    uint8_t y = opcode & 0x00F0u;
    y = y >> 4u;

    if(registers[x] != registers[y]){
        pc += 2;
    }
}

/*
Opcode: ANNN (LD I, addr)
Functionality: Load index register an addr nnn
Implementation: 
*/
void Chip8::OP_Annn(){

    uint16_t nnn = opcode & 0x0FFFu;

    index = nnn;
}

/*
Opcode: Bnnn (JP V0, addr)
Functionality: Jump to location nnn + V0
Implementation: 
*/
void Chip8::OP_Bnnn(){

    uint16_t nnn = opcode & 0x0FFFu;
    
    pc = nnn + registers[0];
}

/*
Opcode: Cxkk (RND Vx, byte) 
Functionality: Set Vx = random byte & kk
Implementation: randomly generate using randByte
*/
void Chip8::OP_Cxkk(){
    
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    uint8_t kk = opcode & 0x00FFu;

    registers[x] = randByte(randGen) & kk;

}

/*
Opcode: DRW Vx, Vy, nibble
Functionality: Display n-byte sprite starting at mem loc I at (Vx, Vy), set VF = collision.
Implementation: 
*/
void Chip8::OP_Dxyn(){
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;
    // sprite is always 8 pixels (8 bits)

    // modulo to video width/height to make it wrap around screen if off bounds
    uint8_t xCoord = registers[x] % VIDEO_WIDTH;
    uint8_t yCoord = registers[y] % VIDEO_HEIGHT;

    // initialize VF as 0 for collision
    registers[0xF] = 0; 

    for(unsigned int row = 0; row < height; ++row)
    {   
        // each byte of sprite represents each row of sprite
        uint8_t spriteByte = memory[index + row];

        for(unsigned int col = 0; col < 8; ++col){
            // spritepixel is pixel of the sprite at given column
            // we don't need to bitshift spritePixel 
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            // screenPixel represents pixel that is on the screen before sprite
            // screenPixel is 32 bit pointer for ease of use for SDL
            uint32_t* screenPixel = &video[(yCoord + row) * VIDEO_WIDTH + (xCoord + col)];

            // if sprite pixel is on at a given position
            if(spritePixel){
                // and if screen pixel is also on on the same position, meaning collision
                if (*screenPixel == 0xFFFFFFFF){
                    // then mark VF as 1
                    registers[0xF] = 1;
                }

                // XOR the sprite pixel with screen pixel to do cool stuff
                *screenPixel = *screenPixel ^ 0xFFFFFFFF;
            }
        }
    }

}

/*
Opcode: Ex9E (SKP Vx) 
Functionality: Skip next instruction if key with the value of Vx is pressed
Implementation: 
*/
void Chip8::OP_Ex9E(){
    uint8_t x = (opcode & 0x0F00u);
    x = x >> 8u;

    uint8_t key = registers[x];

    if(keypad[key]){
        pc += 2;
    }

}

/*
Opcode: 
Functionality: 
Implementation: 
*/
void Chip8::OP_(){
}
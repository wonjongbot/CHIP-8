#include "chip8.hpp"
#include <fstream>
#include <chrono>
#include <cstdint>
#include <random>
#include <cstring>
#include <iostream>

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

    // Function pointer table
    // First three letter is 00E, Tb0 function makes the class pointer depending on last digit (0 or E)
    table[0x0] = &Chip8::Tb0;
    // table[0x1:0xD] points to functions whose entire opcode is unique
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Tb8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    // TbE points to a function that matches its opcode and whose first digit is E
    table[0xE] = &Chip8::TbE;
    // TbF points to a function that matches its opcode and whose first digit is E
    table[0xF] = &Chip8::TbF;

    // Function pointer for Opcodes with first three digit 00E and last two E0
    table0[0x0] = &Chip8::OP_00E0;
    // Function pointer for Opcodes with first three digit 00E and last two EE
    table0[0xE] = &Chip8::OP_00EE;

    // Function pointers for Opcodes with first digit 8
    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    // Function pointers for Opcodes with first digit E
    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    // Function pointers for Opcodes with first digit F
    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::Cycle(){
    /*
     * Instruction Cycle: Fetch
     * Fetch opcode from memory where PC is pointing. Each memory location stores
     * single byte, while an opcode is 2 bytes. Therefore, a byte at PC is fetched,
     * right bit shifted, and another byte is OR'd so two bytes are combined.
     */
    opcode = (memory[pc] << 8u) | memory[pc+1];
    std::cout << "executing "<<std::hex<<opcode<<std::endl;

    /*
     * Instruction Cycle: Increment PC
     * PC now points at next instruction. It is incremented by 2 because CHIP-8's
     * Opcode is 2 bytes long and each memory location is 1 byte long.
     */
    pc += 2;

    /*
     * Instruction Cycle: Decode & Execute
     * Decode and execute opcode at the same time using function table!
     * 12 bit is shifted to the right since our function table looks at first
     * digit of the opcode.
     */
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    // Decrement delay timer if set
    if(delayTimer > 0){
        --delayTimer;
    }
    
    // Decrement sound timer if set (computer emits sound when sound timer is nonzero)
    if(soundTimer > 0){
        --soundTimer;
    }
}

/*
 * This gets the pointer of the current object, dereferences it, then dereferences the function 
 * pointed by the function pointer array. What a mouthfull!
 * 
 * Tb0 function bimasks and extracts only last digit of an opcode,
 * and this digit is used to access the matching function in the array.
 * Only Opocdes that passes this function is opcodes whose first three digit is 00E
 */
void Chip8::Tb0(){
    ((*this).*(table0[opcode & 0x000Fu]))();
}

// Only Opocdes that passes this function is opcodes whose first digit is 8
void Chip8::Tb8(){
    ((*this).*(table8[opcode & 0x000Fu]))();
}

// Only Opocdes that passes this function is opcodes whose first digit is E
void Chip8::TbE(){
    ((*this).*(tableE[opcode & 0x000Fu]))();
}

// Only Opocdes that passes this function is opcodes whose first digit is F
void Chip8::TbF(){
    ((*this).*(tableF[opcode & 0x00FFu]))();
}

// NULL function for invalid OPs
void Chip8::OP_NULL(){
}

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
        registers[0xF] = 0u;
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
        registers[0xF] = 0u;
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
    x = x >> 8u;
    // save LSB to VF
    registers[0xF] = (registers[x] & 0x1);

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
        registers[0xF] = 0u;
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
                *screenPixel = (*screenPixel) ^ 0xFFFFFFFF;
            }
        }
    }

}

/*
Opcode: Ex9E (SKP Vx) 
Functionality: Skip next instruction if key with the value of Vx is pressed
Implementation: decrement PC by 2 if no key is pressed (which just creates an infinite loop)
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
Opcode: ExA1 (SKNP Vx)
Functionality: Skip next instruction if key with the value of Vx is NOT pressed
Implementation: 
*/
void Chip8::OP_ExA1(){
    uint8_t x = (opcode & 0x0F00u);
    x = x >> 8u;

    uint8_t key = registers[x];

    if(!keypad[key]){
        pc += 2;
    }
}

/*
Opcode: Fx07 (LD Vx, DT)
Functionality: set Vx as value of delay timer
Implementation: 
*/
void Chip8::OP_Fx07(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    registers[x] = delayTimer;
}

/*
Opcode: Fx0A (LD, Vx, k)
Functionality: Wati for keypress and store that value in Vx
Implementation: 
*/
void Chip8::OP_Fx0A(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    if (keypad[0]){
        registers[x] = 0;
    }
    else if (keypad[1]){
        registers[x] = 1;
    }
    else if (keypad[2]){
        registers[x] = 2;
    }
    else if (keypad[3]){
        registers[x] = 3;
    }
    else if (keypad[4]){
        registers[x] = 4;
    }
    else if (keypad[5]){
        registers[x] = 5;
    }
    else if (keypad[6]){
        registers[x] = 6;
    }
    else if (keypad[7]){
        registers[x] = 7;
    }
    else if (keypad[8]){
        registers[x] = 8;
    }
    else if (keypad[9]){
        registers[x] = 9; 
    }
    else if (keypad[10]){
        registers[x] = 10;
    }
    else if (keypad[11]){
        registers[x] = 11;
    }
    else if (keypad[12]){
        registers[x] = 12;
    }
    else if (keypad[13]){
        registers[x] = 13;
    }
    else if (keypad[14]){
        registers[x] = 14;
    }
    else if (keypad[15]){
        registers[x] = 15;
    }
    else{
        pc -= 2;
    }
}

/*
Opcode: Fx15 (LD DT, Vx)
Functionality: set delay timer as Vx
Implementation: 
*/
void Chip8::OP_Fx15(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    delayTimer = registers[x];
}

/*
Opcode: Fx18 (LD ST, Vx)
Functionality: set sound timer as Vx
Implementation: 
*/
void Chip8::OP_Fx18(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    soundTimer = registers[x];
}

/*
Opcode: Fx1E (ADD I, Vx)
Functionality: set I = I + Vx
Implementation: 
*/
void Chip8::OP_Fx1E(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    index += registers[x];
}

/*
Opcode: Fx29 (LD F, Vx)
Functionality: set I as location of sprite for digit Vx
Implementation: 
*/
void Chip8::OP_Fx29(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    index  = FONTSET_START_ADDRESS + (registers[x] * 5);
}

/*
Opcode: Fx33 (LD B, Vx)
Functionality: Store BCD rep of Vx (0~255) in mem loc I (1e2), I+1 (1e1), and I+x (1e0)
Implementation: modulo 10 to extract smallest digit, store that, and divide 10 again.
*/
void Chip8::OP_Fx33(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;
    uint8_t num = registers[x];

    memory[index+2] = num % 10;
    num /= 10;

    memory[index+1] = num % 10;
    num /= 10;

    memory[index] = num % 10;
}

/*
Opcode: Fx55 (LD [I], Vx)
Functionality: Store registers V0 through Vx in mem starting at loc I
Implementation: 
*/
void Chip8::OP_Fx55(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    for(uint8_t i = 0; i <= x; ++i){
        memory[index + i] = registers[x];
    }
}

/*
Opcode: Fx65 (LD Vx, [I])
Functionality: Load registers V0 through Vx into mem starting at loc I
Implementation: 
*/
void Chip8::OP_Fx65(){
    uint8_t x = opcode & 0x0F00u;
    x = x >> 8u;

    for(uint8_t i = 0; i <= x; ++i){
        registers[x] = memory[index + i];
    }
}
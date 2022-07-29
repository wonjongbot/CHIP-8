#include <cstdint>
#include <random>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVEL = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8{
    public:
        //constructor for the emulator
        Chip8();
        void LoadROM(char const* filename);

        /* Data Structure for Chip8 class */
        // 15 general registers, 16th register is used to hold flag about operation results
        uint8_t registers[REGISTER_COUNT]{};
        // memory is 4k bits
        uint8_t memory[MEMORY_SIZE]{};
        // Index register store memory addresses for use in operations; LC-3 equivalent of MAR
        uint16_t index{};
        // Program counter
        uint16_t pc{};
        // apparently CHIP-8 has 16 levels call stack; it can hold up til 16 PCs (17 function calls)
        uint16_t stack[STACK_LEVEL]{};
        // stack pointer
        uint8_t sp{};
        // 8 bit delay timer. Only decrements to 0 at rate of 60Hz
        uint8_t delayTimer{};
        // same as above but for sound. Decrements in 60Hz if non-zero
        uint8_t soundTimer{};
        // input arrays
        uint8_t keypad[KEY_COUNT]{};
        // memory for display (64 x 32)
        uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
        // opcodes!
        uint16_t opcode;

    private:
        // random engine is called randGen
        std::default_random_engine randGen;
        // randByte will be used as seed
        std::uniform_int_distribution<uint8_t> randByte;
        
        // CLS
        void OP_00E0();
        // RET
        void OP_00EE();
        // JP
        void OP_1nnn();
        // CALL
        void OP_2nnn();
        // SE (immediate)
        void OP_3xkk();
        // SNE (immediate)
        void OP_4xkk();
        // SE (register)
        void OP_5xy0();
        // LD (immediate)
        void OP_6xkk();
        // ADD (immediate)
        void OP_7xkk();
        // LD (register)
        void OP_8xy0();
        // OR (register)
        void OP_8xy1();
        // AND (register)
        void OP_8xy2();
        // XOR (register)
        void OP_8xy3();
        // ADD (register)
        void OP_8xy4();
        // SUB (register)
        void OP_8xy5();    
        // SHR
        void OP_8xy6();
        // SUBN
        void OP_8xy7();
        // SHL
        void OP_8xyE();
        // SNE
        void OP_9xy0();
        // LD I (immediate)
        void OP_Annn();
        // JP (register)
        void OP_Bnnn();
        // RND (immediate)
        void OP_Cxkk();
        // Dxyn
        void OP_Dxyn();
        // Ex9E
        void OP_Ex9E();
};
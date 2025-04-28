#include <iostream>
#include <vector>
#include <string>

using namespace std;

enum class ProcState {
    IDLE,
    READ_HIT_SME_NORM,
    READ_MISS_I_NORM,
    READ_MISS_I_NORM_RM,
    READ_MISS_S_REP,
    READ_MISS_S_REP_RM,
    READ_MISS_M_REP,
    READ_MISS_M_REP_WB,
    READ_MISS_M_REP_RM,
    WRITE_HIT_M_NORM,
    WRITE_HIT_M_NORM_WH,
    WRITE_HIT_S_NORM,
    WRITE_HIT_S_NORM_IV,
    WRITE_MISS_I_NORM,
    WRITE_MISS_I_NORM_WM,
    WRITE_MISS_S_REP,
    WRITE_MISS_S_REP_WM,
    WRITE_MISS_M_REP,
    WRITE_MISS_M_REP_WB,
    WRITE_MISS_M_REP_WM,
};

// enum class SnoopState {

// };

enum class CacheState {
    INVALID,
    SHARED,
    MODIFIED,
    EXCLUSIVE
};

struct CacheLine {
    int tag;
    CacheState state;
    

    CacheLine() : tag(0), state(CacheState::INVALID) {}
};

enum InstructionType {
    READ,
    WRITE
};

struct Instructions {
    InstructionType type;
    unsigned int address;
};

class Processor {
    private:
        int s;
        int E;
        int b;
        ProcState state;

        vector<vector<CacheLine>> cache;
        vector<Instructions> instructions;

        CacheState getCacheState(unsigned int address) {
            unsigned int setIndex = (address >> b) & ((1 << s) - 1);
            unsigned int tag = address >> (s + b);

            for (int i = 0; i < E; ++i) {
                if (cache[setIndex][i].tag == tag) {
                    return cache[setIndex][i].state;
                }
            }
            return CacheState::INVALID;
        }


    public:
        Processor(int s, int E, int b, vector<Instructions> instructions) : s(s), E(E), b(b), instructions(instructions) {
            cache.resize(1 << s, vector<CacheLine>(E));
            state = ProcState::IDLE;
        }
};


int main () {

}
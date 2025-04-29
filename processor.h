#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include <string>


enum class ProcState
{
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
    WRITE_HIT_S_NORM,
    WRITE_HIT_S_NORM_IV,
    WRITE_MISS_I_NORM,
    WRITE_MISS_I_NORM_WM,
    WRITE_MISS_S_REP,
    WRITE_MISS_S_REP_IV,
    WRITE_MISS_S_REP_WM,
    WRITE_MISS_M_REP,
    WRITE_MISS_M_REP_WB,
    WRITE_MISS_M_REP_WM,
    COMPLETE,
};

enum class CacheState
{
    INVALID,
    SHARED,
    MODIFIED,
    EXCLUSIVE
};

struct CacheLine
{
    int tag;
    CacheState state;

    CacheLine();
};

enum InstructionType
{
    READ,
    WRITE
};

struct Instructions
{
    char type;
    unsigned int address;
};

class Processor
{
private:
    int s;
    int E;
    int b;
    ProcState state;

    int line;
    int core;

    char currentInstruction;
    int currentAddress;

    std::vector<std::vector<CacheLine>> cache;
    std::vector<std::vector<int>> cacheLRU;
    std::vector<std::pair<char, int>> instructions;

    CacheState getCacheState(unsigned int address);

    void setCacheLineState (CacheState cachestate);

public:
    Processor(int s, int E, int b, int core, std::vector<std::pair<char, int>> instructions);
    void completeTransaction();
    bool processInstruction();
    bool requestOwnership();
};

#endif // PROCESSOR_H

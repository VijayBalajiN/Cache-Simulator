#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include <string>


enum class ProcState
{
    IDLE,
    READ_HIT,
    READ_MISS,
    WRITE_HIT,
    WRITE_INVALIDATION,
    WRITE_MISS,
};

enum class CacheState
{
    INVALID,
    SHARED,
    MODIFIED,
    EXCLUSIVE,
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

enum class MemoryCache {
    CORE0 = 0,
    CORE1 = 1,
    CORE2 = 2,
    CORE3 = 3,
    Memory = 4,
    NONE = 5
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

    CacheState getLRUCacheLine(unsigned int address);

    void setCacheLineState (CacheState cachestate);

public:
    Processor(int s, int E, int b, int core, std::vector<std::pair<char, int>> instructions);
    void completeTransaction(MemoryCache owner);
    bool processInstruction();
    bool requestOwnership();
};

#endif // PROCESSOR_H

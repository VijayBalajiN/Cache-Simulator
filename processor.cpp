#include <iostream>
#include <vector>
#include <string>
#include "processor.h"
#include <assert.h>

using namespace std;

CacheLine::CacheLine() : tag(0), state(CacheState::INVALID) {}

CacheState Processor::getCacheState(unsigned int address)
{
    unsigned int setIndex = (address >> b) & ((1 << s) - 1);
    unsigned int tag = address >> (s + b);

    for (int i = 0; i < E; ++i)
    {
        if (cache[setIndex][i].tag == tag)
        {
            return cache[setIndex][i].state;
        }
    }
    return CacheState::INVALID;
}

Processor::Processor(int s, int E, int b, int core, vector<pair<char, int>> instructions) : 
s(s), E(E), b(b), instructions(instructions), line(-1), core(core), currentInstruction(' '), currentAddress(0)
{
    cache.resize(1 << s, vector<CacheLine>(E));
    cacheLRU.resize(1 << s, vector<int>(E, 0));
    state = ProcState::IDLE;
}

void Processor::completeTransaction()
{
    switch (state)
    {
    case ProcState::IDLE:
    case ProcState::READ_HIT_SME_NORM:
    case ProcState::READ_MISS_I_NORM:
    case ProcState::READ_MISS_I_NORM_RM:
        state = ProcState::IDLE;
        break;
    case ProcState::READ_MISS_S_REP_RM:
    }
}

void Processor::setCacheLineState (CacheState cachestate) {
    unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
    unsigned int tag = currentAddress >> (s + b);

    for (int i = 0; i < E; ++i)
    {
        if (cache[setIndex][i].tag == tag)
        {
            cache[setIndex][i].state = cachestate;
            return;
        }
    }

    for (int i = 0; i < E; ++i)
    {
        if (cache[setIndex][i].state == CacheState::INVALID)
        {
            cache[setIndex][i].state = cachestate;
            cache[setIndex][i].tag = tag;
            return;
        }
    }

    cout << "ERROR, CacheLineState unable to be set!" << endl;
    assert(false);
}

bool Processor::requestOwnership() {
    switch (state){
        case ProcState::IDLE: 
        case ProcState::READ_HIT_SME_NORM: 
        case ProcState::WRITE_HIT_M_NORM:
            return false;
        default:
            return true;
    } 
}

bool Processor::processInstruction() {
    switch (state) {
        case ProcState::IDLE:
            line++;
            if (line < instructions.size()) {

                currentInstruction = instructions[line].first;
                currentAddress = instructions[line].second;

                CacheState cacheState = getCacheState(currentAddress);

                // if (currentInstruction == 'R') {
                //     state = ProcState::READ_HIT_SME_NORM;
                //     return true;
                // } else if (currInstruction == 'W') {
                //     state = ProcState::WRITE_HIT_M_NORM;
                //     return true;
                // }
                
            } else {
                state = ProcState::COMPLETE;
                return false;
            }
            break;
        default:
            break;
    }
}



int main()
{
}
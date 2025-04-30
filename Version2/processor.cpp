#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
#include <stack>
#include "parser.h"

#define MEMCYCLES 100

using namespace std;

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

CacheLine::CacheLine() : tag(0), state(CacheState::INVALID) {}

enum class BusState
{
    INVALID,
    RWITM,
    RM,
    TRANSACTION,
    IDLE,
};

struct Core
{
    int line;
    int core;

    char currentInstruction;
    int currentAddress;

    vector<vector<CacheLine>> cache;
    vector<vector<int>> cacheLRU;
    vector<pair<char, int>> instructions;

    Core(int s, int E, int b, const vector<pair<char, int>> instr)
        : line(0), core(0), currentInstruction(0), currentAddress(0), instructions(instr)
    {
        int sets = 1 << s;

        cache.resize(sets, vector<CacheLine>(E));
        cacheLRU.resize(sets, vector<int>(E, 0));
    }

    Core()
        : line(0), core(0), currentInstruction(0), currentAddress(0)
    {
        // Empty cache and instructions by default
    }
};

class Bus
{
private:
    stack<int> owners; // 0-3 are cores, 4-7 are corresponding snoops, 8 is memory

    BusState state;
    int remainingCycles;
    int source;      // 0-3 cores, 4 is memory
    int destination; // 0-3 cores, 4 is memory
    unsigned int address;

    int s;
    int E;
    int b;

    int completed[4];

    int cycle;

    Core cores[4];

    void runCore(int core)
    {
        unsigned int setIndex = (address >> b) & ((1 << s) - 1);
        unsigned int tag = address >> (s + b);

        int line = cores[core].line;
        int size = cores[core].instructions.size();

        cout << "CORE: " << core << " " << line << " SIZE: "<< size << endl;

        if (line < cores[core].instructions.size())
        {

            char currentInstruction = cores[core].instructions[line].first;
            unsigned int currentAddress = cores[core].instructions[line].second;

            CacheState cacheState = getCacheState(core, currentAddress);

            if (currentInstruction == 'R')
            {
                cout << "R" << endl;
                switch (cacheState)
                {
                case CacheState::SHARED:
                case CacheState::EXCLUSIVE:
                case CacheState::MODIFIED:
                    if (!owners.empty() && owners.top() == core)
                        owners.pop();
                    cores[core].line++;
                    break;
                case CacheState::INVALID:
                    cout << "INVALID" << endl;
                    if (owners.empty())
                    {
                        owners.push(core);
                    }

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        CacheState lruState = getInvalidCacheState(core, currentAddress);

                        switch (lruState)
                        {
                        case CacheState::INVALID:
                            state = BusState::RM;
                            destination = core;
                            address = currentAddress;
                            return;
                        case CacheState::MODIFIED:
                            state = BusState::TRANSACTION;
                            remainingCycles = 100;
                            source = core;
                            destination = 4;
                            return;
                        default:
                            cout << "This state is not reachable logically!" << endl;
                            assert(false);
                            break;
                        }
                    }
                }
            }
            else if (currentInstruction == 'W')
            {

                switch (cacheState)
                {
                case CacheState::MODIFIED:
                case CacheState::EXCLUSIVE:
                {
                    CacheState _ = getWriteCacheState(core, currentAddress);
                    if (!owners.empty() && owners.top() == core)
                        owners.pop();
                    cores[core].line++;
                    break;
                }
                case CacheState::SHARED:
                {
                    if (owners.empty())
                    {
                        owners.push(core);
                    }

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        CacheState _ = getWriteCacheState(core, currentAddress);

                        state = BusState::INVALID;
                        address = currentAddress;
                        source = core;
                    }
                    break;
                }
                case CacheState::INVALID:
                    if (owners.empty())
                    {
                        owners.push(core);
                    }

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        CacheState lruState = getInvalidCacheState(core, currentAddress); // Invalidate corresponding LRU State

                        switch (lruState)
                        {
                        case CacheState::INVALID:
                            state = BusState::RWITM;
                            destination = core;
                            address = currentAddress;
                            return;
                        case CacheState::MODIFIED:
                            state = BusState::TRANSACTION;
                            remainingCycles = 100;
                            source = core;
                            destination = 4;
                            return;
                        default:
                            cout << "This state is not reachable logically!" << endl;
                            assert(false);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            completed[core] = 1;
            cout << "COMPLETED: " << core << endl;
        }
    }

    bool isSetFull(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].state == CacheState::INVALID)
            {
                return false;
            }
        }
        return true;
    }

    CacheState getCacheState(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
            {
                return cores[core].cache[setIndex][i].state;
            }
        }

        return CacheState::INVALID;
    }

    CacheState getWriteCacheState(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
            {
                if (cores[core].cache[setIndex][i].state != CacheState::INVALID)
                {
                    cores[core].cache[setIndex][i].state = CacheState::MODIFIED;
                    return CacheState::MODIFIED;
                }
            }
        }

        return CacheState::INVALID;
    }

    CacheState getInvalidCacheState(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].state == CacheState::INVALID)
            {
                cores[core].cache[setIndex][i].tag = tag;
                return CacheState::INVALID;
            }
        }

        int min = 0;

        for (int i = 0; i < E; i++)
        {
            if (cores[core].cacheLRU[setIndex][i] < cores[core].cacheLRU[setIndex][min])
                min = i;
        }

        cores[core].cache[setIndex][min].tag = tag;

        if (cores[core].cache[setIndex][min].state == CacheState::EXCLUSIVE ||
            cores[core].cache[setIndex][min].state == CacheState::SHARED)
        {
            cores[core].cache[setIndex][min].state = CacheState::INVALID;
            return CacheState::INVALID;
        }
        else if (cores[core].cache[setIndex][min].state == CacheState::MODIFIED)
        {
            cores[core].cache[setIndex][min].state = CacheState::INVALID;
            return CacheState::MODIFIED;
        }
    }

    void invalidateAddress(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
            {
                cores[core].cache[setIndex][i].state = CacheState::INVALID;
            }
        }
    }

    void shareAddress(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
            {
                cores[core].cache[setIndex][i].state = CacheState::SHARED;
            }
        }
    }

    bool containsAddress(int core, int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag && cores[core].cache[setIndex][i].state != CacheState::INVALID)
            {
                return true;
            }
        }
    }

    void runSnoop(int core)
    {
        switch (state)
        {
        case BusState::INVALID:
            if (core != source)
            {
                invalidateAddress(core, address);
            }
            break;
        case BusState::RM:
            if (core != source && owners.top() <= 3 && containsAddress(core, address))
            {
                owners.push(4 + core);
                state = BusState::TRANSACTION;
                source = core;
                remainingCycles = 2 * (1 << b);
            }
            shareAddress(core, address);
            break;
        case BusState::RWITM:
            if (core != source && owners.top() <= 3 && getCacheState(core, address) == CacheState::MODIFIED)
            {
                owners.push(4 + core);
                state = BusState::TRANSACTION;
                source = core;
                destination = 4;
                remainingCycles = 100;
            }
            invalidateAddress(core, address);
            break;

        default:
            break;
        }
    }

    void addToCache(int core)
    {
        unsigned int setIndex = (address >> b) & ((1 << s) - 1);
        unsigned int tag = address >> (s + b);

        CacheState cacheState = (source == 4 ? CacheState::EXCLUSIVE : CacheState::SHARED);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
            {
                cores[core].cache[setIndex][i].state = cacheState;
                cores[core].cacheLRU[setIndex][i] = cycle;
                return;
            }
        }

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].state == CacheState::INVALID)
            {
                cores[core].cache[setIndex][i].state = cacheState;
                cores[core].cacheLRU[setIndex][i] = cycle;
                return;
            }
        }

        cout << "Core did not have a corresponding line or a invalid line" << endl;
        assert(false);
    }

    void processTransaction()
    {
        remainingCycles--;

        switch (state)
        {
        case BusState::RM:
        case BusState::RWITM:
            state = BusState::TRANSACTION;
            remainingCycles = MEMCYCLES;
            source = 4;
            break;
        case BusState::INVALID:
            state = BusState::IDLE;
            break;
        case BusState::TRANSACTION:
            if (remainingCycles == 0)
            {
                if (destination < 4)
                    addToCache(destination);
                if (owners.top() >= 4 && owners.top() <= 7)
                    owners.pop();
                state = BusState::INVALID;
            }
            break;
        default:
            break;
        }
    }

    bool checkCompletion()
    {
        return (completed[0] == 1 &&
                completed[1] == 1 &&
                completed[2] == 1 &&
                completed[3] == 1);
    }

    bool runCycle()
    {
        if (checkCompletion())
            return true;

        processTransaction();

        
        int tempOwner;

        if (owners.empty()) {
            tempOwner = 4;
        } else {
            tempOwner = owners.top();
        }

        if (tempOwner <= 3) {
            runCore(tempOwner);
        }

        for (int i = 0; i < 4; i++)
        {
            if (i != tempOwner)
                runCore(i);
        }

        for (int i = 0; i < 4; i++)
        {
            runSnoop(i);
        }

        return false;
    }

public:
    Bus(int s, int E, int b, string appname) : state(BusState::IDLE), address(0), s(s), E(E), b(b)
    {
        Traces traces = parse_traces(appname);
        cores[0] = Core(s, E, b, traces.trace1);
        cores[1] = Core(s, E, b, traces.trace2);
        cores[2] = Core(s, E, b, traces.trace3);
        cores[3] = Core(s, E, b, traces.trace4);

        cout << cores[0].instructions[0].first << endl;

        cycle = 0;

        for (int i = 0; i < 4; i++)
        {
            completed[i] = 0;
        }
    }

    void runApplication()
    {
        cout <<  "RUNNING" << endl;
        while (!runCycle())
        {
            cycle += 1;
            cout << cycle << endl;
        }
        
    }
};

int main()
{
    int s = 6;
    int E = 2;
    int b = 5;

    string appname = "app1";

    Bus bus = Bus(s, E, b, appname);

    bus.runApplication();
}
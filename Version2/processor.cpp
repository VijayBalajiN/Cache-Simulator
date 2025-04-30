#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
#include <stack>
#include "parser.h"
#include <fstream>
#include <iomanip>

#define MEMCYCLES 99

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
    int needsBus;

    int tot_instruc;
    int tot_reads;
    int tot_writes;
    int tot_exe_cycles;  // to be done
    int idle_cycles;     // to be done
    int cache_misses;
    int cache_hits;
    int cache_evic;
    int writebacks;
    int bus_invalid;
    int data_traff;

    int currLine;

    char currentInstruction;
    int currentAddress;

    vector<vector<CacheLine>> cache;
    vector<vector<int>> cacheLRU;
    vector<pair<char, unsigned int>> instructions;

    Core(int s, int E, int b, const vector<pair<char, unsigned int>> instr)
        : line(0), core(0), currentInstruction(0), currentAddress(0), instructions(instr), needsBus(0), tot_reads(0),
          tot_exe_cycles(0), idle_cycles(0), cache_misses(0), cache_evic(0), writebacks(0), bus_invalid(0), data_traff(0), currLine(-1), cache_hits(0)
    {
        int sets = 1 << s;
        tot_instruc = instructions.size();
        cache.resize(sets, vector<CacheLine>(E));
        cacheLRU.resize(sets, vector<int>(E, 0)); //Array of size E
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

    void print_core(ofstream& outfile, int tot_instruc, int tot_reads, int tot_writes,
        int tot_exe_cycles, int idle_cycles, int cache_miss, double cache_miss_rate,
        int cache_evic, int writebacks, int bus_invalid, int data_traff) {
        outfile<< "Total Instructions: " << tot_instruc << "\n";
        outfile<< "Total Reads: " << tot_reads << "\n";
        outfile<< "Total Writes: " << tot_writes << "\n";
        outfile<< "Total Execution Cycles: " << tot_exe_cycles << "\n";
        outfile<< "Idle Cycles: " << idle_cycles << "\n";
        outfile<< "Cache Misses: " << cache_miss << "\n";
        outfile<< "Cache Miss Rate: " << fixed << setprecision(3) << cache_miss_rate << "%\n";
        outfile<< "Cache Evictions: " << cache_evic << "\n";
        outfile<< "Writebacks: " << writebacks << "\n";
        outfile<< "Bus Invalidations: " << bus_invalid << "\n";
        outfile<< "Data Traffic (Bytes): " << data_traff << "\n";
    }

    void print_output(string trace_pref, string output_file) {

        ofstream outfile(output_file);
        if (!outfile.is_open()){
        cerr << "Error: Could not open file " << output_file << " for writing\n";
        return;
        }
        // Calculate cache miss rates
        double core0_cache_miss_rate = (double)cores[0].cache_misses / cores[0].tot_instruc * 100.0;
        double core1_cache_miss_rate = (double)cores[1].cache_misses / cores[1].tot_instruc * 100.0;
        double core2_cache_miss_rate = (double)cores[2].cache_misses / cores[2].tot_instruc * 100.0;
        double core3_cache_miss_rate = (double)cores[3].cache_misses / cores[3].tot_instruc * 100.0;

        // Calculate total bus traffic
        int total_bus_traffic = cores[0].data_traff + cores[1].data_traff + cores[2].data_traff + cores[3].data_traff;

        int block_size = 1 << b;
        int num_sets = 1 << s;
        int cache_size_kb = (num_sets * E * block_size) / 1024;

        outfile<< "Simulation Parameters:\n";
        outfile<< "Trace Prefix: " << trace_pref << "\n";
        outfile<< "Set Index Bits: " << s << "\n";
        outfile<< "Associativity: " << E << "\n";
        outfile<< "Block Bits: " << b << "\n";
        outfile<< "Block Size (Bytes): " << block_size << "\n";
        outfile<< "Number of Sets: " << num_sets << "\n";
        outfile<< "Cache Size (KB per core): " << cache_size_kb << "\n";
        outfile<< "MESI Protocol: Enabled\n"; 
        outfile<< "Write Policy: Write-back, Write-allocate\n";
        outfile<< "Replacement Policy: LRU\n";
        outfile<< "Bus: Central snooping bus\n\n";

        outfile<< "Core 0 Statistics:\n";
        print_core(outfile, cores[0].tot_instruc, cores[0].tot_reads, cores[0].tot_writes,
            cores[0].tot_exe_cycles, cores[0].idle_cycles, cores[0].cache_misses, core0_cache_miss_rate,
            cores[0].cache_evic, cores[0].writebacks, cores[0].bus_invalid, cores[0].data_traff);
        outfile<< "\n";

        outfile<< "Core 1 Statistics:\n";
        print_core(outfile, cores[1].tot_instruc, cores[1].tot_reads, cores[1].tot_writes,
            cores[1].tot_exe_cycles, cores[1].idle_cycles, cores[1].cache_misses, core1_cache_miss_rate,
            cores[1].cache_evic, cores[1].writebacks, cores[1].bus_invalid, cores[1].data_traff);
        outfile<< "\n";

        outfile<< "Core 2 Statistics:\n";
        print_core(outfile, cores[2].tot_instruc, cores[2].tot_reads, cores[2].tot_writes,
            cores[2].tot_exe_cycles, cores[2].idle_cycles, cores[2].cache_misses, core2_cache_miss_rate,
            cores[2].cache_evic, cores[2].writebacks, cores[2].bus_invalid, cores[2].data_traff);
        outfile<< "\n";

        outfile<< "Core 3 Statistics:\n";
        print_core(outfile, cores[3].tot_instruc, cores[3].tot_reads, cores[3].tot_writes,
         cores[3].tot_exe_cycles, cores[3].idle_cycles, cores[3].cache_misses, core3_cache_miss_rate,
         cores[3].cache_evic, cores[3].writebacks, cores[3].bus_invalid, cores[3].data_traff);
        outfile<< "\n";



        outfile<< "Overall Bus Summary:\n";
        // outfile<< "Total Bus Transactions: " << total_bus_transactions << "\n";
        outfile<< "Total Bus Traffic (Bytes): " << total_bus_traffic << "\n";
    }

    void runCore(int core)
    {
        unsigned int setIndex = (address >> b) & ((1 << s) - 1);
        unsigned int tag = address >> (s + b);

        int line = cores[core].line;

        

        if (cores[core].needsBus == 1 && state != BusState::IDLE && (owners.top() == core || destination == core) && cores[core].line < cores[core].tot_instruc) {
            if (cores[core].currLine != line) {
                
                assert(false);
            }
            cores[core].tot_exe_cycles += 1;
            return;
        }
        else if (cores[core].needsBus == 1 && state != BusState::IDLE && owners.top() != core && cores[core].line < cores[core].tot_instruc){
            if (cores[core].currLine != line) {
                cout << "If currLine is not equal to line, then this is the first time this instruction is processed" << endl;
                assert(false);
            }
            cores[core].idle_cycles += 1;
            return;
        }

        
        int size = cores[core].instructions.size();   // not used

        if (line < cores[core].instructions.size())
        {
            cores[core].tot_exe_cycles += 1;
            char currentInstruction = cores[core].instructions[line].first;
            unsigned int currentAddress = cores[core].instructions[line].second;

            CacheState cacheState = getCacheState(core, currentAddress);

            if (currentInstruction == 'R')
            {
                
                switch (cacheState)
                {
                case CacheState::SHARED:
                    
                case CacheState::EXCLUSIVE:
                case CacheState::MODIFIED:
                    if (!owners.empty() && owners.top() == core)
                        owners.pop();
                    cores[core].line++;
                    cores[core].needsBus = 0;
                    cores[core].tot_reads += 1;
                    updateLRU(core, currentAddress);
                    if (cores[core].currLine != line) cores[core].cache_hits += 1;
                    break;
                case CacheState::INVALID:
                    if (cores[core].currLine != line) cores[core].cache_misses += 1;
                    
                    cores[core].needsBus = 1;
                    
                    if (owners.empty())
                    {
                        owners.push(core);
                    }

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        
                        cores[core].data_traff += (1 << b);
                        CacheState lruState = getInvalidCacheState(core, currentAddress);

                        switch (lruState)
                        {
                        case CacheState::INVALID:
                            state = BusState::RM;
                            destination = core;
                            address = currentAddress;
                            
                            break;
                        case CacheState::MODIFIED:
                            cores[core].writebacks += 1;
                            cores[core].data_traff += (1 << b);
                            state = BusState::TRANSACTION;
                            remainingCycles = MEMCYCLES;
                            source = core;
                            destination = 4;
                            
                            break;
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
                    cores[core].needsBus = 0;
                    cores[core].tot_writes += 1;
                    updateLRU(core, currentAddress);
                    if (cores[core].currLine != line) cores[core].cache_hits += 1;
                    break;

                    
                }
                case CacheState::SHARED:
                {
                    
                    if (cores[core].currLine != line) cores[core].cache_hits += 1;
                    
                    cores[core].needsBus = 1;
                    
                    if (owners.empty())
                    {
                        owners.push(core);
                    } 

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        cores[core].bus_invalid += 1;
                        CacheState _ = getWriteCacheState(core, currentAddress);
                        updateLRU(core, currentAddress);
                        state = BusState::INVALID;
                        address = currentAddress;
                        source = core;
                        cores[core].line++;
                        cores[core].needsBus = 0;
                        cores[core].tot_writes += 1;
                        updateLRU(core, currentAddress);
                        if (cores[core].currLine != line) cores[core].cache_hits += 1;
                    }
                    break;
                }
                case CacheState::INVALID:
                    if (cores[core].currLine != line) cores[core].cache_misses += 1;
                    
                    cores[core].needsBus = 1;
                
                    if (owners.empty())
                    {
                        owners.push(core);
                    }

                    if (!owners.empty() && owners.top() == core && state == BusState::IDLE)
                    {
                        
                        cores[core].data_traff += (1 << b);
                        CacheState lruState = getInvalidCacheState(core, currentAddress); // Invalidate corresponding LRU State

                        switch (lruState)
                        {
                        case CacheState::INVALID:
                            
                            // cores[core].bus_invalid += 1;
                            state = BusState::RWITM;
                            destination = core;
                            address = currentAddress;
                            
                            break;
                        case CacheState::MODIFIED:
                            cores[core].writebacks += 1;
                            cores[core].data_traff += (1 << b);
                            state = BusState::TRANSACTION;
                            remainingCycles = MEMCYCLES;
                            source = core;
                            destination = 4;
                            break;
                        default:
                            cout << "This state is not reachable logically!" << endl;
                            assert(false);
                            break;
                        }
                    }
                }
            }
            cores[core].currLine = line;
        }
        else
        {
            completed[core] = 1;
            
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

    void updateLRU(int core, unsigned int currentAddress) {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag
                && cores[core].cache[setIndex][i].state != CacheState::INVALID)
            {
                cores[core].cacheLRU[setIndex][i] = cycle;
            }
        }
    }

    CacheState getCacheState(int core, unsigned int currentAddress)
    {
        unsigned int setIndex = (currentAddress >> b) & ((1 << s) - 1);
        unsigned int tag = currentAddress >> (s + b);

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag
                && cores[core].cache[setIndex][i].state != CacheState::INVALID)
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

        cores[core].cache_evic += 1;

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
            if (cores[core].cache[setIndex][i].tag == tag)   // is it possible that tags match but it is in invalid?
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
        return false;
    }

    void runSnoop(int core)
    {
        switch (state)
        {
        case BusState::INVALID:
            if (core != owners.top())
            {
                
                
                invalidateAddress(core, address);
            }
            break;
        case BusState::RM:
            
            
            if (core != owners.top() && owners.top() <= 3 && containsAddress(core, address))
            {
                
                
                if (getCacheState(core, address) == CacheState::MODIFIED) {
                    owners.push(4 + core);
                    state = BusState::TRANSACTION;
                    remainingCycles = MEMCYCLES;
                    source = core;
                    destination = 4;
                    cores[core].data_traff += (1 << b);
                } else {
                    owners.push(4 + core);
                    cores[core].data_traff += (1 << b);
                    state = BusState::TRANSACTION;
                    source = core;
                    remainingCycles = 2 * (1 << b) / 4 - 1;
                }
            }
            if (core != owners.top()) {
                
                
                shareAddress(core, address);
            }
            break;
        case BusState::RWITM:
            if (core != owners.top() && owners.top() <= 3 && getCacheState(core, address) == CacheState::MODIFIED)
            {
                
                owners.push(4 + core);
                cores[core].data_traff += (1 << b);
                state = BusState::TRANSACTION;
                source = core;
                destination = 4;
                remainingCycles = MEMCYCLES;
            }
            if (core != owners.top()) {
                
                
                invalidateAddress(core, address);
            }
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

        if (source == 5) {
            cacheState = CacheState::MODIFIED;
        }

        for (int i = 0; i < E; ++i)
        {
            if (cores[core].cache[setIndex][i].tag == tag)
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
            state = BusState::TRANSACTION;
            remainingCycles = MEMCYCLES;
            source = 4;
            break;
        case BusState::RWITM:
            state = BusState::TRANSACTION;
            remainingCycles = MEMCYCLES;
            source = 5;
            break;
        case BusState::INVALID:
            state = BusState::IDLE;
            owners.pop();
            break;
        case BusState::TRANSACTION:
            if (remainingCycles == 0)
            {
                if (destination < 4)
                    addToCache(destination);
                if (owners.top() >= 4 && owners.top() <= 7)
                    owners.pop();
                state = BusState::IDLE;
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
        }

        
        
    }

    void printCore(string outputfilename, string appname) {
        // ofstream outfile(output_file);
        // if (!outfile.is_open()){
        //     cerr << "Error: Could not open file " << output_file << " for writing\n";
        //     return;
        // }
        print_output(appname, outputfilename);

    }
};

void print_help(){
    cout << "Usage: ./L1simulate  [options]\n";
    cout << "Simulate L1 cache for quad core processors, with cache coherence support.\n";
    cout << "Options:\n";
    cout << "\t-t <tracefile>   name of parallel application (e.g. app1) whose 4 traces are to be used in the simulation\n";
    cout << "\t-s <s>           number of set inddex bits (number of sets in the cache = S = 2 ^ s)\n";
    cout << "\t-E <E>           associativity (number of cache lines per set)\n";
    cout << "\t-b <b>           number of block bits (block size = B = 2^b)\n";
    cout << "\t-o <outfilename> logs output in file for plotting etc.\n";
    cout << "\t-h               prints this help\n";
}

int main(int argc, char *argv[])
{
    int i = 1;
    string tracefile = "app1";
    int s = 6;
    int E = 2;
    int b = 5;
    string output_file = "output.txt";
    while (i < argc){
        if (string(argv[i]) == "-h"){
            print_help();
            return 0;
        }
        else if (string(argv[i]) == "-t"){
            tracefile = argv[i+1];
            i+=2;
        }
        else if (string(argv[i]) == "-s"){
            s = stoi(argv[i+1]);
            i+=2;
        }
        else if (string(argv[i]) == "-b"){
            b = stoi(argv[i+1]);
            i+=2;
        }
        else if (string(argv[i]) == "-E"){
            E = stoi(argv[i+1]);
            i+=2;
        }
        else if (string(argv[i]) == "-o"){
            output_file = argv[i+1];
            i+=2;
        }
        else{
            cout << "Error in the arguments. Expected a option. Found:" << argv[i] << "\n";
            return 0;
        }
    }

    string appname = tracefile;
    string outputfilename = output_file;

    Bus bus = Bus(s, E, b, appname);

    bus.runApplication();

    bus.printCore(outputfilename, appname);
}
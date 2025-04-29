#include "processor.h"
#include "parser.h"
#include <iostream>
#include <vector>
#include <string>

#define MEMCYCLES 100
#define CACHECYCLES 2

using namespace std;

enum class BusState {
    IDLE,
    RM,
    IV,
    RWITM,
    WB,
    CORE_COMMIT,
    CORE_TRANSFER,
    MEM_TRANSFER,
};

enum class MemoryCache {
    CORE0 = 0,
    CORE1 = 1,
    CORE2 = 2,
    CORE3 = 3,
    Memory = 4,
    NONE = 5
};


class Bus {
    private:
        BusState state;
        MemoryCache owner;

        int address;
        int transactionCycles;
        MemoryCache dataFrom;
        MemoryCache dataTo;

        int s;
        int E;
        int b;

        bool isWaitingForCore;

        Processor *processors[4];

        int transactionRequest[4];

        void updateTransactionOwnerState () {
            processors[(int)owner]->completeTransaction();
        }

        void cleanRequests() {
            for (int i = 0; i < 4; i++) {
                transactionRequest[i] = 0;
            }
        }

        void updateBusState() {
            switch (state) {
                case BusState::IDLE:
                    break;
                case BusState::RM:
                    state = BusState::MEM_TRANSFER;
                    transactionCycles = MEMCYCLES;
                    break;
                case BusState::IV:
                    state = BusState::IDLE;
                    break;
                case BusState::RWITM:
                    state = BusState::MEM_TRANSFER;
                    transactionCycles = MEMCYCLES;
                    break;
                case BusState::WB:
                    if (transactionCycles == 0) {
                        state = BusState::IDLE;
                        updateTransactionOwnerState();
                    }
                    else {
                        transactionCycles--;
                    }
                    break;
                case BusState::CORE_COMMIT:
                    state = BusState::CORE_TRANSFER;
                    break;
                case BusState::CORE_TRANSFER:
                    if (transactionCycles == 0) {
                        state = BusState::IDLE;
                        updateTransactionOwnerState();
                    }
                    else {
                        transactionCycles--;
                    }
                    break;
                case BusState::MEM_TRANSFER:
                    if (transactionCycles == 0) {
                        state = BusState::IDLE;
                        updateTransactionOwnerState();
                    }
                    else {
                        transactionCycles--;
                    }
                    break;
                
                default:
                    break;
            }
        }

        void runCycle() {
            cleanRequests();
            updateBusState();
            if (! processors[(int)owner]->requestOwnership()) {
                owner = MemoryCache::NONE;
            }

            for (int i = 0; i < 4; i++) {
                
            }
        }

    public:
        Bus(int s, int E, int b, string appname) : 
        state(BusState::IDLE), owner(MemoryCache::NONE), address(0), s(s), E(E), b(b), dataFrom(MemoryCache::NONE), dataTo(MemoryCache::NONE), isWaitingForCore(false) {
            Traces traces = parse_traces(appname);
            processors[0] = &Processor(s, E, b, 0, traces.trace1);
            processors[1] = &Processor(s, E, b, 1, traces.trace2);
            processors[2] = &Processor(s, E, b, 2, traces.trace3);
            processors[3] = &Processor(s, E, b, 3, traces.trace4);      
        }

        
};
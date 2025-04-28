// #include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <vector>
#include "parser.h"

using namespace std;

void print_help();

int main(int argc, char* argv[]){
    int i = 1;
    string tracefile = "app1";
    int index_bits = 3;
    int associativity = 1;
    int block_bits = 64;
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
            index_bits = stoi(argv[i+1]);
            i+=2;
        }
        else if (string(argv[i]) == "-b"){
            block_bits = stoi(argv[i+1]);
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
    vector<pair<char, int>> trace1, trace2, trace3, trace4;
    Traces traces = parse_traces(tracefile);
    trace1 = traces.trace1;
    trace2 = traces.trace2;
    trace3 = traces.trace3;
    trace4 = traces.trace4;
    // for (auto p : trace2){
    //     cout << p.first << " " << p.second << "\n";
    // }
}

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
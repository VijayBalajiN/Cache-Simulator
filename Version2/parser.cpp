// #include <bits/stdc++.h>
#include <string>
#include <vector>
#include "parser.h"
#include <fstream>
#include <sstream>

using namespace std;

vector<pair<char, int>> parse_trace(string trace_file_name);


Traces parse_traces(string trace_name){
    vector<pair<char, int>> trace1, trace2, trace3, trace4;
    string file1 = trace_name + "_proc0.trace";
    string file2 = trace_name + "_proc1.trace";
    string file3 = trace_name + "_proc2.trace";
    string file4 = trace_name + "_proc3.trace";
    Traces trace;
    trace.trace1 = parse_trace(file1);
    trace.trace2 = parse_trace(file2);
    trace.trace3 = parse_trace(file3);
    trace.trace4 = parse_trace(file4);
    return trace;
}

vector<pair<char, int>> parse_trace(string trace_file_name){
    ifstream trace(trace_file_name);
    string line;
    vector<pair<char, int>> trace_i;
    if (trace.is_open()){
        while (getline(trace, line)){
            char instruction;
            int address;
            stringstream ss(line);
            string ins;
            getline(ss, ins, ' ');
            string add;
            getline(ss, add); 
            instruction = ins[0];
            address = stoi(add, nullptr, 0);
            trace_i.push_back(make_pair(instruction, address));

        }
        trace.close();
    } else {
        cerr << "Unable to open file: " << trace_file_name << "\n";
    }
    return trace_i;
}
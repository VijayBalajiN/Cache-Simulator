// #include "bits/stdc++.h" 
#include <vector>
#include <iostream>
#include <string>

using namespace std;

typedef struct Traces{
    vector<pair<char, unsigned int> > trace1, trace2, trace3, trace4;
} Traces;

Traces parse_traces(string trace);


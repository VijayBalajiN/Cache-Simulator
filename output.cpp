#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> // For setting precision

using namespace std;

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

void print_output(string trace_pref, int set_index_bits, int associativity, int block_bits,
                  int core0_tot_instruc, int core0_tot_reads, int core0_tot_writes,
                  int core0_tot_exe_cycles, int core0_idle_cycles, int core0_cache_miss,
                  int core0_cache_evic, int core0_writebacks, int core0_bus_invalid, int core0_data_traff,
                  int core1_tot_instruc, int core1_tot_reads, int core1_tot_writes,
                  int core1_tot_exe_cycles, int core1_idle_cycles, int core1_cache_miss,
                  int core1_cache_evic, int core1_writebacks, int core1_bus_invalid, int core1_data_traff,
                  int core2_tot_instruc, int core2_tot_reads, int core2_tot_writes,
                  int core2_tot_exe_cycles, int core2_idle_cycles, int core2_cache_miss,
                  int core2_cache_evic, int core2_writebacks, int core2_bus_invalid, int core2_data_traff,
                  int core3_tot_instruc, int core3_tot_reads, int core3_tot_writes,
                  int core3_tot_exe_cycles, int core3_idle_cycles, int core3_cache_miss,
                  int core3_cache_evic, int core3_writebacks, int core3_bus_invalid, int core3_data_traff,
                  int total_bus_transactions, string output_file) {
    
    ofstream outfile(output_file);
    if (!outfile.is_open()){
        cerr << "Error: Could not open file " << output_file << " for writing\n";
        return;
    }
    // Calculate cache miss rates
    double core0_cache_miss_rate = (double)core0_cache_miss / core0_tot_instruc * 100.0;
    double core1_cache_miss_rate = (double)core1_cache_miss / core1_tot_instruc * 100.0;
    double core2_cache_miss_rate = (double)core2_cache_miss / core2_tot_instruc * 100.0;
    double core3_cache_miss_rate = (double)core3_cache_miss / core3_tot_instruc * 100.0;

    // Calculate total bus traffic
    int total_bus_traffic = core0_data_traff + core1_data_traff + core2_data_traff + core3_data_traff;
    
    int block_size = 1 << block_bits;
    int num_sets = 1 << set_index_bits;
    int cache_size_kb = (num_sets * associativity * block_size) / 1024;

    outfile<< "Simulation Parameters:\n";
    outfile<< "Trace Prefix: " << trace_pref << "\n";
    outfile<< "Set Index Bits: " << set_index_bits << "\n";
    outfile<< "Associativity: " << associativity << "\n";
    outfile<< "Block Bits: " << block_bits << "\n";
    outfile<< "Block Size (Bytes): " << block_size << "\n";
    outfile<< "Number of Sets: " << num_sets << "\n";
    outfile<< "Cache Size (KB per core): " << cache_size_kb << "\n";
    outfile<< "MESI Protocol: Enabled\n"; 
    outfile<< "Write Policy: Write-back, Write-allocate\n";
    outfile<< "Replacement Policy: LRU\n";
    outfile<< "Bus: Central snooping bus\n\n";

    outfile<< "Core 0 Statistics:\n";
    print_core(outfile, core0_tot_instruc, core0_tot_reads, core0_tot_writes,
               core0_tot_exe_cycles, core0_idle_cycles, core0_cache_miss, core0_cache_miss_rate,
               core0_cache_evic, core0_writebacks, core0_bus_invalid, core0_data_traff);
    outfile<< "\n";

    outfile<< "Core 1 Statistics:\n";
    print_core(outfile, core1_tot_instruc, core1_tot_reads, core1_tot_writes,
               core1_tot_exe_cycles, core1_idle_cycles, core1_cache_miss, core1_cache_miss_rate,
               core1_cache_evic, core1_writebacks, core1_bus_invalid, core1_data_traff);
    outfile<< "\n";

    outfile<< "Core 2 Statistics:\n";
    print_core(outfile, core2_tot_instruc, core2_tot_reads, core2_tot_writes,
               core2_tot_exe_cycles, core2_idle_cycles, core2_cache_miss, core2_cache_miss_rate,
               core2_cache_evic, core2_writebacks, core2_bus_invalid, core2_data_traff);
    outfile<< "\n";

    outfile<< "Core 3 Statistics:\n";
    print_core(outfile, core3_tot_instruc, core3_tot_reads, core3_tot_writes,
               core3_tot_exe_cycles, core3_idle_cycles, core3_cache_miss, core3_cache_miss_rate,
               core3_cache_evic, core3_writebacks, core3_bus_invalid, core3_data_traff);
    outfile<< "\n";

    

    outfile<< "Overall Bus Summary:\n";
    outfile<< "Total Bus Transactions: " << total_bus_transactions << "\n";
    outfile<< "Total Bus Traffic (Bytes): " << total_bus_traffic << "\n";
}

// Test main function to verify output formatting
int main() {
    // Parameters from output.txt
    string trace_prefix = "app1";
    int set_index_bits = 5;
    int associativity = 2;
    int block_bits = 5;
    
    // Core 0 statistics from output.txt
    int core0_tot_instruc = 1450;
    int core0_tot_reads = 820;
    int core0_tot_writes = 630;
    int core0_tot_exe_cycles = 3920;
    int core0_idle_cycles = 2100;
    int core0_cache_miss = 370;
    int core0_cache_evic = 38;
    int core0_writebacks = 15;
    int core0_bus_invalid = 51;
    int core0_data_traff = 2680;
    
    // Core 1 statistics from output.txt
    int core1_tot_instruc = 1403;
    int core1_tot_reads = 789;
    int core1_tot_writes = 614;
    int core1_tot_exe_cycles = 3860;
    int core1_idle_cycles = 2050;
    int core1_cache_miss = 355;
    int core1_cache_evic = 36;
    int core1_writebacks = 13;
    int core1_bus_invalid = 48;
    int core1_data_traff = 2540;
    
    // Core 2 statistics from output.txt
    int core2_tot_instruc = 1385;
    int core2_tot_reads = 765;
    int core2_tot_writes = 620;
    int core2_tot_exe_cycles = 3785;
    int core2_idle_cycles = 1970;
    int core2_cache_miss = 340;
    int core2_cache_evic = 33;
    int core2_writebacks = 14;
    int core2_bus_invalid = 46;
    int core2_data_traff = 2430;
    
    // Core 3 statistics from output.txt
    int core3_tot_instruc = 1422;
    int core3_tot_reads = 800;
    int core3_tot_writes = 622;
    int core3_tot_exe_cycles = 3905;
    int core3_idle_cycles = 2085;
    int core3_cache_miss = 365;
    int core3_cache_evic = 39;
    int core3_writebacks = 16;
    int core3_bus_invalid = 50;
    int core3_data_traff = 2610;

    // Overall Statistics
    int total_bus_transactions = 175;

    // Output file to be written
    string output_file = "output1.txt";
    
    // Call print_output with all parameters
    print_output(trace_prefix, set_index_bits, associativity, block_bits,
                 core0_tot_instruc, core0_tot_reads, core0_tot_writes,
                 core0_tot_exe_cycles, core0_idle_cycles, core0_cache_miss,
                 core0_cache_evic, core0_writebacks, core0_bus_invalid, core0_data_traff,
                 core1_tot_instruc, core1_tot_reads, core1_tot_writes,
                 core1_tot_exe_cycles, core1_idle_cycles, core1_cache_miss,
                 core1_cache_evic, core1_writebacks, core1_bus_invalid, core1_data_traff,
                 core2_tot_instruc, core2_tot_reads, core2_tot_writes,
                 core2_tot_exe_cycles, core2_idle_cycles, core2_cache_miss,
                 core2_cache_evic, core2_writebacks, core2_bus_invalid, core2_data_traff,
                 core3_tot_instruc, core3_tot_reads, core3_tot_writes,
                 core3_tot_exe_cycles, core3_idle_cycles, core3_cache_miss,
                 core3_cache_evic, core3_writebacks, core3_bus_invalid, core3_data_traff, 
                 total_bus_transactions, output_file);
    
    return 0;
}
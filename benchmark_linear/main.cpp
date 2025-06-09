#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <fstream>
#include "IOUtils.hpp"

using namespace std;
using namespace std::chrono;

vector<long long> brute_force_locate(const string& reference, const string& pattern, int max_err) {
    vector<long long> pos;
    long long n = reference.size();
    long long m = pattern.size();
    if (m == 0 || n < m) {
        return pos;
    }
    for (long long i = 0; i + m <= n; i++) {
        int err = 0;
        for (long long j = 0; j < m && err <= max_err; j++) {
            if (reference[i + j] != pattern[j]) {
                err++;
            }
        }
        if (err <= max_err) {
            pos.push_back(i);
        }
    }
    return pos;
}

string assemble_reads(const string& reference, const vector<string>& reads, int max_err) {
    auto t_map_s = high_resolution_clock::now();
    vector<vector<long long>> positions;
    positions.reserve(reads.size());
    for (const string& read : reads) {
        positions.push_back(brute_force_locate(reference, read, max_err));
    }
    auto t_map_e = high_resolution_clock::now();
    long long map_ms = duration_cast<milliseconds>(t_map_e - t_map_s).count();

    auto t_asm_s = high_resolution_clock::now();
    string assembled(reference.size(), 'N');
    for (size_t i = 0; i < reads.size(); i++) {
        const string& read = reads[i];
        for (long long p : positions[i]) {
            if (p >= 0 && p + read.size() <= reference.size()) {
                for (size_t j = 0; j < read.size(); j++) {
                    assembled[p + j] = read[j];
                }
            }
        }
    }
    auto t_asm_e = high_resolution_clock::now();
    long long asm_ms = duration_cast<milliseconds>(t_asm_e - t_asm_s).count();

    long long total_ms = map_ms + asm_ms;
    ofstream tfs("linear_timing.txt");
    if (!tfs) {
        throw runtime_error("Failed to open timing file.");
    }
    if (tfs) {
        tfs << "Read mapping time       : " << map_ms << " ms\n";
        tfs << "Assembly time           : " << asm_ms << " ms\n";
        tfs << "Total pipeline time     : " << total_ms << " ms\n";
    }
    return assembled;
}

int main() {
    const string ref_path  = "reference.txt";
    const string read_path = "reads.txt";
    const string out_path  = "linear_assembled.txt";

    int max_err = 0;
    cout << "Enter max mismatch (D): ";
    if (!(cin >> max_err) || max_err < 0) {
        cerr << "Invalid integer.\n";
        return 1;
    }

    try {
        const string reference     = io::read_reference(ref_path);
        const vector<string> reads = io::read_reads(read_path);
        string assembled           = assemble_reads(reference, reads, max_err);
        io::write_text(out_path, assembled);
        cout << "Assembly finished. Output: " << out_path << '\n';
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

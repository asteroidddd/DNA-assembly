#ifndef ASSEMBLE_HPP
#define ASSEMBLE_HPP

#include <chrono>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <algorithm>
#include "IOUtils.hpp"
#include "FMIndex.hpp"

using namespace std;
using namespace chrono;

// 어셈블 함수
inline string assemble_reads(const string& reference, const vector<string>& reads, int max_err) {
    size_t ref_len  = reference.size();
    size_t read_cnt = reads.size();

    // FM-index 구축
    auto t_build_start = high_resolution_clock::now();
    FMIndex fm(reference);
    auto t_build_end   = high_resolution_clock::now();
    long long build_ms = duration_cast<milliseconds>(t_build_end - t_build_start).count();

    // 리드 매핑
    auto t_map_start = high_resolution_clock::now();
    vector<vector<size_t>> positions(read_cnt);
    for (size_t i = 0; i < read_cnt; ++i) {
        positions[i] = fm.locate(reads[i], max_err);
    }
    auto t_map_end = high_resolution_clock::now();
    long long map_ms = duration_cast<milliseconds>(t_map_end - t_map_start).count();

    // 다수결 어셈블
    vector<array<int, 256>> vote(ref_len);
    for (size_t i = 0; i < read_cnt; ++i) {
        const auto& read = reads[i];
        for (size_t pos : positions[i]) {
            // 위치가 레퍼런스 범위를 벗어나면 건너뜀
            if (pos + read.size() > ref_len) {
                continue;
            }
            for (size_t j = 0; j < read.size(); ++j) {
                unsigned char c = static_cast<unsigned char>(read[j]);
                vote[pos + j][c]++;
            }
        }
    }

    string assembled(ref_len, 'N');
    for (size_t i = 0; i < ref_len; ++i) {
        auto& counts = vote[i];
        auto it = max_element(counts.begin(), counts.end());
        if (*it > 0) {
            assembled[i] = static_cast<char>(distance(counts.begin(), it));
        }
    }

    auto t_asm_end = high_resolution_clock::now();
    long long asm_ms = duration_cast<milliseconds>(t_asm_end - t_map_end).count();

    // 타이밍 로그
    long long total_ms = build_ms + map_ms + asm_ms;
    ofstream tfs("cfmindex_timing.txt");
    if (tfs) {
        tfs << "FM-index build time     : " << build_ms  << " ms\n";
        tfs << "Read mapping time       : " << map_ms    << " ms\n";
        tfs << "Consensus assembly time : " << asm_ms    << " ms\n";
        tfs << "Total pipeline time     : " << total_ms  << " ms\n";
    }

    return assembled;
}

#endif // ASSEMBLE_HPP

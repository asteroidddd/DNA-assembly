#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <array>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <fstream>
#include "IOUtils.hpp"

using namespace std;
using namespace std::chrono;

constexpr uint8_t SENT = 0;

class FM_index {
    long long n;
    vector<long long> sa;
    vector<uint8_t> bwt;
    array<int, 256> C{};
    vector<array<int, 256>> occ;

    void build_SA(const vector<uint8_t>& text) {
        sa.resize(n);
        iota(sa.begin(), sa.end(), 0LL);
        sort(sa.begin(), sa.end(), [&](long long a, long long b) {
            while (a < n && b < n) {
                if (text[a] != text[b]) {
                    return text[a] < text[b];
                }
                a++;
                b++;
            }
            return a == n;
        });
    }

    void build_BWT(const vector<uint8_t>& text) {
        bwt.resize(n);
        for (long long i = 0; i < n; i++) {
            bwt[i] = text[(sa[i] + n - 1) % n];
        }
    }

    void build_C() {
        array<int, 256> freq{};
        for (const auto ch : bwt) {
            freq[ch]++;
        }
        int sum = 0;
        for (int c = 0; c < 256; c++) {
            if (freq[c] == 0) {
                C[c] = -1;
            } else {
                C[c] = sum;
                sum += freq[c];
            }
        }
    }

    void build_occ() {
        occ.resize(n + 1);
        occ[0].fill(0);
        for (long long i = 0; i < n; i++) {
            occ[i + 1] = occ[i];
            occ[i + 1][bwt[i]]++;
        }
    }

public:
    FM_index(const vector<uint8_t>& text_in) {
        vector<uint8_t> text = text_in;
        text.push_back(SENT);
        n = static_cast<long long>(text.size());
        build_SA(text);
        build_BWT(text);
        build_C();
        build_occ();
    }

    vector<long long> locate(const vector<uint8_t>& pat, int max_err) const {
        struct Node { int idx; long long l, r; int err; };
        vector<Node> st{{static_cast<int>(pat.size()) - 1, 0, n, 0}};
        unordered_set<long long> pos_set;
        while (!st.empty()) {
            Node cur = st.back();
            st.pop_back();
            if (cur.idx < 0) {
                for (long long i = cur.l; i < cur.r; i++) {
                    pos_set.insert(sa[i]);
                }
                continue;
            }
            uint8_t pch = pat[cur.idx];
            for (int c = 0; c < 256; c++) {
                if (C[c] == -1) {
                    continue;
                }
                int next_err = cur.err + (c != pch);
                if (next_err > max_err) {
                    continue;
                }
                long long nl = C[c] + occ[cur.l][c];
                long long nr = C[c] + occ[cur.r][c];
                if (nl < nr) {
                    st.push_back({cur.idx - 1, nl, nr, next_err});
                }
            }
        }
        vector<long long> res(pos_set.begin(), pos_set.end());
        sort(res.begin(), res.end());
        return res;
    }
};

string assemble_reads(const string& reference_str, const vector<string>& reads, int max_err) {
    vector<uint8_t> reference(reference_str.begin(), reference_str.end());
    auto t_build_s = high_resolution_clock::now();
    FM_index fm(reference);
    auto t_build_e = high_resolution_clock::now();
    long long build_ms = duration_cast<milliseconds>(t_build_e - t_build_s).count();

    auto t_map_s = high_resolution_clock::now();
    vector<vector<long long>> positions;
    positions.reserve(reads.size());
    for (const string& read : reads) {
        vector<uint8_t> pat(read.begin(), read.end());
        positions.push_back(fm.locate(pat, max_err));
    }
    auto t_map_e = high_resolution_clock::now();
    long long map_ms = duration_cast<milliseconds>(t_map_e - t_map_s).count();

    auto t_asm_s = high_resolution_clock::now();
    string assembled(reference.size(), 'N');
    for (size_t i = 0; i < reads.size(); i++) {
        const string& read = reads[i];
        for (long long pos : positions[i]) {
            if (pos < 0 || static_cast<size_t>(pos) + read.size() > reference.size()) {
                continue;
            }
            for (size_t j = 0; j < read.size(); j++) {
                assembled[pos + j] = read[j];
            }
        }
    }
    auto t_asm_e = high_resolution_clock::now();
    long long asm_ms = duration_cast<milliseconds>(t_asm_e - t_asm_s).count();

    long long total_ms = build_ms + map_ms + asm_ms;
    ofstream tfs("fmindex_timing.txt");
    if (tfs) {
        tfs << "FM-index build time     : " << build_ms << " ms\n";
        tfs << "Read mapping time       : " << map_ms << " ms\n";
        tfs << "Assembly time           : " << asm_ms << " ms\n";
        tfs << "Total pipeline time     : " << total_ms << " ms\n";
    }
    return assembled;
}

int main() {
    const string ref_path  = "reference.txt";
    const string read_path = "reads.txt";
    const string out_path  = "fmindex_assembled.txt";
    int max_err = 0;
    cout << "Enter max mismatch (D): ";
    if (!(cin >> max_err) || max_err < 0) {
        cerr << "Invalid integer.\n";
        return 1;
    }
    try {
        const string reference = io::read_reference(ref_path);
        const vector<string> reads = io::read_reads(read_path);
        string assembled = assemble_reads(reference, reads, max_err);
        io::write_text(out_path, assembled);
        cout << "Assembly finished. Output: " << out_path << '\n';
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

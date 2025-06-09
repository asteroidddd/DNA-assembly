#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <unordered_set>

using namespace std;

char mutate_base(char b, mt19937& g) {
    static const char bases[4] = {'A', 'C', 'G', 'T'};
    uniform_int_distribution<int> d(1, 3);
    int idx = (b == 'A') ? 0 : (b == 'C') ? 1 : (b == 'G') ? 2 : 3;
    return bases[(idx + d(g)) % 4];
}

int main() {
    const string ref_filename   = "reference.txt";
    const string reads_filename = "reads.txt";
    const string mut_filename   = "reference_mutated.txt";

    long long read_len = 0;
    int repeat_cnt = 0;
    int max_mis = 0;

    cout << "Enter read length: ";
    if (!(cin >> read_len) || read_len <= 0) {
        cerr << "Invalid length.\n";
        return 1;
    }

    cout << "Enter repeat count: ";
    if (!(cin >> repeat_cnt) || repeat_cnt <= 0) {
        cerr << "Invalid repeat count.\n";
        return 1;
    }

    cout << "Enter max mismatches per read: ";
    if (!(cin >> max_mis) || max_mis < 0) {
        cerr << "Invalid mismatch count.\n";
        return 1;
    }

    if (max_mis > read_len) {
        max_mis = static_cast<int>(read_len);
    }

    ifstream ifs(ref_filename);
    if (!ifs) {
        cerr << "File open error: " << ref_filename << '\n';
        return 1;
    }

    string ref, line;
    while (getline(ifs, line)) {
        ref += line;
    }
    ifs.close();

    if (ref.size() < static_cast<size_t>(read_len)) {
        cerr << "Reference too short.\n";
        return 1;
    }

    string mut = ref;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis_mis(0, max_mis);
    uniform_int_distribution<long long> dis_pos(0, read_len - 1);
    uniform_int_distribution<long long> dis_start(0, read_len - 1);

    for (long long block_start = 0; block_start + read_len <= mut.size(); block_start += read_len) {
        int k = dis_mis(gen);
        unordered_set<long long> idx;
        while (static_cast<int>(idx.size()) < k) {
            idx.insert(block_start + dis_pos(gen));
        }
        for (long long p : idx) {
            mut[p] = mutate_base(mut[p], gen);
        }
    }

    ofstream mut_ofs(mut_filename);
    if (!mut_ofs) {
        cerr << "File open error: " << mut_filename << '\n';
        return 1;
    }
    mut_ofs << mut;
    mut_ofs.close();

    ofstream reads_ofs(reads_filename);
    if (!reads_ofs) {
        cerr << "File open error: " << reads_filename << '\n';
        return 1;
    }

    for (int r = 0; r < repeat_cnt; r++) {
        long long start_idx = dis_start(gen);
        for (long long pos = start_idx; pos + read_len <= static_cast<long long>(mut.size()); pos += read_len) {
            reads_ofs << mut.substr(pos, read_len) << ',';
        }
    }
    reads_ofs.close();

    cout << "Done.\n";
    return 0;
}

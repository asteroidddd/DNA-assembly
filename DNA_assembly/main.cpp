#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "SeqCoder.hpp"
#include "MultiFMIndex.hpp"

using namespace std;

// reference -> string
string read_reference(const string& filepath) {
    ifstream ifs(filepath);
    if (!ifs) {
        throw runtime_error("Failed to open reference file: " + filepath);
    }
    ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

// lead -> string
vector<string> read_leads(const string& filepath) {
    ifstream ifs(filepath);
    if (!ifs) {
        throw runtime_error("Failed to open leads file: " + filepath);
    }
    string line;
    if (!getline(ifs, line)) {
        throw runtime_error("Failed to read from leads file: " + filepath);
    }

    vector<string> leads;
    size_t start = 0;
    while (true) {
        size_t pos = line.find(',', start);
        if (pos == string::npos) {
            leads.push_back(line.substr(start));
            break;
        }
        leads.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    return leads;
}

int main() {
    const string ref_path  = "reference.txt";
    const string lead_path = "reads.txt";

    try {
        // 1) 파일 읽기
        string reference = read_reference(ref_path);
        vector<string> leads = read_leads(lead_path);

        // 2) reference 문자열을 3‐frame FM‐Index로 빌드
        Multi_FM_index mfm(reference);

        // 3) 각 lead마다 위치 탐색 및 출력
        for (const auto& lead : leads) {
            auto positions = mfm.locate(lead, /*max_err=*/0);
            cout << "Lead: " << lead << " -> positions:";
            if (positions.empty()) {
                cout << " (not found)";
            } else {
                for (int p : positions) 
                    cout << ' ' << p;
            }
            cout << '\n';
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

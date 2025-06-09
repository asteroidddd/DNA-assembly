#ifndef IOUTILS_HPP
#define IOUTILS_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace io {

// 참조 읽기
inline string read_reference(const string& path) {

    ifstream ifs(path);
    if (!ifs) {
        throw runtime_error("open fail: " + path);
    }

    ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

// 리드 읽기
inline vector<string> read_reads(const string& path) {

    ifstream ifs(path);
    string line;
    if (!ifs) {
        throw runtime_error("open fail: " + path);
    }
    if (!getline(ifs, line)) {
        throw runtime_error("read fail: " + path);
    }

    vector<string> reads; // 결과 벡터
    size_t start = 0;
    while (true) {
        size_t pos = line.find(',', start);
        if (pos == string::npos) {
            reads.push_back(line.substr(start));
            break;
        }
        reads.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    return reads;
}

// 텍스트 쓰기
inline void write_text(const string& path, const string& content) {

    ofstream ofs(path);
    if (!ofs) {
        throw runtime_error("open fail: " + path);
    }
    
    ofs << content;
}

} // namespace io

#endif // IOUTILS_HPP
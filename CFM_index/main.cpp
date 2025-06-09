#include <iostream>
#include <string>
#include <vector>
#include "IOUtils.hpp"
#include "Assemble.hpp"

using namespace std;

int main() {
    const string ref_path  = "reference.txt";          // 레퍼런스 파일
    const string read_path = "reads.txt";              // 리드 파일
    const string out_path  = "cfmindex_assembled.txt"; // 결과 출력 파일

    // 사용자 입력
    int max_err = 0;
    cout << "Enter max mismatch (D): ";
    if (!(cin >> max_err) || max_err < 0) {
        cerr << "Invalid integer.\n";
        return 1;
    }

    try {
        // 입력 로드
        string reference          = io::read_reference(ref_path);
        vector<string> reads = io::read_reads(read_path);

        // 어셈블 호출
        string assembled = assemble_reads(reference, reads, max_err);

        // 결과 저장
        io::write_text(out_path, assembled);
        cout << "Assembly finished. Output: " << out_path << "\n";
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

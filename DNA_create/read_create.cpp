#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

int main() {
    // 기본값 설정
    const string ref_filename   = "reference.txt";  // 입력 레퍼런스 파일
    const string reads_filename = "reads.txt";      // 출력 리드 파일
    const long long read_length = 32;              // 리드 길이
    const int repeat_count      = 10;               // 반복 횟수

    // 원본 시퀀스 읽기
    ifstream ifs(ref_filename);
    if (!ifs) {
        cerr << "파일 열기 오류" << endl;
        return 1;
    }
    string ref_seq, line;
    while (getline(ifs, line)) {
        ref_seq += line;
    }
    ifs.close();

    long long ref_len = ref_seq.size();
    if (ref_len < read_length) {
        cerr << "길이 오류" << endl;
        return 1;
    }

    // 랜덤 설정 및 분포 준비
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<long long> dis_start(0, read_length - 1);

    // 리드 생성
    ofstream ofs(reads_filename);
    if (!ofs) {
        cerr << "파일 열기 오류" << endl;
        return 1;
    }

    for (int iter = 0; iter < repeat_count; ++iter) {
        long long start_idx = dis_start(gen);

        // start_idx에서 시작해 read_length 간격으로 잘라내기
        for (long long pos = start_idx; pos + read_length <= ref_len; pos += read_length) {
            ofs << ref_seq.substr(pos, read_length) << ',';
        }
    }

    ofs.close();
    cout << "완료" << endl;
    return 0;
}

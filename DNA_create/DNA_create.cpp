#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

int main() {
    // 기본값 설정
    const long long length = 3000;              // 생성할 DNA 염기서열 길이
    const string filename  = "reference.txt";    // 저장할 파일명 및 경로

    // 랜덤 시드 및 분포 설정
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 3);
    const char bases[4] = {'A', 'C', 'G', 'T'};

    // 파일 열기
    ofstream ofs(filename);
    if (!ofs) {
        cerr << "파일 열기 오류: " << filename << endl;
        return 1;
    }

    // 랜덤 서열 생성 및 쓰기
    for (long long i = 0; i < length; ++i) {
        ofs << bases[dis(gen)];
    }
    ofs.close();

    cout << "완료" << endl;
    return 0;
}

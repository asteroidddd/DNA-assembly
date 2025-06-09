#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

int main() {
    const string filename = "reference.txt";
    long long length = 0;
    cout << "Enter DNA sequence length: ";
    if (!(cin >> length) || length <= 0) {
        cerr << "Invalid length.\n";
        return 1;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 3);
    const char bases[4] = {'A', 'C', 'G', 'T'};

    ofstream ofs(filename);
    if (!ofs) {
        cerr << "File open error: " << filename << '\n';
        return 1;
    }

    for (long long i = 0; i < length; i++) {
        ofs << bases[dis(gen)];
    }

    ofs.close();
    cout << "Done.\n";
    return 0;
}

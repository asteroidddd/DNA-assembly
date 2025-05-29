// FMIndex.cpp
#include "FMIndex.hpp"
#include <algorithm>
#include <numeric>

// 생성자: reference 끝에 센티널을 추가하고 내부 인덱스 빌드
FM_index::FM_index(std::vector<uint8_t> reference) {
    reference.push_back(SENT);
    n = static_cast<int>(reference.size());
    build_SA(reference);
    build_BWT(reference);
    build_C();
    build_occ();
}

// locate 구현
std::vector<int> FM_index::locate(const std::vector<uint8_t>& pat, int max_err) const {
    struct Node { int idx, l, r, err; };
    std::vector<Node> st{{static_cast<int>(pat.size())-1, 0, n, 0}};
    std::unordered_set<int> pos_set;

    while (!st.empty()) {
        Node cur = st.back();
        st.pop_back();
        if (cur.idx < 0) {
            for (int i = cur.l; i < cur.r; ++i)
                pos_set.insert(sa[i]);
            continue;
        }
        uint8_t pch = pat[cur.idx];
        for (int c = 0; c < 256; ++c) {
            if (C[c] == -1) {
                continue;
            }
            int next_err = cur.err + (c != pch);
            if (next_err > max_err) {
                continue;
            }
            int nl = C[c] + occ[cur.l][c];
            int nr = C[c] + occ[cur.r][c];
            if (nl < nr) {
                st.push_back({cur.idx-1, nl, nr, next_err});
            }
        }
    }

    std::vector<int> res;
    res.reserve(pos_set.size());
    for (int x : pos_set) {
        res.push_back(x);
    }
    std::sort(res.begin(), res.end());
    return res;
}

// 접미사 배열 생성
void FM_index::build_SA(const std::vector<uint8_t>& text) {
    sa.resize(n);
    std::iota(sa.begin(), sa.end(), 0);
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        while (a < n && b < n) {
            if (text[a] ^ text[b]) return text[a] < text[b];
            a++; b++;
        }
        return a == n;
    });
}

// BWT 생성
void FM_index::build_BWT(const std::vector<uint8_t>& text) {
    bwt.resize(n);
    for (int i = 0; i < n; ++i) {
        bwt[i] = text[(sa[i] + n - 1) % n];
    }
}

// C 배열 계산
void FM_index::build_C() {
    std::array<int,256> freq{};
    freq.fill(0);
    for (int i = 0; i < n; ++i) {
        ++freq[bwt[i]];
    }

    int sum = 0;
    for (int c = 0; c < 256; ++c) {
        if (freq[c] == 0) {
            C[c] = -1;
        } else {
            C[c] = sum;
            sum += freq[c];
        }
    }
}

// occ 테이블 계산
void FM_index::build_occ() {
    occ.clear();
    occ.resize(n+1);
    for (int j = 0; j < 256; j++) {
        occ[0][j] = 0;
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 256; j++) {
            occ[i+1][j] = occ[i][j];
        }
        occ[i+1][bwt[i]]++;
    }
}

#ifndef FMINDEX_HPP
#define FMINDEX_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <tuple>
#include <stack>
#include <numeric>
#include "CodeUtil.hpp"

using namespace std;

static constexpr uint8_t SENT_BYTE = code::SENT_PAIR; // 센티넬 바이트
static const array<uint8_t, 16> ALPHABET = code::alphabet(); // 16쌍 알파벳 테이블

class FMIndex {
public:

    // 생성자: FM-Index 구축
    FMIndex(const string& reference) {

        // 염기 -> 1바이트로 팩킹 + 끝에 '$' 추가
        auto packed_text = code::pack_pairs(reference);

        length = packed_text.size(); // 바이트 단위
        build_sa(packed_text);
        build_bwt(packed_text);
        build_c();
        build_occ();
    }

    // 패턴 검색: max_err 만큼 mismatch 허용
    vector<size_t> locate(const string& pattern, int max_err) const {

        // 패턴 팩킹
        auto pbytes = code::pack_pairs(pattern);
        pbytes.pop_back(); // 팩커가 붙인 '$' 제거

        vector<size_t> result;
        stack<tuple<int, size_t, size_t, int>> stk;
        stk.emplace(static_cast<int>(pbytes.size()) - 1, 0, length, max_err);

        while (!stk.empty()) {
            int idx;
            size_t left, right;
            int errs;
            tie(idx, left, right, errs) = stk.top();
            stk.pop();

            if (errs < 0 || left >= right) {
                continue;
            }

            // 패턴 끝 도달시 SA 구간의 위치 추가
            if (idx < 0) {
                for (size_t i = left; i < right; i++) {
                    result.push_back(sa[i] * 2); // 바이트 -> 염기 좌표
                }
                continue;
            }

            uint8_t target = pbytes[idx];
            for (uint8_t byte : ALPHABET) {
                int k  = code::byte_to_idx(byte);
                size_t nl = C[k] + (left  ? occ[left  - 1][k] : 0);
                size_t nr = C[k] +          occ[right - 1][k];
                if (nl >= nr) continue;
                stk.emplace(idx - 1, nl, nr, errs - (byte != target));
            }
        }

        sort(result.begin(), result.end());
        result.erase(unique(result.begin(), result.end()), result.end());
        return result;
    }

private:
    size_t length;                    // 레퍼런스 길이
    vector<size_t> sa;                // 접두사 배열
    vector<uint8_t> bwt;              // BWT 배열
    array<uint32_t, 17> C;            // 누적 빈도 배열
    vector<array<uint32_t, 17>> occ;  // OCC 테이블

    // SA 구축
    void build_sa(const vector<uint8_t>& text) {
        sa.resize(length);
        iota(sa.begin(), sa.end(), static_cast<size_t>(0));
        sort(sa.begin(), sa.end(),
            [&](size_t a, size_t b) {
                return lexicographical_compare(
                    text.begin() + a, text.end(),
                    text.begin() + b, text.end()
                );
            });
    }

    // BWT 구축
    void build_bwt(const vector<uint8_t>& text) {
        bwt.resize(length);
        for (size_t i = 0; i < length; i++) {
            size_t pos = (sa[i] == 0) ? (length - 1) : (sa[i] - 1);
            bwt[i] = text[pos];
        }
    }

    // C 구축
    void build_c() {
        C.fill(0);
        for (uint8_t byte : bwt) {
            C[code::byte_to_idx(byte)]++;
        }
        uint32_t sum = 0;
        for (size_t k = 0; k < C.size(); k++) {
            uint32_t cnt = C[k];
            C[k] = sum;
            sum += cnt;
        }
    }

    // OCC 테이블 구축
    void build_occ() {
        occ.assign(length, array<uint32_t,17>{});
        array<uint32_t,17> acc{};
        for (size_t i = 0; i < length; i++) {
            acc[code::byte_to_idx(bwt[i])]++;
            occ[i] = acc;
        }
    }
};

#endif // FMINDEX_HPP

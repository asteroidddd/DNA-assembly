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
#include "CodeUtil.hpp"

using namespace std;

static constexpr uint8_t SENT_CODE = 0x0; // 센티넬: $
static const array<uint8_t,4> ALPHABET = { 0x1, 0x5, 0x9, 0xD }; // A, C, G, T

class FMIndex {
public:
    // 생성자
    explicit FMIndex(const string& reference) {
        string ref_with_sent = reference + "$";
        vector<uint8_t> codes;
        codes.reserve(ref_with_sent.size());
        for (char c : ref_with_sent) {
            codes.push_back(code::encode_base(c));
        }

        // 페킹
        vector<uint8_t> packed_text;
        packed_text.reserve((codes.size() + 1) / 2);
        for (size_t i = 0; i < codes.size(); i += 2) {
            uint8_t high = codes[i];
            uint8_t low  = (i + 1 < codes.size()) ? codes[i + 1] : SENT_CODE;
            packed_text.push_back(static_cast<uint8_t>((high << 4) | (low & 0xF)));
        }

        length = ref_with_sent.size();
        build_sa(packed_text);
        build_bwt(packed_text);
        build_c();
        build_occ();
    }

    // 패턴 검색: max_err 만큼 mismatch 허용
    vector<size_t> locate(const string& pattern, int max_err) const {
        auto packed_pat = code::pack_codes(pattern);
        auto pat        = code::unpack_codes(packed_pat);
        pat.resize(pattern.size());

        vector<size_t> result;
        stack<tuple<int, size_t, size_t, int>> stk;
        stk.emplace(static_cast<int>(pat.size()) - 1, 0, length, max_err);

        while (!stk.empty()) {
            int idx;
            size_t left, right;
            int errs;
            tie(idx, left, right, errs) = stk.top();
            stk.pop();

            if (errs < 0 || left >= right) {
                continue;
            }

            // 패턴 끝에 도달하면 SA 범위 내 모든 위치를 결과에 추가
            if (idx < 0) {
                for (size_t i = left; i < right; i++) {
                    result.push_back(sa[i]);
                }
                continue;
            }

            uint8_t target = pat[idx];
            for (uint8_t code_val : ALPHABET) {
                size_t k    = code::code_to_idx(code_val);
                size_t base = C[k];
                size_t nl   = base + (left  > 0 ? occ[left  - 1][k] : 0);
                size_t nr   = base + (right > 0 ? occ[right - 1][k] : 0);
                if (nl >= nr) {
                    continue;
                }
                stk.emplace(idx - 1, nl, nr, errs - (code_val != target));
            }
        }

        sort(result.begin(), result.end());
        result.erase(unique(result.begin(), result.end()), result.end());
        return result;
    }

private:
    size_t length;                    // 레퍼런스 길이
    vector<size_t> sa;                // 접두사 배열
    vector<uint8_t> bwt_packed;       // BWT 배열
    array<uint32_t,5> C;              // 누적 빈도 배열
    vector<array<uint32_t,5>> occ;    // OCC 테이블

    // SA 구축
    void build_sa(const vector<uint8_t>& text) {
        sa.resize(length);
        vector<size_t> idx(length);
        for (size_t i = 0; i < length; i++) {
            idx[i] = i;
        }

        auto codes = code::unpack_codes(text);
        codes.resize(length);

        sort(idx.begin(), idx.end(),
            [&](size_t a, size_t b) {
                return lexicographical_compare(
                    codes.begin() + a, codes.end(),
                    codes.begin() + b, codes.end()
                );
            }
        );
        sa.swap(idx);
    }

    // BWT 구축
    void build_bwt(const vector<uint8_t>& text) {
        bwt_packed.resize((length + 1) / 2);
        fill(bwt_packed.begin(), bwt_packed.end(), 0);

        for (size_t i = 0; i < length; i++) {
            size_t pos = (sa[i] == 0) ? (length - 1) : (sa[i] - 1);
            uint8_t byte = text[pos >> 1];
            uint8_t code_val = (pos & 1) ? (byte & 0x0F) : (byte >> 4);

            if (i & 1) {
                bwt_packed[i >> 1] |= (code_val & 0x0F);
            } else {
                bwt_packed[i >> 1] = static_cast<uint8_t>((code_val << 4) & 0xF0);
            }
        }
    }

    // BWT의 4-bit 코드
    inline uint8_t bwt_code(size_t idx) const {
        uint8_t byte = bwt_packed[idx >> 1];
        if (idx & 1) {
            return (byte & 0x0F);
        } else {
            return (byte >> 4);
        }
    }

    // C 구축
    void build_c() {
        C.fill(0);
        for (size_t i = 0; i < length; i++) {
            size_t k = code::code_to_idx(bwt_code(i));
            C[k]++;
        }
        uint32_t sum = 0;
        for (size_t k = 0; k < C.size(); k++) {
            uint32_t cnt = C[k];
            C[k] = sum;
            sum += cnt;
        }
    }

    // OCC 구축
    void build_occ() {
        occ.assign(length, array<uint32_t,5>{});
        array<uint32_t,5> acc{};
        for (size_t i = 0; i < length; i++) {
            size_t k = code::code_to_idx(bwt_code(i));
            acc[k]++;
            occ[i] = acc;
        }
    }

    inline uint32_t occ_count(uint8_t code_val, size_t pos) const {
        return occ[pos][code::code_to_idx(code_val)];
    }
};

#endif // FMINDEX_HPP

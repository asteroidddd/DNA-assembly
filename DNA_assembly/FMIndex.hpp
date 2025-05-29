// FMIndex.hpp
#ifndef FMINDEX_HPP
#define FMINDEX_HPP

#include <vector>
#include <array>
#include <cstdint>
#include <unordered_set>

static constexpr uint8_t SENT = 0; // 센티널

class FM_index {
public:
    FM_index(std::vector<uint8_t> reference);
    std::vector<int> locate(const std::vector<uint8_t>& pat, int max_err) const;

private:
    int n;                                  // reference 길이 (센티널 포함)
    std::vector<int> sa;                    // 접미사 배열
    std::vector<uint8_t> bwt;               // BWT
    std::array<int,256> C;                  // 첫 열 누적 카운트
    std::vector<std::array<int,256>> occ;   // occ 테이블: pos별 누적 등장 횟수

    void build_SA(const std::vector<uint8_t>& text);
    void build_BWT(const std::vector<uint8_t>& text);
    void build_C();
    void build_occ();
};

#endif // FMINDEX_HPP
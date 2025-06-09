#ifndef CODEUTIL_HPP
#define CODEUTIL_HPP

#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace code {

// 센티넬: $
constexpr uint8_t SENT_PAIR = 0x00;

// 염기 -> 코드
inline uint8_t encode_base(char c) {
    switch (c) {
        case 'A': return 0x1;
        case 'C': return 0x5;
        case 'G': return 0x9;
        case 'T': return 0xD;
        case '$': return 0x0; // 센티넬
        default:  throw std::invalid_argument("encode_base");
    }
}

// 코드 -> 염기
inline char decode_base(uint8_t code) {
    switch (code & 0xF) {
        case 0x1: return 'A';
        case 0x5: return 'C';
        case 0x9: return 'G';
        case 0xD: return 'T';
        case 0x0: return '$';
        default:  throw std::invalid_argument("decode_base");
    }
}

// 코드 -> 인덱스(A,C,G,T)
inline int nibble_to_idx(uint8_t n) {
    switch (n & 0xF) {
        case 0x1: return 0;   // A
        case 0x5: return 1;   // C
        case 0x9: return 2;   // G
        case 0xD: return 3;   // T
        default:  throw std::invalid_argument("nibble_to_idx");
    }
}

// 문자열 -> 팩킹: 홀수 길이면 마지막 염기 버리고 '$' 추가
inline std::vector<uint8_t> pack_pairs(const std::string& seq) {
    const std::size_t pair_cnt = seq.size() / 2;
    std::vector<uint8_t> res;
    res.reserve(pair_cnt + 1);

    for (std::size_t i = 0; i < pair_cnt; ++i) {
        uint8_t high = encode_base(seq[2 * i]);
        uint8_t low = encode_base(seq[2 * i + 1]);
        res.push_back(static_cast<uint8_t>((high << 4) | low));
    }
    res.push_back(SENT_PAIR);
    return res;
}

// 1바이트 코드 -> 알파벳 인덱스
inline int byte_to_idx(uint8_t b) {
    if (b == SENT_PAIR) {
        return 0;
    }
    int high = nibble_to_idx(b >> 4);
    int low = nibble_to_idx(b & 0xF);
    return 1 + (high << 2) + low;
}

// 사전식 16쌍 테이블
inline const std::array<uint8_t, 16>& alphabet() {
    static const std::array<uint8_t, 16> table = [] {
        std::array<uint8_t, 16> t{};
        int k = 0;
        for (uint8_t high : {0x1u, 0x5u, 0x9u, 0xDu}) {
            for (uint8_t low : {0x1u, 0x5u, 0x9u, 0xDu}) {
                t[k++] = static_cast<uint8_t>((high << 4) | low);
            }
        }
        return t;
    }();
    return table;
}

// 팩킹 -> 문자열
inline std::string to_string(const std::vector<uint8_t>& packed) {
    std::string out;
    for (uint8_t byte : packed) {
        if (byte == SENT_PAIR) { out.push_back('$'); break; }
        out.push_back(decode_base(byte >> 4));
        out.push_back(decode_base(byte & 0xF));
    }
    return out;
}

} // namespace code

#endif // CODEUTIL_HPP

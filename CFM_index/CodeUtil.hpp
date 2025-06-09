#ifndef CODEUTIL_HPP
#define CODEUTIL_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>

namespace code {

// 센티넬 코드
constexpr uint8_t SENT_CODE = 0x0; // '$'
// 패딩 코드
constexpr uint8_t PAD_CODE  = 0xF; // 'N'

// 염기 -> 코드 변환
inline uint8_t encode_base(char c)
{
    switch (c) {
        case 'A': return 0x1;
        case 'C': return 0x5;
        case 'G': return 0x9;
        case 'T': return 0xD;
        case '$': return SENT_CODE;
        case 'N': return PAD_CODE;       // 패딩용
        default:  throw std::invalid_argument("encode_base: invalid base");
    }
}

// 코드 -> 염기 변환
inline char decode_base(uint8_t code)
{
    switch (code & 0xF) {
        case 0x1: return 'A';
        case 0x5: return 'C';
        case 0x9: return 'G';
        case 0xD: return 'T';
        case 0x0: return '$';
        case 0xF: return 'N';            // 패딩
        default:  throw std::invalid_argument("decode_base: invalid code");
    }
}

// 코드 -> 인덱스 변환
inline int code_to_idx(uint8_t code)
{
    switch (code & 0xF) {
        case 0x1: return 1;   // A
        case 0x5: return 2;   // C
        case 0x9: return 3;   // G
        case 0xD: return 4;   // T
        case 0x0: return 0;   // $
        default:  throw std::invalid_argument("code_to_idx: invalid code");
    }
}

// 문자열 -> 팩킹 변환
inline std::vector<uint8_t> pack_codes(const std::string& seq)
{
    std::vector<uint8_t> res;                  // 결과 벡터
    res.reserve((seq.size() + 1) / 2);
    for (size_t i = 0; i < seq.size(); i += 2) {
        uint8_t high = encode_base(seq[i]);                                       // 상위 4bit
        uint8_t low  = (i + 1 < seq.size()) ? encode_base(seq[i + 1]) : PAD_CODE; // 하위 4bit
        res.push_back(static_cast<uint8_t>((high << 4) | (low & 0xF)));
    }
    return res;
}

// 팩킹 -> 코드 벡터 변환
inline std::vector<uint8_t> unpack_codes(const std::vector<uint8_t>& packed)
{
    std::vector<uint8_t> codes;
    codes.reserve(packed.size() * 2);
    for (uint8_t byte : packed) {
        codes.push_back((byte >> 4) & 0xF);
        codes.push_back(byte & 0xF);
    }
    return codes;
}

// 팩킹 -> 문자열 변환
inline std::string to_string(const std::vector<uint8_t>& packed)
{
    std::string seq;
    seq.reserve(packed.size() * 2);
    for (uint8_t byte : packed) {
        seq.push_back(decode_base((byte >> 4) & 0xF));
        seq.push_back(decode_base(byte & 0xF));
    }
    return seq;
}

} // namespace code

#endif // CODEUTIL_HPP

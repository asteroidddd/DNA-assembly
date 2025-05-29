// SeqCoder.cpp
#include "SeqCoder.hpp"
#include <stdexcept>

// 인코딩: 3글자씩만 처리, 나머지 무시
std::vector<uint8_t> SeqCoder::encode(const std::string& seq) {
    std::vector<uint8_t> out;
    size_t max_idx = seq.size() / 3 * 3; // 처리 대상 마지막 인덱스
    size_t i = 0;

    while (i < max_idx) {
        uint8_t byte = 0;
        // 3글자 인코딩
        for (int k = 0; k < 3; k++) {
            byte <<= 2;
            byte |= encode_base(seq[i++]);
        }
        // 마지막 2비트는 항상 11
        byte = (byte << 2) | 0b11;
        out.push_back(byte);
    }
    return out;
}

// 프레임별 인코딩
std::array<std::vector<uint8_t>, 3> SeqCoder::all_encode(const std::string& seq) {
    std::array<std::vector<uint8_t>, 3> frames;
    for (int frame = 0; frame < 3; ++frame) {
        if (frame >= static_cast<int>(seq.size())) {
            frames[frame] = {};
        } else {
            frames[frame] = encode(seq.substr(frame));
        }
    }
    return frames;
}

// 디코딩
std::string SeqCoder::decode(const std::vector<uint8_t>& data) {
    std::string seq;
    seq.reserve(data.size() * 3);

    for (uint8_t byte : data) {
        uint8_t bits6 = byte >> 2;  // 하위 6비트(염기 3개)
        for (int shift = 4; shift >= 0; shift -= 2) {
            uint8_t code = (bits6 >> shift) & 0b11;
            seq.push_back(decode_base(code));
        }
    }
    return seq;
}

// 헬퍼: 문자 <-> 2비트 코드
uint8_t SeqCoder::encode_base(char c) {
    switch (c) {
        case 'A': case 'a': return 0b00;
        case 'C': case 'c': return 0b01;
        case 'G': case 'g': return 0b10;
        case 'T': case 't': return 0b11;
        default: throw std::invalid_argument("Invalid base for encoding");
    }
}

char SeqCoder::decode_base(uint8_t code) {
    switch (code & 0b11) {
        case 0b00: return 'A';
        case 0b01: return 'C';
        case 0b10: return 'G';
        case 0b11: return 'T';
    }
    return 'N';  // 디버깅 용
}

// SeqCoder.hpp
#ifndef SEQCODER_HPP
#define SEQCODER_HPP

#include <string>
#include <vector>
#include <array>
#include <cstdint>

class SeqCoder {
public:
    static std::vector<uint8_t> encode(const std::string& seq);
    static std::array<std::vector<uint8_t>, 3> all_encode(const std::string& seq);
    static std::string decode(const std::vector<uint8_t>& data);

private:
    static uint8_t encode_base(char c);
    static char decode_base(uint8_t code);
};

#endif // SEQCODER_HPP

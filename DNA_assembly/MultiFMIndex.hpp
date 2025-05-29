// MultiFMIndex.hpp
#ifndef MULTIFMINDEX_HPP
#define MULTIFMINDEX_HPP

#include <string>
#include <vector>
#include <array>
#include <memory>
#include "FMIndex.hpp"

class Multi_FM_index {
public:
    explicit Multi_FM_index(const std::string& seq);
    std::vector<int> locate(const std::string& pattern, int max_err = 0) const;

private:
    // 스마트 포인터 배열로 변경
    std::array<std::unique_ptr<FM_index>, 3> indexes_;
};

#endif // MULTIFMINDEX_HPP

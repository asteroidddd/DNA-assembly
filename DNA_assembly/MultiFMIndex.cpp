// MultiFMIndex.cpp
#include "MultiFMIndex.hpp"
#include "SeqCoder.hpp"
#include <algorithm>

Multi_FM_index::Multi_FM_index(const std::string& seq) {
    auto frames = SeqCoder::all_encode(seq);
    // 각 포인터에 FM_index 인스턴스를 생성해 할당
    for (int i = 0; i < 3; i++) {
        indexes_[i] = std::make_unique<FM_index>(frames[i]);
    }
}

std::vector<int> Multi_FM_index::locate(const std::string& pattern, int max_err) const {
    size_t tripletCount = pattern.size() / 3;
    auto pat_enc = SeqCoder::encode(pattern.substr(0, tripletCount * 3));

    std::vector<int> hits;
    for (int frame = 0; frame < 3; frame++) {
        auto locs = indexes_[frame]->locate(pat_enc, max_err);
        for (int pos : locs) {
            hits.push_back(pos * 3 + frame);
        }
    }
    std::sort(hits.begin(), hits.end());
    hits.erase(std::unique(hits.begin(), hits.end()), hits.end());
    return hits;
}

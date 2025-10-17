#pragma once
#include <vector>
#include <fstream>
#include <algorithm>
namespace Utils {

    static std::vector<char> getDataBlock(std::ifstream& in, uint32_t diff, uint32_t next){
        in.seekg(diff, std::ios::beg);
        in.seekg(0, std::ios::beg);

        std::vector<char> buffer(static_cast<size_t>(next));
        in.read(buffer.data(), next);
        return buffer;
    }
}
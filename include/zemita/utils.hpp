#pragma once
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <print>
namespace Utils {

    static char* getDataBlock(std::ifstream& in, uint32_t diff, uint32_t next, std::vector<char>& buffer){
        in.seekg(diff, std::ios::beg);
        in.read(buffer.data(), next);
        char* raw = buffer.data();
        std::cout << "raw data: ";
        std::cout.write(raw, next); // prints exactly 'next' bytes
        std::cout << std::endl;
        return raw;
    }
    template <typename Container>
    static void printIntervalFromContainer(Container& container, int start, int end) {
        if (start > end || end >= static_cast<int>(container.size())) {
            std::cerr << "Invalid range!" << std::endl;
            return;
        }

        std::copy(
            container.begin() + start,
            container.begin() + end + 1,
            std::ostream_iterator<typename Container::value_type>(std::cout, " ")
        );

        std::cout << std::endl;
    }

    template <typename Container, std::size_t N>
    static void printIntervalFromContainer(Container (&container)[N], int start, int end){
        if (start > end || end >= static_cast<int>(N)) {
            std::cerr << "Invalid range!" << std::endl;
            return;
        }
        std::copy(container + start, container + end + 1, std::ostream_iterator<Container>(std::cout));
        std::cout << std::endl;
    }
}
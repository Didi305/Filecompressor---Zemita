#pragma once
#include <algorithm>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <print>
#include <span>
#include <string>
#include <vector>

const int READER_BUFFER_SIZE = 6 * 1024;
const int WRITER_BUFFER_SIZE = 4 * 1024;
const int SEARCH_WINDOW_SIZE = 32 * 1024;
const int LOOKAHEAD_BUFFER_SIZE = 258 * 1024;

struct Match
{
    int offset;
    int length;
    char next;
};

namespace Utils
{

static char* getDataBlock(std::ifstream& in, uint32_t diff, uint32_t next, std::vector<char>& buffer)
{
    in.seekg(diff, std::ios::beg);
    in.read(buffer.data(), next);
    char* raw = buffer.data();
    std::cout << "raw data: ";
    std::cout.write(raw, next);  // prints exactly 'next' bytes
    std::cout << std::endl;
    return raw;
}
template <typename Container>
static void printIntervalFromContainer(Container& container, int start, int end)
{
    if (start > end || end >= static_cast<int>(container.size()))
    {
        std::cerr << "Invalid range!" << std::endl;
        return;
    }

    std::copy(container.begin() + start, container.begin() + end + 1,
              std::ostream_iterator<typename Container::value_type>(std::cout, " "));

    std::cout << std::endl;
}

template <typename Container, std::size_t N>
static void printIntervalFromContainer(Container (&container)[N], int start, int end)
{
    if (start > end || end >= static_cast<int>(N))
    {
        std::cerr << "Invalid range!" << std::endl;
        return;
    }
    std::copy(container + start, container + end + 1, std::ostream_iterator<Container>(std::cout));
    std::cout << std::endl;
}

static auto findLastMatch(std::deque<char>& deq, char character)
{
    std::vector<long long int> matchIndexes;
    auto it = deq.begin();
    while ((it = std::find(it, deq.end(), character)) != deq.end())
    {
        auto newIndex = std::distance(deq.begin(), it);

        if (!matchIndexes.empty() && matchIndexes.back() == newIndex - 1)
        {
            matchIndexes.pop_back();
        }
        matchIndexes.push_back(newIndex);
        it++;
    }
    return matchIndexes;
}

static auto ahBufferContainsMatch(std::deque<char>& aheadBuffer, std::vector<char>& placeholder)
{
    auto size = static_cast<int>(placeholder.size());
    std::ranges::subrange sub(aheadBuffer.begin(), aheadBuffer.begin() + size);
    std::vector<char> subVector(sub.begin(), sub.end());
    return subVector == placeholder;
}

}  // namespace Utils
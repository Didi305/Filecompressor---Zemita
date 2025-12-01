#include "codecs/lz77.hpp"

#include <String>
#include <algorithm>
#include <optional>
#include <vector>

LZ77Codec::LZ77Codec(int windowSize, int lookAHead) : windowSize_(windowSize), lookAHead_(lookAHead)
{
    std::println("LZ77 Codec created with size: {} and {}", windowSize, lookAHead);
}

std::vector<Match> LZ77Codec::compress(std::span<const char> blockData)
{
    std::vector<Match> matches;
    char afterMatchChar;
    std::vector<char> placeholder;
    aheadBuffer.assign(blockData.begin(), blockData.end());
    while (!aheadBuffer.empty())
    {
        int length = 0;
        auto nextChar = aheadBuffer.front();
        auto nextCharPos = std::find(searchWindow.begin(), searchWindow.end(), nextChar);
        if (nextCharPos == searchWindow.end() || searchWindow.empty())
        {
            searchWindow.push_back(nextChar);
            if (searchWindow.size() > windowSize_)
                searchWindow.pop_front();
            aheadBuffer.pop_front();
            Match match = {.offset = 0, .length = 0, .next = nextChar};
            matches.emplace_back(match);
            continue;
        }
        auto matchStartPos = std::distance(searchWindow.begin(), nextCharPos);

        placeholder.emplace_back(nextChar);
        aheadBuffer.pop_front();
        length++;
        while (!aheadBuffer.empty() && matchStartPos + length < searchWindow.size() &&
               searchWindow[matchStartPos + length] == aheadBuffer[0])
        {
            nextChar = aheadBuffer.front();
            nextCharPos = std::find(searchWindow.begin(), searchWindow.end(), nextChar);
            if (nextCharPos == searchWindow.end())
            {
                break;
            }
            placeholder.push_back(nextChar);
            aheadBuffer.pop_front();
            length++;
        }
        if (!aheadBuffer.empty())
        {
            afterMatchChar = aheadBuffer.front();
            aheadBuffer.pop_front();
        }
        Match match = {
            .offset = static_cast<int>(searchWindow.size() - matchStartPos), .length = length, .next = afterMatchChar};
        matches.emplace_back(match);
        if (placeholder.size() + searchWindow.size() > windowSize_)
        {
            size_t overflow = (searchWindow.size() + placeholder.size()) - windowSize_;
            searchWindow.erase(searchWindow.begin(), searchWindow.begin() + overflow);
        }
        searchWindow.insert(searchWindow.end(), placeholder.begin(), placeholder.end());
        searchWindow.push_back(afterMatchChar);
        placeholder.clear();
    }
    return matches;
}

std::vector<char> LZ77Codec::decompress(std::span<const Match> matcheas)
{
    std::vector<char> out;
    // your implementation
    return out;
}
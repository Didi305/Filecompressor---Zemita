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
    std::vector<char> placeholder;
    aheadBuffer.assign(blockData.begin(), blockData.end());
    while (!aheadBuffer.empty())
    {
        char afterMatchChar = '?';
        int length = 0;
        auto nextChar = aheadBuffer.front();
        auto matchIndexes = Utils::findLastMatch(searchWindow, nextChar);
        if (matchIndexes.empty())
        {
            searchWindow.push_back(nextChar);
            if (searchWindow.size() > windowSize_)
            {
                searchWindow.pop_front();
            }
            aheadBuffer.pop_front();
            Match match = {.offset = 0, .length = 0, .next = nextChar};
            matches.emplace_back(match);
            continue;
        }

        placeholder.push_back(nextChar);
        int maxLength = 1;
        long long bestIndex = 0;
        for (auto index : matchIndexes)
        {
            auto aheadBufferCopy = aheadBuffer;
            aheadBufferCopy.pop_front();
            int matchLength = 1;
            while (!aheadBufferCopy.empty() && index + matchLength < searchWindow.size())
            {
                if (!(searchWindow[index + matchLength] == aheadBufferCopy.front()))
                {
                    break;
                }
                nextChar = aheadBufferCopy.front();
                placeholder.push_back(nextChar);
                aheadBufferCopy.pop_front();
                matchLength++;
            }
            if (matchLength >= maxLength)
            {
                maxLength = matchLength;
                bestIndex = index;
            }
        }

        aheadBuffer.erase(aheadBuffer.begin(), aheadBuffer.begin() + maxLength);

        while (!aheadBuffer.empty() && Utils::ahBufferContainsMatch(aheadBuffer, placeholder))
        {
            auto matchSize = static_cast<int>(placeholder.size());

            aheadBuffer.erase(aheadBuffer.begin(), aheadBuffer.begin() + matchSize);
            maxLength += matchSize;
        }
        if (!aheadBuffer.empty())
        {
            afterMatchChar = aheadBuffer.front();
            aheadBuffer.pop_front();
        }
        Match match = {
            .offset = static_cast<int>(searchWindow.size() - bestIndex), .length = maxLength, .next = afterMatchChar};
        matches.emplace_back(match);
        if (placeholder.size() + searchWindow.size() > windowSize_)
        {
            auto overflow = static_cast<int>(searchWindow.size() + placeholder.size()) - windowSize_;
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
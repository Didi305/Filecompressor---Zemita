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
            Match match = {.offset = {0, 0}, .length = 0, .next = nextChar};
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
        auto matchSize = static_cast<int>(placeholder.size());
        aheadBuffer.erase(aheadBuffer.begin(), aheadBuffer.begin() + maxLength);

        while (!aheadBuffer.empty() && Utils::ahBufferContainsMatch(aheadBuffer, placeholder))
        {
            aheadBuffer.erase(aheadBuffer.begin(), aheadBuffer.begin() + matchSize);
            maxLength += matchSize;
        }
        if (!aheadBuffer.empty())
        {
            afterMatchChar = aheadBuffer.front();
            aheadBuffer.pop_front();
        }
        Match match = {.offset = {static_cast<int>(searchWindow.size() - bestIndex), matchSize},
                       .length = maxLength,
                       .next = afterMatchChar};
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

std::vector<char> LZ77Codec::decompress(std::span<const Match> matches)
{
    std::vector<char> out;
    for (const auto& match : matches)
    {
        std::println("offset: {}, length: {}, nextChar: {}", match.offset, match.length, match.next);
        auto off = std::get<0>(match.offset);
        auto numberOfChar = std::get<1>(match.offset);
        if (match.length == 0)
        {
            out.emplace_back(match.next);
        }
        else
        {
            int match_repetition = match.length / numberOfChar;
            auto iterator = out.begin() + out.size() - off;
            std::vector<char> temp1(iterator, iterator + numberOfChar);
            std::vector<char> temp2;
            temp2.reserve(temp1.size() * match_repetition);
            for (int i = 0; i < match_repetition; i++)
            {
                temp2.insert(temp2.end(), temp1.begin(), temp1.end());
            }
            out.insert(out.end(), temp2.begin(), temp2.end());
            if (match.next != '?')
            {
                out.emplace_back(match.next);
            }
        }
    }
    std::println("output: {}", out);
    return out;
}
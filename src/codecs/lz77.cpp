#include "codecs/lz77.hpp"

#include <algorithm>
#include <vector>

#include "tracy/public/tracy/Tracy.hpp"

LZ77Codec::LZ77Codec(int windowSize, uint16_t lookAHead)
    : capacity_(windowSize + lookAHead), masterBuffer(windowSize + lookAHead)
{
}

auto LZ77Codec::compress(std::span<const char> blockData) -> std::vector<Match>
{
    ZoneScoped;
    auto LASIZE = masterBuffer.lookaheadSize();
    auto availableDataSize =
        std::min(LOOKAHEAD_BUFFER_SIZE, static_cast<int>(blockData.size())) - masterBuffer.lookaheadSize();
    int blockDataOffset = 0;
    int zamel = 0;
    std::vector<Match> matches;
    if (blockData.empty())
    {
        return matches;
    };
    std::vector<char>& ringBuffer = masterBuffer.getBuffer();
    std::copy(blockData.begin(), blockData.begin() + availableDataSize,
              ringBuffer.begin() + masterBuffer.getAheadFrontIndex());
    masterBuffer.setAheadEnd(masterBuffer.getAheadEndIndex() + availableDataSize);
    blockDataOffset += availableDataSize;
    int nextThreeBytesHashed;
    while (!masterBuffer.lookAheadEmpty())
    {
        auto matchesSize = matches.size();
        auto lookahedSize = masterBuffer.lookaheadSize();
        auto windowSize = masterBuffer.windowSize();
        int length = 0;

        if (!((masterBuffer.lookaheadSize() >= 3)))
        {
            ZoneScopedN("Step 1: Literal");
            matches.push_back(
                {.length = 0, .offset = static_cast<uint16_t>(ringBuffer[masterBuffer.getAheadFrontIndex()])});
            masterBuffer.setAheadFront(masterBuffer.getAheadFrontIndex() + 1);
            masterBuffer.setWindowEnd(masterBuffer.getAheadFrontIndex());
            masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
            length++;
        }
        else if (!match_map.contains(LZ77::hashNextThreeBytes(ringBuffer, capacity_, masterBuffer.getWindowEndIndex())))
        {
            ZoneScopedN("Step 2: Adding hash to map");
            matches.push_back(
                {.length = 0, .offset = static_cast<uint16_t>(ringBuffer[masterBuffer.getAheadFrontIndex()])});
            match_map.emplace(LZ77::hashNextThreeBytes(ringBuffer, capacity_, masterBuffer.getWindowEndIndex()),
                              RingBuffer<int>(MAX_NUMBER_MATCH_OPTIONS, {masterBuffer.getAheadFrontIndex()}));
            masterBuffer.setAheadFront(masterBuffer.getAheadFrontIndex() + 1);
            masterBuffer.setWindowEnd(masterBuffer.getAheadFrontIndex());
            masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
            length++;
        }
        else
        {
            auto matchIndexes =
                match_map.at(LZ77::hashNextThreeBytes(ringBuffer, capacity_, masterBuffer.getWindowEndIndex()))
                    .getBuffer();

            int maxLength = 3;
            int bestIndex = 0;
            auto matchStartIndex = masterBuffer.getWindowEndIndex();
            for (auto index : matchIndexes)
            {
                ZoneScopedN("Step 4: Checking indexes");
                int matchLength = 3;
                if (maxLength >= 8)
                {
                    break;
                }
                if (!masterBuffer.isInWindow(index))
                    continue;  // quick check first
                auto lookAheadFrontIndex = masterBuffer.getAheadFrontIndex();
                auto lookAheadEndIndex = masterBuffer.getAheadEndIndex();
                auto windowEndIndex = masterBuffer.getWindowEndIndex();
                auto windowFrontIndex = masterBuffer.getWindowFrontIndex();

                lookAheadFrontIndex = (lookAheadFrontIndex + 3) % capacity_;
                while (lookAheadFrontIndex != lookAheadEndIndex)
                {
                    ZoneScopedN("Step 5: looking for longest match");
                    auto upcomingChar = ringBuffer[lookAheadFrontIndex];
                    if (!(ringBuffer[(index + matchLength) % capacity_] == upcomingChar))
                    {
                        break;
                    }
                    lookAheadFrontIndex = (lookAheadFrontIndex + 1) % capacity_;
                    matchLength++;
                    windowEndIndex = lookAheadFrontIndex;
                }
                if (matchLength == maxLength)
                {
                    bestIndex = index;
                }
                else if (matchLength > maxLength)
                {
                    maxLength = matchLength;
                    bestIndex = index;
                }
            }
            auto buffer = masterBuffer.getBuffer();
            auto aheadEndIndex = masterBuffer.getAheadEndIndex();
            for (int i{0}; i < maxLength; i += 3)
            {
                ZoneScopedN("Step 6: Filling hashes");
                int pos = (matchStartIndex + i) % capacity_;
                auto hash = LZ77::hashNextThreeBytes(buffer, capacity_, pos);

                auto iterator = match_map.find(hash);
                if (iterator != match_map.end())
                {
                    iterator->second.pushIndex(pos);
                }
                else
                {
                    match_map.emplace(hash, RingBuffer<int>(MAX_NUMBER_MATCH_OPTIONS, {pos}));
                }
            }
            masterBuffer.refillLookahead(blockDataOffset, blockData, maxLength);
            masterBuffer.setAheadFront(masterBuffer.getAheadFrontIndex() + maxLength);
            masterBuffer.setWindowEnd(masterBuffer.getAheadFrontIndex());
            auto matchLength = maxLength;
            auto matchable = true;

            int offset = matchStartIndex - bestIndex;
            if (offset < 0)
                offset += capacity_;
            Match match = {.length = static_cast<uint16_t>(maxLength), .offset = static_cast<uint16_t>(offset)};
            matches.emplace_back(match);
        }
        zamel++;
    }
    /* int totalLength = 0;
    int matchCount = 0;
    int maxFound = 0;
    int short_matches = 0;   // length 3-5
    int medium_matches = 0;  // length 6-20
    int long_matches = 0;    // length 21+
    for (auto& m : matches)
    {
        if (m.length > 0 && m.length <= 5)
            short_matches++;
        else if (m.length <= 20)
            medium_matches++;
        else if (m.length > 20)
            long_matches++;
        if (m.length > 0)
        {
            totalLength += m.length;
            matchCount++;
        }
        if (m.length > maxFound)
            maxFound = m.length;
    }
    std::println("Avg match length: {}", matchCount ? totalLength / matchCount : 0);
    std::println("Total matches: {}, Total literals: {}", matchCount, matches.size() - matchCount);
    std::println("Longest match: {}", maxFound);
    std::println("Short: {}, Medium: {}, Long: {}", short_matches, medium_matches, long_matches); */
    return matches;
};

auto LZ77Codec::decompress(std::span<const Match> matches) -> std::vector<char>
{
    std::vector<char> out;
    /* for (const auto& match : matches)
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
    std::println("output: {}", out); */
    return out;
}
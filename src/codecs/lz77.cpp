#include "codecs/lz77.hpp"

#include <algorithm>
#include <vector>

#include "tracy/Tracy.hpp"

LZ77Codec::LZ77Codec(int windowSize, uint16_t lookAHead)
    : capacity_(windowSize + lookAHead), masterBuffer(windowSize + lookAHead), match_table(TABLE_SIZE)
{
}

const std::vector<Match> LZ77Codec::compress(std::span<const char> blockData, BufferedWriter& writer, int number)
{
    ZoneScoped;
    auto availableDataSize =
        std::min(LOOKAHEAD_BUFFER_SIZE, static_cast<int>(blockData.size())) - masterBuffer.lookaheadSize();
    int blockDataOffset = 0;
    int outputPosition = 0;  // track how many bytes we've "output"

    std::vector<Match> matches;

    std::vector<char>& ringBuffer = masterBuffer.getBuffer();
    auto front = masterBuffer.getAheadFrontIndex();
    if (front + availableDataSize <= capacity_)
    {
        std::copy(blockData.begin(), blockData.begin() + availableDataSize, ringBuffer.begin() + front);
    }
    else
    {
        auto lil = capacity_ - front;
        std::copy(blockData.begin(), blockData.begin() + lil, ringBuffer.begin() + front);
        masterBuffer.setAheadEnd(front + lil);
        std::copy(blockData.begin(), blockData.begin() + availableDataSize - lil,
                  ringBuffer.begin() + masterBuffer.getAheadEndIndex());
    }
    masterBuffer.setAheadEnd(masterBuffer.getAheadEndIndex() + availableDataSize);

    blockDataOffset += availableDataSize;

    while (!masterBuffer.lookAheadEmpty())
    {
        int aheadFront = masterBuffer.getAheadFrontIndex();
        int aheadEnd = masterBuffer.getAheadEndIndex();
        int windowEnd = masterBuffer.getWindowEndIndex();
        bool atLeastThreeBytes = masterBuffer.lookaheadSize() >= 3;
        int hash = atLeastThreeBytes ? LZ77::hashFunction(ringBuffer, capacity_, windowEnd) : -1;

        if (!(atLeastThreeBytes))
        {
            matches.push_back(
                {.length = 0, .offset = static_cast<uint16_t>(static_cast<unsigned char>(ringBuffer[aheadFront]))});
            aheadFront++;
            windowEnd = aheadFront;
            masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
            outputPosition++;
        }
        else
        {
            int slot = hash % TABLE_SIZE;

            if (match_table[slot].isEmpty())
            {
                matches.push_back(
                    {.length = 0, .offset = static_cast<uint16_t>(static_cast<unsigned char>(ringBuffer[aheadFront]))});
                match_table[slot] = matchIndices(aheadFront);
                masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
                outputPosition++;
                aheadFront++;
                windowEnd++;
            }

            else
            {
                auto& matchIndexes = match_table[slot];
                int maxLength = 3;
                int bestIndex = -1;
                auto matchStartIndex = windowEnd;
                for (auto& index : matchIndexes.indices)
                {
                    int matchLength = 3;
                    if (maxLength >= 32 || index < 0)
                    {
                        break;
                    }
                    if (!masterBuffer.isInWindow(index) || hash != LZ77::hashFunction(ringBuffer, capacity_, index))
                    {
                        continue;  // quick check first
                    }
                    auto lookAheadFrontIndex = aheadFront;
                    auto lookAheadEndIndex = aheadEnd;
                    auto windowEndIndex = windowEnd;

                    lookAheadFrontIndex = (lookAheadFrontIndex + 3) % capacity_;
                    while (lookAheadFrontIndex != lookAheadEndIndex)
                    {
                        if (!(ringBuffer[(index + matchLength) % capacity_] == ringBuffer[lookAheadFrontIndex]))
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

                // No valid match found - emit single literal
                if (bestIndex < 0)
                {
                    matches.push_back(
                        {.length = 0,
                         .offset = static_cast<uint16_t>(static_cast<unsigned char>(ringBuffer[aheadFront]))});
                    match_table[slot].pushIndex(aheadFront);
                    aheadFront++;
                    windowEnd++;
                    masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
                    outputPosition++;
                }
                else
                {
                    // Valid match found - update hash table and emit match
                    for (int i{0}; i < maxLength; i++)
                    {
                        int pos = (matchStartIndex + i) % capacity_;

                        hash = LZ77::hashFunction(ringBuffer, capacity_, pos);
                        int innerSlot = hash % TABLE_SIZE;
                        if (match_table[innerSlot].count > 0)
                        {
                            match_table[innerSlot].pushIndex(pos);
                        }
                        else
                        {
                            match_table[innerSlot].count++;
                            match_table[innerSlot] = matchIndices(pos);
                        }
                    }
                    masterBuffer.refillLookahead(blockDataOffset, blockData, maxLength);
                    aheadFront += maxLength;
                    windowEnd = aheadFront;

                    int offset = matchStartIndex - bestIndex;
                    if (offset < 0)
                    {
                        offset += capacity_;
                    }

                    Match match = {.length = static_cast<uint16_t>(maxLength), .offset = static_cast<uint16_t>(offset)};
                    matches.emplace_back(match);
                    outputPosition += maxLength;
                }
            }
        }
        masterBuffer.setAheadFront(aheadFront);
        masterBuffer.setWindowEnd(windowEnd);
    }
    return matches;
};

auto LZ77Codec::decompress(std::span<const Match> matches, std::vector<char>& full) -> std::vector<char>
{
    std::vector<char> out;

    out.reserve(matches.size() * 2);  // rough estimate

    for (const auto& match : matches)
    {
        if (match.length == 0)
        {
            // Literal: offset contains the byte value
            full.push_back(static_cast<char>(match.offset));
            out.push_back(static_cast<char>(match.offset));
        }
        else
        {
            // Back-reference: copy 'length' bytes from 'offset' positions back
            size_t start = full.size() - match.offset;
            for (uint16_t i = 0; i < match.length; i++)
            {
                char byte = full[start + i];
                full.push_back(byte);
                out.push_back(byte);
            }
        }
    }

    return out;
}
#include "codecs/lz77.hpp"

#include <algorithm>
#include <vector>

#include "tracy/Tracy.hpp"

LZ77Codec::LZ77Codec(int windowSize, uint16_t lookAHead)
    : capacity_(windowSize + lookAHead), masterBuffer(windowSize + lookAHead), match_table(TABLE_SIZE)
{
}

auto LZ77Codec::compress(std::span<const char> blockData, BufferedWriter& writer, int number) -> int
{
    ZoneScoped;
    writer.reset();
    std::vector<char> literals;
    auto availableDataSize =
        std::min(LOOKAHEAD_BUFFER_SIZE, static_cast<int>(blockData.size())) - masterBuffer.lookaheadSize();
    int blockDataOffset = 0;
    int outputPosition = 0;  // track how many bytes we've "output"
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

    int numberOfLiterals = 0;
    int literalsFlagBits = 0;

    int numberOfMatches = 0;
    int matchesFlagBits = 0;
    auto addLiteral = [&](char c)
    {
        if (literals.size() >= 255)
        {
            uint8_t size = 255;
            writer.write(reinterpret_cast<char*>(&size), sizeof(uint8_t));
            writer.write(literals.data(), 255);
            literalsFlagBits++;
            literals.clear();
        }
        literals.push_back(c);
        numberOfLiterals++;
    };
    while (!masterBuffer.lookAheadEmpty())
    {
        int aheadFront = masterBuffer.getAheadFrontIndex();
        int aheadEnd = masterBuffer.getAheadEndIndex();
        int windowEnd = masterBuffer.getWindowEndIndex();
        bool atLeastThreeBytes = masterBuffer.lookaheadSize() >= 3;
        int hash = atLeastThreeBytes ? LZ77::hashFunction(ringBuffer, capacity_, windowEnd) : -1;

        if (!(atLeastThreeBytes))
        {
            addLiteral(ringBuffer[aheadFront]);
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
                match_table[slot] = matchIndices(aheadFront);
                masterBuffer.refillLookahead(blockDataOffset, blockData, 1);
                outputPosition++;
                addLiteral(ringBuffer[aheadFront]);
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
                    addLiteral(ringBuffer[aheadFront]);
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
                    auto size = literals.size();
                    if (size > 0)
                    {
                        writer.write(reinterpret_cast<char*>(&size), sizeof(uint8_t));
                        writer.write(literals.data(), literals.size());
                        literalsFlagBits++;
                        literals.clear();
                    }
                    const char matchFlagBit = 0x00;
                    writer.write(&matchFlagBit, sizeof(char));
                    writer.write(reinterpret_cast<char*>(&maxLength), sizeof(uint16_t));
                    writer.write(reinterpret_cast<char*>(&offset), sizeof(uint16_t));
                    outputPosition += maxLength;
                    numberOfMatches++;
                }
            }
        }
        masterBuffer.setAheadFront(aheadFront);
        masterBuffer.setWindowEnd(windowEnd);
    }
    if (!literals.empty())
    {
        auto size = literals.size();
        writer.write(reinterpret_cast<char*>(&size), sizeof(uint8_t));
        writer.write(literals.data(), literals.size());
        literalsFlagBits++;
    }
    return numberOfLiterals + literalsFlagBits + (numberOfMatches * 5);
};

auto LZ77Codec::decompress(std::span<const char> data, std::vector<char>& full) -> std::vector<char>
{
    std::vector<char> out;
    auto readPos = 0;
    uint16_t length, offset;
    out.reserve(data.size() * 2);  // rough estimate

    while (readPos < data.size())
    {
        if (data[readPos] == 0x00)
        {
            readPos++;
            std::memcpy(&length, &data[readPos], sizeof(uint16_t));
            readPos += sizeof(uint16_t);
            std::memcpy(&offset, &data[readPos], sizeof(uint16_t));
            readPos += sizeof(uint16_t);

            auto start = full.size() - offset;
            for (uint16_t i = 0; i < length; i++)
            {
                char byte = full[start + i];
                full.push_back(byte);
                out.push_back(byte);
            }
            // Literal: offset contains the byte value
        }
        else
        {
            auto numberOfLiterals = static_cast<uint8_t>(data[readPos]);
            readPos++;
            full.insert(full.end(), data.begin() + readPos, data.begin() + readPos + numberOfLiterals);
            out.insert(out.end(), data.begin() + readPos, data.begin() + readPos + numberOfLiterals);

            readPos += numberOfLiterals;
        }
    }

    return out;
}
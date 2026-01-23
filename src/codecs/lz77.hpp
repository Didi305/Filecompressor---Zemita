#pragma once
#include <span>

#include "codecs/ICodec.hpp"
#include "io/ring_buffer.hpp"
#include "zemita/utils.hpp"

const int TABLE_SIZE = 16777259;
const int MAX_NUMBER_MATCH_OPTIONS = 8;
const int SECOND_DIGIT_SHIFTER = 8;
const int THIRD_DIGIT_SHIFTER = 16;

struct matchIndices
{
    uint8_t count = 0;
    std::array<int, MAX_NUMBER_MATCH_OPTIONS> indices;
    matchIndices() = default;

    matchIndices(int& firstIndex) : count(1) { indices[0] = firstIndex; }
    bool isEmpty() const { return count == 0; };
    void pushIndex(int& idx)
    {
        indices[count % MAX_NUMBER_MATCH_OPTIONS] = idx;
        count++;
    }
};

namespace LZ77
{
// Helper function - only used within LZ77 compression
inline auto hashFunction(std::vector<char>& buffer, int capacity, int index) -> int
{
    return static_cast<unsigned char>(buffer[index]) +
           (static_cast<unsigned char>(buffer[(index + 1) % capacity]) << SECOND_DIGIT_SHIFTER) +
           (static_cast<unsigned char>(buffer[(index + 2) % capacity]) << THIRD_DIGIT_SHIFTER);
}

inline auto rollingHash(const std::vector<char>& buffer, int& previousHash, int index, int& capacity)
{
    return ((previousHash - static_cast<unsigned char>(buffer[(index - 1 + capacity) % capacity])) >> 8) +
           (static_cast<unsigned char>(buffer[(index + 2) % capacity]) << 16);
}

/* inline auto ahBufferContainsMatch(RingBuffer<char>& masterBuffer, std::vector<char>& ringBuffer, int&
matchLength)
{
    if (masterBuffer.getAheadEndIndex() - masterBuffer.getAheadFrontIndex() < matchLength)
    {
        return false;
    }
    auto subWindow = masterBuffer.getWindowRange(masterBuffer.getWindowEndIndex(), matchLength);
    auto subAhead = masterBuffer.getAheadRange(masterBuffer.getWindowFrontIndex(), matchLength);
    return subWindow == subAhead;
} */
}  // namespace LZ77

class LZ77Codec : public ICodec
{
   private:
    int capacity_;
    RingBuffer<char> masterBuffer; /* one buffer manages searchWindow and lookAhead*/
    std::vector<matchIndices> match_table;

   public:
    explicit LZ77Codec(int windowSize, uint16_t lookAhead);
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    const std::vector<Match> compress(std::span<const char> blockData, BufferedWriter& writer, int number) override;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    std::vector<char> decompress(std::span<const Match> matches, std::vector<char>& full) override;
};

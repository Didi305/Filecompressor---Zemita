#pragma once

#include <io/ring_buffer.hpp>

#include "codecs/ICodec.hpp"
#include "zemita/utils.hpp"

const int MAX_NUMBER_MATCH_OPTIONS = 8;
const int SECOND_DIGIT_SHIFTER = 8;
const int THIRD_DIGIT_SHIFTER = 16;
namespace LZ77
{

// Helper function - only used within LZ77 compression
inline auto hashNextThreeBytes(std::vector<char>& buffer, int capacity, int index) -> int
{
    return static_cast<unsigned char>(buffer[index]) +
           (static_cast<unsigned char>(buffer[(index + 1) % capacity]) << SECOND_DIGIT_SHIFTER) +
           (static_cast<unsigned char>(buffer[(index + 2) % capacity]) << THIRD_DIGIT_SHIFTER);
}

/* inline auto ahBufferContainsMatch(RingBuffer<char>& masterBuffer, std::vector<char>& ringBuffer, int& matchLength)
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
    std::unordered_map<int, RingBuffer<int>> match_map;

   public:
    explicit LZ77Codec(int windowSize, uint16_t lookAhead);
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    const std::vector<Match> compress(std::span<const char> blockData) override;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    std::vector<char> decompress(std::span<const Match> matches) override;
};

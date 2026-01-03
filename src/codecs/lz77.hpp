#pragma once

#include <io/ring_buffer.hpp>

#include "codecs/ICodec.hpp"
#include "zemita/utils.hpp"

const int MAX_NUMBER_MATCH_OPTIONS = 32;
const int SECOND_DIGIT_SHIFTER = 8;
const int THIRD_DIGIT_SHIFTER = 16;
namespace LZ77
{

// Helper function - only used within LZ77 compression
inline auto hashNextThreeBytes(RingBuffer<char>& masterBuffer, int index) -> int
{
    auto capacity = SEARCH_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE;
    auto buffer = masterBuffer.getBuffer();
    int result;

    if ((index + 2) % capacity >= masterBuffer.getAheadEndIndex())
    {
        return 0;
    }
    result = buffer[index % capacity] + (buffer[(index + 1) % capacity] << SECOND_DIGIT_SHIFTER) +
             (buffer[(index + 2) % capacity] << THIRD_DIGIT_SHIFTER);
    return result;
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
    std::vector<Match> compress(std::span<const char> blockData) override;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    std::vector<char> decompress(std::span<const Match> matches) override;
};

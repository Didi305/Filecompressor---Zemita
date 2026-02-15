#pragma once
#include <array>
#include <span>
#include <vector>

#include "io/buffered_writer.hpp"
#include "io/ring_buffer.hpp"
#include "zemita/utils.hpp"

const int MAX_NUMBER_MATCH_OPTIONS = 24;
const int SECOND_DIGIT_SHIFTER = 8;
const int THIRD_DIGIT_SHIFTER = 16;

struct DistTier
{
    uint16_t minOffset;
    uint8_t symbol;
    uint8_t extraBitsCount;
};

constexpr std::array<DistTier, 30> distTiers = {
    {{1, 0, 0},      {2, 1, 0},      {3, 2, 0},      {4, 3, 0},       {5, 4, 1},       {7, 5, 1},
     {9, 6, 2},      {13, 7, 2},     {17, 8, 3},     {25, 9, 3},      {33, 10, 4},     {49, 11, 4},
     {65, 12, 5},    {97, 13, 5},    {129, 14, 6},   {193, 15, 6},    {257, 16, 7},    {385, 17, 7},
     {513, 18, 8},   {769, 19, 8},   {1025, 20, 9},  {1537, 21, 9},   {2049, 22, 10},  {3073, 23, 10},
     {4097, 24, 11}, {6145, 25, 11}, {8193, 26, 12}, {12289, 27, 12}, {16385, 28, 13}, {24577, 29, 13}}};

struct Symbol
{
    uint16_t sym;
    uint8_t extraBitsCount;
    uint16_t extraBits;
};

struct matchIndices
{
    uint32_t generation = 0;
    uint8_t count = 0;
    std::array<int, MAX_NUMBER_MATCH_OPTIONS> indices;
    matchIndices() = default;

    void init(int firstIndex, uint32_t gen)
    {
        generation = gen;
        count = 1;
        indices[0] = firstIndex;
    }
    [[nodiscard]] auto isEmpty(uint32_t gen) const { return generation != gen || count == 0; };
    void pushIndex(int idx, uint32_t gen)
    {
        if (generation != gen)
        {
            generation = gen;
            count = 0;
        }
        indices[count % MAX_NUMBER_MATCH_OPTIONS] = idx;
        count++;
    }
};

class LZSSCodec
{
   private:
    int lits{};
    int matches{};

    static constexpr size_t SEARCH_WINDOW_SIZE = 32768;  // 32KB
    static constexpr size_t LOOKAHEAD_BUFFER_SIZE = 258;
    static constexpr int TABLE_SIZE = 1 << 18;  // 262,144
    static constexpr size_t LENGTH_SYMBOL_TABLE_SIZE = 256;
    static constexpr size_t LITERAl_LENGTH_FREQ_ARRAY_SIZE = 286;
    static constexpr size_t OFFSET_FREQ_ARRAY_SIZE = 30;

    std::array<Symbol, LENGTH_SYMBOL_TABLE_SIZE> info;
    std::array<uint32_t, LITERAl_LENGTH_FREQ_ARRAY_SIZE> litLenFreq{};
    std::array<uint32_t, OFFSET_FREQ_ARRAY_SIZE> offsetFreq{};

    uint16_t capacity_ = SEARCH_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE;
    RingBuffer<char> masterBuffer; /* one buffer manages searchWindow and lookAhead*/
    std::vector<matchIndices> match_table;
    uint32_t generation_ = 0;

   public:
    explicit LZSSCodec();

    const auto& getLitLenFreq() const { return litLenFreq; };
    const auto& getOffsetFreq() const { return offsetFreq; };
    auto compress(std::span<const char> blockData, BufferedWriter& writer, int number) -> int;
    auto decompress(std::span<const char> data, std::vector<char>& full) -> std::vector<char>;
    static auto lengthToSymbol(uint16_t length) -> Symbol;
    static auto offsetToSymbol(uint16_t offset) -> Symbol;
    auto afterCompressionInfo() -> void;
};
namespace LZSS
{
// Helper function - only used within LZSS compression
inline auto hashFunction(std::vector<char>& buffer, int capacity, int index) -> int
{
    return static_cast<unsigned char>(buffer[index]) +
           (static_cast<unsigned char>(buffer[(index + 1) % capacity]) << SECOND_DIGIT_SHIFTER) +
           (static_cast<unsigned char>(buffer[(index + 2) % capacity]) << THIRD_DIGIT_SHIFTER);
}

inline auto rollingHash(const std::vector<char>& buffer, int& previousHash, int index, int& capacity)
{
    return ((previousHash - static_cast<unsigned char>(buffer[(index - 1 + capacity) % capacity])) >>
            SECOND_DIGIT_SHIFTER) +
           (static_cast<unsigned char>(buffer[(index + 2) % capacity]) << THIRD_DIGIT_SHIFTER);
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
}  // namespace LZSS
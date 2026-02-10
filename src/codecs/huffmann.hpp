#pragma once

#include <array>
#include <cstdint>
#include <queue>
#include <span>
#include <vector>

struct HuffmanNode
{
    uint16_t symbol;
    uint32_t frequency;
    HuffmanNode* left = nullptr;
    HuffmanNode* right = nullptr;

    [[nodiscard]] auto isLeaf() const -> bool { return left == nullptr && right == nullptr; }
};

struct HuffmanNodeCompare
{
    auto operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const -> bool
    {
        return lhs->frequency > rhs->frequency;
    }
};

struct HuffmanCode
{
    uint32_t bits = 0;
    uint8_t length = 0;
};

using HuffmanPQ = std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanNodeCompare>;

class Huffmann
{
   public:
    static constexpr uint16_t END_OF_BLOCK = 256;
    static constexpr size_t LIT_LEN_SIZE = 286;
    static constexpr size_t OFFSET_SIZE = 30;
    static constexpr uint8_t MAX_LITERAL_RUN = 255;

    explicit Huffmann() = default;

    void buildTrees(const std::array<uint32_t, LIT_LEN_SIZE>& litLenFreq,
                    const std::array<uint32_t, OFFSET_SIZE>& offsetFreq);
    auto encode(std::span<const char> lzssOutput) -> std::vector<char>;
    auto decode(std::span<const char> huffmanData) -> std::vector<char>;

   private:
    std::vector<HuffmanNode> nodePool_;
    HuffmanNode* litLenRoot_ = nullptr;
    HuffmanNode* offsetRoot_ = nullptr;
    std::array<HuffmanCode, LIT_LEN_SIZE> litLenCodes_{};
    std::array<HuffmanCode, OFFSET_SIZE> offsetCodes_{};
    std::array<uint32_t, LIT_LEN_SIZE> litLenFreq_{};
    std::array<uint32_t, OFFSET_SIZE> offsetFreq_{};

    auto buildTree(HuffmanPQ& queue) -> HuffmanNode*;

    template <size_t N>
    void fillQueue(const std::array<uint32_t, N>& frequencies, HuffmanPQ& queue);

    static void generateCodes(HuffmanNode* node, uint32_t code, uint8_t depth,
                               std::array<HuffmanCode, LIT_LEN_SIZE>& table);
    static void generateCodes(HuffmanNode* node, uint32_t code, uint8_t depth,
                               std::array<HuffmanCode, OFFSET_SIZE>& table);

    void scanFrequencies(std::span<const char> lzssOutput);
    void writeHeader(std::vector<char>& output) const;
    void readHeader(std::span<const char> data);
};

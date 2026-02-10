#include "huffmann.hpp"

#include <cstring>

#include "codecs/lzss.hpp"

static constexpr uint8_t BITS_PER_BYTE = 8;
static constexpr uint8_t BYTE_MASK = 0xFF;


struct LengthDecode
{
    uint16_t baseLength;
    uint8_t extraBits;
};

// Index 0 = symbol 257, index 28 = symbol 285
static constexpr std::array<LengthDecode, 29> LENGTH_DECODE_TABLE = {{
    {3, 0},   {4, 0},   {5, 0},   {6, 0},   {7, 0},   {8, 0},   {9, 0},   {10, 0},   // 257-264
    {11, 1},  {13, 1},  {15, 1},  {17, 1},                                             // 265-268
    {19, 2},  {23, 2},  {27, 2},  {31, 2},                                             // 269-272
    {35, 3},  {43, 3},  {51, 3},  {59, 3},                                             // 273-276
    {67, 4},  {83, 4},  {99, 4},  {115, 4},                                            // 277-280
    {131, 5}, {163, 5}, {195, 5}, {227, 5},                                            // 281-284
    {258, 0},                                                                           // 285
}};

// Header size: litLen freqs + offset freqs
static constexpr size_t HEADER_SIZE =
    (Huffmann::LIT_LEN_SIZE * sizeof(uint32_t)) + (Huffmann::OFFSET_SIZE * sizeof(uint32_t));
static constexpr uint16_t FIRST_LENGTH_SYMBOL = 257;

template <size_t N>
void Huffmann::fillQueue(const std::array<uint32_t, N>& frequencies, HuffmanPQ& queue)
{
    for (size_t i = 0; i < N; i++)
    {
        if (frequencies[i] > 0)
        {
            auto& node = nodePool_.emplace_back();
            node.symbol = static_cast<uint16_t>(i);
            node.frequency = frequencies[i];
            queue.push(&node);
        }
    }
}

auto Huffmann::buildTree(HuffmanPQ& queue) -> HuffmanNode*
{
    if (queue.empty())
    {
        return nullptr;
    }

    if (queue.size() == 1)
    {
        auto* leaf = queue.top();
        queue.pop();
        auto& root = nodePool_.emplace_back();
        root.symbol = 0;
        root.frequency = leaf->frequency;
        root.left = leaf;
        return &root;
    }

    while (queue.size() > 1)
    {
        auto* left = queue.top();
        queue.pop();
        auto* right = queue.top();
        queue.pop();

        auto& parent = nodePool_.emplace_back();
        parent.symbol = 0;
        parent.frequency = left->frequency + right->frequency;
        parent.left = left;
        parent.right = right;
        queue.push(&parent);
    }

    return queue.top();
}

template <size_t N>
static void generateCodesImpl(HuffmanNode* node, uint32_t code, uint8_t depth,
                               std::array<HuffmanCode, N>& table)
{
    if (node == nullptr)
    {
        return;
    }
    if (node->isLeaf())
    {
        table[node->symbol] = {code, depth};
        return;
    }
    generateCodesImpl(node->left, code, depth + 1, table);                   // bit at depth = 0
    generateCodesImpl(node->right, code | (1U << depth), depth + 1, table);  // bit at depth = 1
}

void Huffmann::generateCodes(HuffmanNode* node, uint32_t code, uint8_t depth,
                             std::array<HuffmanCode, LIT_LEN_SIZE>& table)
{
    generateCodesImpl(node, code, depth, table);
}

void Huffmann::generateCodes(HuffmanNode* node, uint32_t code, uint8_t depth,
                             std::array<HuffmanCode, OFFSET_SIZE>& table)
{
    generateCodesImpl(node, code, depth, table);
}

void Huffmann::scanFrequencies(std::span<const char> lzssOutput)
{
    litLenFreq_.fill(0);
    offsetFreq_.fill(0);

    size_t pos = 0;
    while (pos < lzssOutput.size())
    {
        auto byte = static_cast<uint8_t>(lzssOutput[pos]);

        if (byte == 0x00)
        {
            // Match: [0x00][length_u16][offset_u16]
            pos++;
            uint16_t length;
            uint16_t offset;
            std::memcpy(&length, &lzssOutput[pos], sizeof(uint16_t));
            pos += sizeof(uint16_t);
            std::memcpy(&offset, &lzssOutput[pos], sizeof(uint16_t));
            pos += sizeof(uint16_t);

            litLenFreq_[LZSSCodec::lengthToSymbol(length).sym]++;
            offsetFreq_[LZSSCodec::offsetToSymbol(offset).sym]++;
        }
        else
        {
            // Literal run: [count][literals...]
            auto count = byte;
            pos++;
            for (uint8_t i = 0; i < count; i++)
            {
                litLenFreq_[static_cast<uint8_t>(lzssOutput[pos])]++;
                pos++;
            }
        }
    }

    // Always include end-of-block symbol
    litLenFreq_[END_OF_BLOCK] = 1;
}

void Huffmann::writeHeader(std::vector<char>& output) const
{
    size_t offset = output.size();
    output.resize(output.size() + HEADER_SIZE);
    std::memcpy(output.data() + offset, litLenFreq_.data(), LIT_LEN_SIZE * sizeof(uint32_t));
    offset += LIT_LEN_SIZE * sizeof(uint32_t);
    std::memcpy(output.data() + offset, offsetFreq_.data(), OFFSET_SIZE * sizeof(uint32_t));
}

void Huffmann::readHeader(std::span<const char> data)
{
    std::memcpy(litLenFreq_.data(), data.data(), LIT_LEN_SIZE * sizeof(uint32_t));
    std::memcpy(offsetFreq_.data(), data.data() + (LIT_LEN_SIZE * sizeof(uint32_t)),
                OFFSET_SIZE * sizeof(uint32_t));
}

void Huffmann::buildTrees(const std::array<uint32_t, LIT_LEN_SIZE>& litLenFreq,
                          const std::array<uint32_t, OFFSET_SIZE>& offsetFreq)
{
    nodePool_.clear();
    litLenCodes_.fill({});
    offsetCodes_.fill({});
    nodePool_.reserve((LIT_LEN_SIZE + OFFSET_SIZE) * 2);

    litLenFreq_ = litLenFreq;
    offsetFreq_ = offsetFreq;

    HuffmanPQ litLenQueue;
    fillQueue(litLenFreq_, litLenQueue);
    litLenRoot_ = buildTree(litLenQueue);
    generateCodes(litLenRoot_, 0, 0, litLenCodes_);

    HuffmanPQ offsetQueue;
    fillQueue(offsetFreq_, offsetQueue);
    offsetRoot_ = buildTree(offsetQueue);
    generateCodes(offsetRoot_, 0, 0, offsetCodes_);
}

auto Huffmann::encode(std::span<const char> lzssOutput) -> std::vector<char>
{
    
    scanFrequencies(lzssOutput);

    
    buildTrees(litLenFreq_, offsetFreq_);

    
    std::vector<char> output;
    output.reserve(HEADER_SIZE + lzssOutput.size());
    writeHeader(output);

    
    uint32_t bitBuffer = 0;
    uint8_t bitsInBuffer = 0;

    auto writeBits = [&](uint32_t bits, uint8_t count) -> void
    {
        bitBuffer |= (bits << bitsInBuffer);
        bitsInBuffer += count;
        while (bitsInBuffer >= BITS_PER_BYTE)
        {
            output.push_back(static_cast<char>(bitBuffer & BYTE_MASK));
            bitBuffer >>= BITS_PER_BYTE;
            bitsInBuffer -= BITS_PER_BYTE;
        }
    };

    size_t pos = 0;
    while (pos < lzssOutput.size())
    {
        auto byte = static_cast<uint8_t>(lzssOutput[pos]);

        if (byte == 0x00)
        {
            
            pos++;
            uint16_t length;
            uint16_t offset;
            std::memcpy(&length, &lzssOutput[pos], sizeof(uint16_t));
            pos += sizeof(uint16_t);
            std::memcpy(&offset, &lzssOutput[pos], sizeof(uint16_t));
            pos += sizeof(uint16_t);

            
            auto lengthSym = LZSSCodec::lengthToSymbol(length);
            const auto& lengthCode = litLenCodes_[lengthSym.sym];
            writeBits(lengthCode.bits, lengthCode.length);
            if (lengthSym.extraBitsCount > 0)
            {
                writeBits(lengthSym.extraBits, lengthSym.extraBitsCount);
            }

            
            auto offsetSym = LZSSCodec::offsetToSymbol(offset);
            const auto& offsetCode = offsetCodes_[offsetSym.sym];
            writeBits(offsetCode.bits, offsetCode.length);
            if (offsetSym.extraBitsCount > 0)
            {
                writeBits(offsetSym.extraBits, offsetSym.extraBitsCount);
            }
        }
        else
        {
            
            auto count = byte;
            pos++;
            for (uint8_t i = 0; i < count; i++)
            {
                auto literal = static_cast<uint8_t>(lzssOutput[pos]);
                const auto& code = litLenCodes_[literal];
                writeBits(code.bits, code.length);
                pos++;
            }
        }
    }

    
    const auto& eobCode = litLenCodes_[END_OF_BLOCK];
    writeBits(eobCode.bits, eobCode.length);

    // Flush remaining bits
    if (bitsInBuffer > 0)
    {
        output.push_back(static_cast<char>(bitBuffer & BYTE_MASK));
    }

    return output;
}

auto Huffmann::decode(std::span<const char> huffmanData) -> std::vector<char>
{
    
    readHeader(huffmanData);
    buildTrees(litLenFreq_, offsetFreq_);

    
    std::vector<char> lzssOutput;
    lzssOutput.reserve(huffmanData.size() * 2);

    
    std::vector<char> literals;

    auto flushLiterals = [&]() -> void
    {
        while (!literals.empty())
        {
            auto runSize = std::min(static_cast<size_t>(MAX_LITERAL_RUN), literals.size());
            lzssOutput.push_back(static_cast<char>(runSize));
            lzssOutput.insert(lzssOutput.end(), literals.begin(),
                              literals.begin() + static_cast<ptrdiff_t>(runSize));
            literals.erase(literals.begin(), literals.begin() + static_cast<ptrdiff_t>(runSize));
        }
    };

    
    size_t bytePos = HEADER_SIZE;
    uint8_t bitPos = 0;

    auto readBit = [&]() -> uint32_t
    {
        if (bytePos >= huffmanData.size())
        {
            return 0;
        }
        uint32_t bit = (static_cast<uint8_t>(huffmanData[bytePos]) >> bitPos) & 1;
        bitPos++;
        if (bitPos >= BITS_PER_BYTE)
        {
            bitPos = 0;
            bytePos++;
        }
        return bit;
    };

    auto readBits = [&](uint8_t count) -> uint32_t
    {
        uint32_t value = 0;
        for (uint8_t i = 0; i < count; i++)
        {
            value |= (readBit() << i);
        }
        return value;
    };

    while (bytePos < huffmanData.size())
    {
        // Walk litLen tree to decode a symbol
        HuffmanNode* node = litLenRoot_;
        while (node != nullptr && !node->isLeaf())
        {
            uint32_t bit = readBit();
            node = (bit == 0) ? node->left : node->right;
        }

        if (node == nullptr)
        {
            break;
        }

        uint16_t symbol = node->symbol;

        if (symbol == END_OF_BLOCK)
        {
            // End of block - flush remaining literals
            flushLiterals();
            break;
        }

        if (symbol < END_OF_BLOCK)
        {
            // Literal
            literals.push_back(static_cast<char>(symbol));
            if (literals.size() >= MAX_LITERAL_RUN)
            {
                flushLiterals();
            }
        }
        else
        {
            
            flushLiterals();

            // Decode length
            const auto& lenDecode = LENGTH_DECODE_TABLE[symbol - FIRST_LENGTH_SYMBOL];
            uint16_t length = lenDecode.baseLength;
            if (lenDecode.extraBits > 0)
            {
                length += static_cast<uint16_t>(readBits(lenDecode.extraBits));
            }

            // Walk offset tree to decode offset symbol
            HuffmanNode* offsetNode = offsetRoot_;
            while (offsetNode != nullptr && !offsetNode->isLeaf())
            {
                uint32_t bit = readBit();
                offsetNode = (bit == 0) ? offsetNode->left : offsetNode->right;
            }

            if (offsetNode == nullptr)
            {
                break;
            }

            // Decode offset
            uint16_t offsetSym = offsetNode->symbol;
            uint16_t offset = distTiers[offsetSym].minOffset;
            if (distTiers[offsetSym].extraBitsCount > 0)
            {
                offset += static_cast<uint16_t>(readBits(distTiers[offsetSym].extraBitsCount));
            }

            // Write match in LZSS format: [0x00][length_u16][offset_u16]
            lzssOutput.push_back(0x00);
            lzssOutput.resize(lzssOutput.size() + sizeof(uint16_t));
            std::memcpy(&lzssOutput[lzssOutput.size() - sizeof(uint16_t)], &length, sizeof(uint16_t));
            lzssOutput.resize(lzssOutput.size() + sizeof(uint16_t));
            std::memcpy(&lzssOutput[lzssOutput.size() - sizeof(uint16_t)], &offset, sizeof(uint16_t));
        }
    }

    return lzssOutput;
}

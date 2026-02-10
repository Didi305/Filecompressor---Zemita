#include "codecs/deflate.hpp"

#include "container/container.hpp"

DeflateCodec::DeflateCodec() = default;

auto DeflateCodec::compress(std::span<const char> blockData, BufferedWriter& writer, int number) -> int
{
    // Step 1: LZSS compression (writes to writer's buffer)
    lzss_.compress(blockData, writer, number);

    // Step 2: Extract LZSS output from writer buffer
    auto* buffer = writer.getWriterBuffer();
    auto lzssDataStart = sizeof(BlockHeader);
    auto lzssDataSize = writer.getWritePos() - lzssDataStart;

    // Copy LZSS data out before resetting writer
    auto start = static_cast<ptrdiff_t>(lzssDataStart);
    std::vector<char> lzssData(buffer->begin() + start,
                                buffer->begin() + start + static_cast<ptrdiff_t>(lzssDataSize));

    // Step 3: Huffman encode (scans freqs, builds tree, writes header + bitstream)
    auto huffmanOutput = huffman_.encode(lzssData);

    // Step 4: Reset writer and write Huffman output
    writer.reset();
    writer.write(huffmanOutput.data(), huffmanOutput.size());

    return static_cast<int>(huffmanOutput.size());
}

auto DeflateCodec::decompress(std::span<const char> data, std::vector<char>& full) -> std::vector<char>
{
    // Step 1: Huffman decode (reads header, rebuilds tree, decodes bitstream → LZSS format)
    auto lzssData = huffman_.decode(data);

    // Step 2: LZSS decompress
    return lzss_.decompress(lzssData, full);
}

auto DeflateCodec::afterCompressionInfo() -> void
{
    lzss_.afterCompressionInfo();
}

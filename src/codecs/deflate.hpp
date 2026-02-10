#pragma once

#include "codecs/ICodec.hpp"
#include "codecs/huffmann.hpp"
#include "codecs/lzss.hpp"

class DeflateCodec : public ICodec
{
   public:
    explicit DeflateCodec();

    auto compress(std::span<const char> blockData, BufferedWriter& writer, int number) -> int override;
    auto decompress(std::span<const char> data, std::vector<char>& full) -> std::vector<char> override;
    auto afterCompressionInfo() -> void;

   private:
    LZSSCodec lzss_;
    Huffmann huffman_;
};
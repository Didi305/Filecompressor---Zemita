#pragma once
#include <io/buffered_writer.hpp>
#include <span>
#include <vector>

struct Match;

class ICodec
{
   public:
    virtual ~ICodec() = default;

    virtual int compress(std::span<const char> data, BufferedWriter& writer, int number) = 0;
    virtual std::vector<char> decompress(std::span<const char> data, std::vector<char>& full) = 0;
};

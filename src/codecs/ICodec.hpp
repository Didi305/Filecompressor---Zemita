#pragma once
#include <span>
#include <vector>

struct Match;

class ICodec
{
   public:
    virtual ~ICodec() = default;

    virtual std::vector<Match> compress(std::span<const char> data) = 0;
    virtual std::vector<char> decompress(std::span<const Match> matches) = 0;
};

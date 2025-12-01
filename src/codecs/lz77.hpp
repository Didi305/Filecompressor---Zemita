#pragma once
#include <deque>
#include <optional>

#include "codecs/ICodec.hpp"
#include "zemita/utils.hpp"

class LZ77Codec : public ICodec
{
   private:
    int windowSize_;
    int lookAHead_;
    std::deque<char> searchWindow;
    std::deque<char> aheadBuffer;

   public:
    explicit LZ77Codec(int windowSize, int lookAhead);

    std::vector<Match> compress(std::span<const char> blockData) override;
    std::vector<char> decompress(std::span<const Match> matches) override;
};

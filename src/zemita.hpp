#include <iostream>

#include "codecs/ICodec.hpp"
#include "container/container.hpp"

class ZemitaApp
{
   public:
    explicit ZemitaApp(std::unique_ptr<ICodec> codec);

    void compress(const std::string& input_path) const;
    void decompress(const std::string& input_path) const;

   private:
    std::unique_ptr<ICodec> codec_;
};
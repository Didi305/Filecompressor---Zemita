#include <iostream>
#include "container.hpp"
class ZemitaApp{
public:
    explicit ZemitaApp() = default;

    void compress(const std::string& input_path);
    void decompress(const std::string& input_path);
};
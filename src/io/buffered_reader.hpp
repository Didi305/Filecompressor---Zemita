#include <print>
#include <zemita/utils.hpp>
class BufferedReader
{
   public:
    explicit BufferedReader(const std::string& filePath, uint32_t bufferSize);
    void read(char* toBeReadData, size_t dataSize);
    bool eof() const { return in_.eof() && readPos_ >= bufferFilled_; }

    void refillBuffer();
    std::ifstream in_;
    std::vector<char> buffer_;
    uint32_t bufferSize_;
    uint32_t bufferFilled_;
    uint32_t readPos_;
};
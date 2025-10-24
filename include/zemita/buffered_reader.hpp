#include <print>
#include <zemita/utils.hpp>
class BufferedReader{
public:    
    explicit BufferedReader(const std::string& filePath, uint32_t bufferSize);
    uint32_t read(char* toBeReadData, size_t dataSize);
    bool eof() const {
        return in_.eof() && readPos_ >= bufferFilled_;
    }

    void refillBuffer() {
    if (in_.eof() || in_.fail())
        in_.clear(); // 🔥 Reset fail/eof flags

    buffer_.resize(bufferSize_);
    in_.read(buffer_.data(), buffer_.size());
    bufferFilled_ = static_cast<std::size_t>(in_.gcount());
    readPos_ = 0;

    std::println("refillBuffer(): gcount={}, eof={}, fail={}",
        bufferFilled_, in_.eof(), in_.fail());
    }
private:
    std::ifstream in_;
    std::vector<char> buffer_;
    uint32_t bufferSize_;
    uint32_t bufferFilled_;
    uint32_t readPos_;

};   
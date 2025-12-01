#include "zemita/utils.hpp"

class BufferedWriter{
public:    
    explicit BufferedWriter(const std::string& out, const uint32_t bufferSize);
    void write(const char* toBeWrittenData, size_t dataSize);
    void flush();
private:
    std::ofstream out_;
    uint32_t bufferSize_;
    std::vector<char> buffer_;
    uint16_t writePos_ = 0;
};
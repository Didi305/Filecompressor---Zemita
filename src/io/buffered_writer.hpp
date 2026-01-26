#pragma once

#include "zemita/utils.hpp"

class BufferedWriter
{
   public:
    explicit BufferedWriter(const std::string& out, const uint32_t bufferSize);
    void writeGlobalHeader(const char* toBeWrittenData, size_t dataSize);
    void write(const char* toBeWrittenData, size_t dataSize);
    void flush();
    auto getWriterBuffer() { return &buffer_; }
    void reset();

   private:
    std::ofstream out_;
    uint32_t bufferSize_;
    std::vector<char> buffer_;
    uint32_t writePos_ = 0;
};
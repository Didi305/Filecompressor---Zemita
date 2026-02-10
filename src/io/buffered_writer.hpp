#pragma once

#include "zemita/utils.hpp"

class BufferedWriter
{
   public:
    explicit BufferedWriter(const std::string& out);
    void writeGlobalHeader(const char* toBeWrittenData, size_t dataSize);
    void write(const char* toBeWrittenData, size_t dataSize);
    void flush();
    auto getWriterBuffer() { return &buffer_; }
    [[nodiscard]] auto getWritePos() const { return writePos_; }
    void reset();

   private:
    static constexpr int WRITER_BUFFER_SUZE = 128 * 1024;
    std::ofstream out_;
    std::vector<char> buffer_;
    uint32_t writePos_ = 0;
};
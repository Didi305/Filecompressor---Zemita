#include <cstdint>
#include <zemita/buffered_reader.hpp>

BufferedReader::BufferedReader(const std::string& filePath, uint32_t bufferSize) 
    : in_(filePath, std::ios::binary)
    , buffer_(bufferSize)
    , bufferSize_(bufferSize)
    , bufferFilled_(0)
    , readPos_(0)
    {
        if (!in_.is_open()) {
            std::cerr << "❌ Failed to open file in BufferedReader\n";
        }
        refillBuffer();
    }

uint32_t BufferedReader::read(char* dataReadTo, size_t dataSize){
    std::println("BufferedReader::read() start — bufferFilled_={}, readPos_={}, eof={}", 
              bufferFilled_, readPos_, in_.eof());
    std::println("buffer size when reading: {}", dataSize);
    std::println("first element in the buffer: {}", buffer_[23]);
    uint32_t totalRead = 0;
    while (dataSize > 0){
        if(readPos_ >= bufferFilled_){
            refillBuffer();
            std::println("bufferFilled: {}", bufferFilled_);
            if (bufferFilled_ == 0) {
                std::println("reach the end of file"); 
                break;
            }
        }
        
        size_t availableSpaceForData = bufferFilled_ - readPos_;
        auto toCopy = std::min(availableSpaceForData, dataSize);
        std::memcpy(dataReadTo + totalRead, buffer_.data() + readPos_, toCopy);
        readPos_ += toCopy;
        dataSize -= toCopy;
        totalRead += toCopy;
        std::println("read loop — bufferFilled_={}, readPos_={}, dataSize={} eof={}", 
              bufferFilled_, readPos_, dataSize, in_.eof());
        }
        return totalRead;
    }

   
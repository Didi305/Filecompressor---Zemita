#include "io/buffered_writer.hpp"

BufferedWriter::BufferedWriter(const std::string& filePath, const uint32_t bufferSize)
    : out_(filePath, std::ios::binary)
    , bufferSize_(bufferSize)
    , buffer_(std::vector<char>(bufferSize))
    {
        
}


void BufferedWriter::write(const char* toBeWrittenData, size_t dataSize){
    while (dataSize > 0){
        auto remainingSize = buffer_.size() - writePos_;
        if(remainingSize> 0){
            std::println("adding data to Buffer: {}", toBeWrittenData[0]);
            auto availableSpaceForData = std::min(remainingSize, dataSize);
            std::memcpy(buffer_.data() + writePos_, toBeWrittenData, availableSpaceForData);
            writePos_ += availableSpaceForData;
            dataSize -= availableSpaceForData;
            toBeWrittenData += availableSpaceForData;
            std::println("buffer fill = {} bytes", writePos_);
            
            if(writePos_ == buffer_.size()){
                out_.write(buffer_.data(), writePos_); 
                out_.flush();
                writePos_ = 0;
            }
            std::println("buffer fill = {} bytes", buffer_.data());
        }
    }
    
}

void BufferedWriter::flush(){
    out_.write(buffer_.data(), writePos_);
    out_.flush();
    writePos_ = 0;
}
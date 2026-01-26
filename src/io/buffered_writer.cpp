#include "io/buffered_writer.hpp"

#include "container/container.hpp"
#include "tracy/Tracy.hpp"

BufferedWriter::BufferedWriter(const std::string& filePath, const uint32_t bufferSize)
    : out_(filePath, std::ios::binary), bufferSize_(bufferSize), buffer_(std::vector<char>(bufferSize))
{
}

void BufferedWriter::reset()
{
    buffer_.clear();
    buffer_.resize(sizeof(BlockHeader));
    writePos_ = sizeof(BlockHeader);
}

void BufferedWriter::writeGlobalHeader(const char* toBeWrittenData, size_t dataSize)
{
    out_.write(toBeWrittenData, sizeof(GlobalHeader));
    out_.flush();
}

void BufferedWriter::write(const char* toBeWrittenData, size_t dataSize)
{
    while (dataSize > 0)
    {
        auto remainingSize = buffer_.capacity() - writePos_;
        if (remainingSize > 0)
        {
            auto availableSpaceForData = std::min(remainingSize, dataSize);
            buffer_.resize(buffer_.size() + availableSpaceForData);
            std::memcpy(buffer_.data() + writePos_, toBeWrittenData, availableSpaceForData);
            writePos_ += availableSpaceForData;
            dataSize -= availableSpaceForData;

            if (writePos_ == buffer_.capacity())
            {
                out_.write(buffer_.data(), writePos_);
                out_.flush();
                writePos_ = 0;
            }
        }
    }
}

void BufferedWriter::flush()
{
    out_.write(buffer_.data(), writePos_);
    out_.flush();
    writePos_ = 0;
}
#include <cstdint>
#include <io/buffered_reader.hpp>

BufferedReader::BufferedReader(const std::string& filePath, uint32_t bufferSize)
    : in_(filePath, std::ios::binary), buffer_(bufferSize), bufferSize_(bufferSize), bufferFilled_(0), readPos_(0)
{
    if (!in_.is_open())
    {
        std::cerr << "❌ Failed to open file in BufferedReader\n";
    }
    refillBuffer();
}

void BufferedReader::refillBuffer()
{
    if (in_.eof() || in_.fail())
        in_.clear();  // 🔥 Reset fail/eof flags

    buffer_.resize(bufferSize_);
    in_.read(buffer_.data(), buffer_.size());
    bufferFilled_ = static_cast<std::size_t>(in_.gcount());
    readPos_ = 0;
}

void BufferedReader::read(char* dataReadTo, size_t dataSize)
{
    uint32_t totalRead = 0;
    while (dataSize > 0)
    {
        if (readPos_ >= bufferFilled_)
        {
            refillBuffer();

            if (bufferFilled_ == 0)
            {
                break;
            }
        }

        size_t availableSpaceForData = bufferFilled_ - readPos_;
        auto toCopy = std::min(availableSpaceForData, dataSize);
        std::memcpy(dataReadTo + totalRead, buffer_.data() + readPos_, toCopy);
        readPos_ += toCopy;
        dataSize -= toCopy;
        totalRead += toCopy;
    }
}

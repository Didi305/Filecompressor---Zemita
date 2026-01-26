#include "container.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#include "tracy/Tracy.hpp"

ContainerWriter::ContainerWriter(std::string& filePath, const GlobalHeader& gHeader)
    : writer_(filePath, WRITER_BUFFER_SIZE)
{
    std::println("tried adding header");
    writer_.writeGlobalHeader(reinterpret_cast<const char*>(&gHeader), sizeof(gHeader));
}

ContainerWriter::~ContainerWriter()
{
    if (out_.is_open())
        out_.close();
}

void ContainerWriter::finalize()
{
    writer_.flush();
}

void ContainerWriter::write(const char* toBeWrittenData, size_t dataSize)
{
    writer_.write(toBeWrittenData, dataSize);
}

void ContainerWriter::writeBlock(BlockHeader& bHeader)
{
    auto* buffer = writer_.getWriterBuffer();
    memcpy(buffer->data(), reinterpret_cast<char*>(&bHeader), sizeof(bHeader));
    writer_.flush();
    writer_.reset();
}

#include <filesystem>
ContainerReader::ContainerReader(const std::string& input_path) : reader_(input_path, READER_BUFFER_SIZE)
{
}
auto ContainerReader::readGlobalHeader(const std::string& path) -> GlobalHeader
{
    GlobalHeader gHeader{};
    std::println("size directly  from file: {}", std::filesystem::file_size(path));
    reader_.read(reinterpret_cast<char*>(&gHeader.magicBytes), sizeof(gHeader));

    std::println("size of the file from the global header: {}", gHeader.original_size);
    std::println("extensssssss: {}", gHeader.original_extension);
    auto numberOfBlocks = std::ceil(double(gHeader.original_size) / gHeader.block_size);
    blocks.resize(numberOfBlocks);
    std::println("size of t: {}", blocks.size());
    return gHeader;
}
ContainerReader::~ContainerReader()
{
    if (in_.is_open())
    {
        in_.close();
    }
}

auto ContainerReader::readAllBlocks() -> std::map<BlockHeader, std::vector<char>>
{
    std::cout << "readAllBlocks called!\n";
    std::map<BlockHeader, std::vector<char>> blockMap;
    uint32_t numberOfBlocks = blocks.size();
    size_t iterator{};
    while (iterator < numberOfBlocks)
    {
        reader_.read(reinterpret_cast<char*>(&blocks[iterator]), sizeof(BlockHeader));
        std::println("Reading block {} with compressed_size {}", blocks[iterator].block_seq_num,
                     blocks[iterator].compressed_size);

        std::vector<char> blockData(blocks[iterator].compressed_size);
        reader_.read(reinterpret_cast<char*>(blockData.data()), blocks[iterator].compressed_size);
        blockMap[blocks[iterator]] = std::move(blockData);
        iterator++;
    }

    std::cout << "readAllBlocks returning!\n";
    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockMap;
}
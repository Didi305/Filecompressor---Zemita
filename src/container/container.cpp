#include "container.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#include "tracy/Tracy.hpp"

ContainerWriter::ContainerWriter(std::string& filePath, const GlobalHeader& gHeader) : writer_(filePath)
{
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
ContainerReader::ContainerReader(const std::string& input_path) : reader_(input_path)
{
}
auto ContainerReader::readGlobalHeader(const std::string& path) -> GlobalHeader
{
    GlobalHeader gHeader{};
    reader_.read(reinterpret_cast<char*>(&gHeader.magicBytes), sizeof(gHeader));
    auto numberOfBlocks = std::ceil(double(gHeader.original_size) / gHeader.block_size);
    blocks.resize(numberOfBlocks);
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
    std::map<BlockHeader, std::vector<char>> blockMap;
    uint32_t numberOfBlocks = blocks.size();
    size_t iterator{};
    while (iterator < numberOfBlocks)
    {
        reader_.read(reinterpret_cast<char*>(&blocks[iterator]), sizeof(BlockHeader));

        std::vector<char> blockData(blocks[iterator].compressed_size);
        reader_.read(reinterpret_cast<char*>(blockData.data()), blocks[iterator].compressed_size);
        blockMap[blocks[iterator]] = std::move(blockData);
        iterator++;
    }

    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockMap;
}
#include "container.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#include "tracy/public/tracy/Tracy.hpp"

ContainerWriter::ContainerWriter(std::string& filePath, const GlobalHeader& gHeader) : writer_(filePath, BLOCK_SIZE)
{
    std::println("tried adding header");
    writer_.write(reinterpret_cast<const char*>(&gHeader), sizeof(gHeader));
    writer_.flush();
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

void ContainerWriter::writeBlock(BlockHeader& bHeader, std::vector<Match>& matches)
{
    writer_.write(reinterpret_cast<char*>(&bHeader.block_seq_num), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.uncompressed_size), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.compressed_size), sizeof(uint32_t));
    for (auto& match : matches)
    {
        writer_.write(reinterpret_cast<const char*>(&match), sizeof(Match));
    }
}

#include <filesystem>
ContainerReader::ContainerReader(const std::string& input_path) : reader_(input_path, 4 * 1024)
{
}
GlobalHeader ContainerReader::readGlobalHeader(const std::string& path)
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

std::map<BlockHeader, char*> ContainerReader::readAllBlocks()
{  // after global header
    std::cout << "readAllBlocks called!\n";
    std::map<BlockHeader, char*> blockMap;
    uint32_t numberOfBlocks = blocks.size();
    std::print("seq0num: ", blocks[0].block_seq_num);
    size_t iterator{};
    while (iterator < numberOfBlocks)
    {
        reader_.read(reinterpret_cast<char*>(&blocks[iterator]), sizeof(BlockHeader));
        std::print("seq num: ", blocks[iterator].block_seq_num);
        char* blockData = new char[blocks[iterator].compressed_size];
        reader_.read(blockData, blocks[iterator].compressed_size);
        blockMap[blocks[iterator]] = blockData;
        std::println("Data read from block {} is: {}", blocks[iterator].block_seq_num, blockData);
        iterator++;
    }

    std::cout << "readAllBlocks returning!\n";
    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockMap;
}
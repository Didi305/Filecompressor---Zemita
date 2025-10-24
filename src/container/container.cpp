#include <cmath>
#include <cstdint>
#include <print>
#include <string>
#include <fstream>

#include <iostream>
#include <zemita/container.hpp>


ContainerWriter::ContainerWriter(std::string& filePath, const GlobalHeader& gHeader) 
    : writer_(filePath, 2*1024)
    {
    std::println("tried adding header");
    writer_.write(reinterpret_cast<const char*>(&gHeader), sizeof(gHeader));
}

ContainerWriter::~ContainerWriter() {
    if (out_.is_open()) out_.close();
}

void ContainerWriter::finalize(){
    writer_.flush();
}

void ContainerWriter::writeBlock(BlockHeader& bHeader, char* data){
    writer_.write(reinterpret_cast<char*>(&bHeader.block_seq_num), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.uncompressed_size), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.compressed_size), sizeof(uint32_t));
    writer_.write(data, bHeader.compressed_size);
}

#include <filesystem>
ContainerReader::ContainerReader(const std::string& input_path) 
    : reader_(input_path, 4*1024) 
{
    GlobalHeader gHeader{};
    std::println("size directly  from file: {}", std::filesystem::file_size(input_path));
    auto dataRead = reader_.read(reinterpret_cast<char*>(&gHeader.magicBytes), sizeof(gHeader));
    std::println("data: {}", dataRead);
    std::println("size of the file from the global header: {}", gHeader.original_size);
    auto numberOfBlocks = std::ceil(double (gHeader.original_size) / gHeader.block_size);
    blocks.resize(numberOfBlocks);
    std::println("size of t: {}", blocks.size());
}

ContainerReader::~ContainerReader() {
    if (in_.is_open()) in_.close();
}



std::map<BlockHeader, char*> ContainerReader::readAllBlocks() { // after global header
    std::cout << "readAllBlocks called!\n";
    std::map<BlockHeader, char*> blockMap;
    uint32_t numberOfBlocks = blocks.size();
    std::print("seq0num: ", blocks[0].block_seq_num);
    int i = 0;
    while (i < numberOfBlocks) {
        reader_.read(reinterpret_cast<char*>(&blocks[i]), sizeof(BlockHeader));
        std::print("seq num: ", blocks[i].block_seq_num);
        char* blockData = new char[blocks[i].compressed_size];
        reader_.read(blockData, blocks[i].compressed_size);
        blockMap[blocks[i]] = blockData;
        std::println("Data read from block {} is: {}", blocks[i].block_seq_num, blockData);
        i++;
    }

    std::cout << "readAllBlocks returning!\n";
    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockMap;
}
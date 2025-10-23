#include <cstdint>
#include <print>
#include <string>
#include <fstream>

#include <iostream>
#include <zemita/container.hpp>

ContainerWriter::ContainerWriter(const std::string& filePath, const GlobalHeader& gHeader) 
    : out_(filePath, std::ios::binary | std::ios::trunc)
    , writer_(&out_, 2*1024)
    {
    if (!out_) throw std::runtime_error("Failed to open output file: " + filePath);
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


ContainerReader::ContainerReader(const std::string& input_path) 
    : reader_(input_path, 4*1024) 
{
    GlobalHeader gHeader{};
    std::cout << "sizeof(GlobalHeader) = " << sizeof(GlobalHeader) << '\n';
    in_.read(reinterpret_cast<char*>(&gHeader), sizeof(gHeader));
}

ContainerReader::~ContainerReader() {
    if (in_.is_open()) in_.close();
}



BlockHeader* ContainerReader::readAllBlocks() { // after global header
    std::cout << "readAllBlocks called!\n";
    uint32_t numberOfBlocks = blocks.size();
    int i = 0;
    while (i < numberOfBlocks) {
        reader_.read(reinterpret_cast<char*>(&blocks[i].block_seq_num), sizeof(uint32_t));
        std::print("seq num: ", blocks[i].block_seq_num);
        reader_.read(reinterpret_cast<char*>(&blocks[i].uncompressed_size), sizeof(uint32_t));
        reader_.read(reinterpret_cast<char*>(&blocks[i].compressed_size), sizeof(uint32_t));
        

        std::cout << "PUSHING!! !\n";
        numberOfBlocks--;
    }

    BlockHeader* blockis = blocks.data();
    std::cout << "readAllBlocks returning!\n";
    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockis;
}
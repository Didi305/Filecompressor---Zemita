#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "zemita/container.hpp"

ContainerWriter::ContainerWriter(const std::string& out_path) : out_(out_path, std::ios::binary){
    if (!out_) throw std::runtime_error("Failed to open output file: " + out_path);
}

ContainerWriter::~ContainerWriter() {
    if (out_.is_open()) out_.close();
}

void ContainerWriter::writeGlobalHeader(GlobalHeader& gHeader){
    
    out_.write(reinterpret_cast<char*>(&gHeader), sizeof(gHeader));

}

void ContainerWriter::writeBlock(BlockHeader& bHeader){
    out_.write(reinterpret_cast<char*>(&bHeader.block_seq_num), sizeof(uint32_t));
    out_.write(reinterpret_cast<char*>(&bHeader.uncompressed_size), sizeof(uint32_t));
    out_.write(reinterpret_cast<char*>(&bHeader.compressed_size), sizeof(uint32_t));
    out_.write(bHeader.data, bHeader.compressed_size);
}


ContainerReader::ContainerReader(const std::string& input_path) : in_(std::ifstream(input_path)) {
    if (!in_) throw std::runtime_error("no input path");
}

ContainerReader::~ContainerReader() {
    if (in_.is_open()) in_.close();
}

GlobalHeader ContainerReader::readGlobalHeader() {
    std::cout << "[readGlobalHeader START]\n" << std::flush;

    if (!in_) {
        std::cerr << "❌ File stream invalid\n";
    }

    GlobalHeader gHeader{};
    std::cout << "sizeof(GlobalHeader) = " << sizeof(GlobalHeader) << '\n';
    in_.read(reinterpret_cast<char*>(&gHeader), sizeof(gHeader));
    return gHeader;
}



std::vector<BlockHeader> ContainerReader::readAllBlocks(int numberOfBlocks){
    std::cout << "number of blockis: " << numberOfBlocks << "\n";
    std::vector<BlockHeader> blocks(numberOfBlocks);
    int offset = 22;
    while (numberOfBlocks > 0 ){
        in_.seekg(offset, in_.beg);
        BlockHeader bHeader{};
        in_.read(reinterpret_cast<char*>(&bHeader), sizeof(bHeader));
        blocks.emplace(blocks.begin(), bHeader);
        numberOfBlocks--;
        std::cout << "size of in loop: " << numberOfBlocks << " is " << sizeof(bHeader) << "\n";
        offset += sizeof(bHeader);
        std::cout << "offset in loop: " << numberOfBlocks << " is " << offset << "\n" ;
    }
    return blocks;
}
#include <cstdint>
#include <print>
#include <string>
#include <fstream>

#include <iostream>
#include <zemita/container.hpp>

ContainerWriter::ContainerWriter(const std::string& filePath, const GlobalHeader& gHeader) 
    : out_(filePath, std::ios::binary)
    , writer_(&out_, 4*1024)
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

void ContainerWriter::writeBlock(BlockHeader& bHeader){
    writer_.write(reinterpret_cast<char*>(&bHeader.block_seq_num), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.uncompressed_size), sizeof(uint32_t));
    writer_.write(reinterpret_cast<char*>(&bHeader.compressed_size), sizeof(uint32_t));
    writer_.write(bHeader.data, bHeader.compressed_size);
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



BlockHeader* ContainerReader::readAllBlocks(std::vector<BlockHeader>& blocks, uint32_t numberOfBlocks) {
    int offset = 22; // after global header
    std::cout << "readAllBlocks called!\n";


    while (numberOfBlocks > 0) {
        in_.seekg(offset, std::ios::beg);
        BlockHeader bHeader{};
        in_.read(reinterpret_cast<char*>(&bHeader.block_seq_num), sizeof(uint32_t));
        in_.read(reinterpret_cast<char*>(&bHeader.uncompressed_size), sizeof(uint32_t));
        in_.read(reinterpret_cast<char*>(&bHeader.compressed_size), sizeof(uint32_t));
        bHeader.data = new char[bHeader.compressed_size];
        in_.read(bHeader.data, bHeader.compressed_size);

        blocks.push_back(bHeader);
        std::cout << "PUSHING!! !\n";
        offset += 12 + bHeader.compressed_size;
        numberOfBlocks--;
    }

    BlockHeader* blockis = blocks.data();
    std::cout << "readAllBlocks returning!\n";
    std::cout << "Number of blocks read = " << blocks.size() << "\n";
    return blockis;
}
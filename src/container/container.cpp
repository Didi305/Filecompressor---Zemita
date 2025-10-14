#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "zemita/container.hpp"

ContainerWriter::ContainerWriter(const std::string& out_path) : out_(out_path, std::ios::binary | std::ios::trunc){
    if (!out_) throw std::runtime_error("Failed to open output file: " + out_path);
}

ContainerWriter::~ContainerWriter() {
    if (out_.is_open()) out_.close();
}

void ContainerWriter::writeGlobalHeader(GlobalHeader& gHeader){
    out_.write(gHeader.magicBytes, 4);
    out_.write(reinterpret_cast<char*>(&gHeader.original_size), sizeof(gHeader.original_size));
    out_.write(reinterpret_cast<char*>(&gHeader.checksum_id), sizeof(gHeader.checksum_id));
    out_.write(reinterpret_cast<char*>(&gHeader.block_size), sizeof(gHeader.block_size));
    out_.write(reinterpret_cast<char*>(&gHeader.version), sizeof(gHeader.version));
    out_.write(reinterpret_cast<char*>(&gHeader.original_size), sizeof(gHeader.original_size));
}

void ContainerWriter::writeBlock(BlockHeader& bHeader, std::vector<char>& data){
    out_.write(reinterpret_cast<char*>(&bHeader), sizeof(bHeader));
    out_.write(data.data(), sizeof(data));
}
#include <zemita/zemita.hpp>
#include <print>
#include <filesystem>
#include <iostream>
#include <assert.h>
#include "zemita/utils.hpp"
void ZemitaApp::compress(const std::string& input_path) {
    namespace fs = std::filesystem;
    std::ifstream input_reader(input_path, std::ios::binary);
    if (!fs::exists(input_path)) {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }
    fs::path in_path(input_path);
    // Prepare output path
    std::string filePath = in_path.stem().string() + ".zem";

    // Initialize gHeader
    GlobalHeader gHeader;
    gHeader.original_size = fs::file_size(input_path);
    ContainerWriter writer(filePath, gHeader);
    std::cout << "Opening path: " << std::filesystem::absolute(input_path) << "\n";
    BufferedReader reader(input_path, 6*1024);
    
    int i = 0;
    uint32_t uncompressed_size = gHeader.original_size;
    while (uncompressed_size > 0){

        BlockHeader bHeader{};
        bHeader.block_seq_num = i;
        bHeader.compressed_size = std::min(uncompressed_size, gHeader.block_size);
        
        bHeader.uncompressed_size = uncompressed_size;
        uint32_t diff = gHeader.original_size - bHeader.uncompressed_size;
        uncompressed_size -= bHeader.compressed_size;
    
        
        std::vector<char> buffer(bHeader.compressed_size);
        auto dataRead = reader.read(buffer.data(), bHeader.compressed_size);
        for (int i = 0; i < 16; ++i){
            std::printf("%02X ", static_cast<unsigned char>(buffer[i]));
            std::printf("\n");
        }
        char* data = buffer.data();
        writer.writeBlock(bHeader, data);
        std::println("uncompressed size left: {}", uncompressed_size);
        i++;
    }
    writer.finalize();
}   


void ZemitaApp::decompress(const std::string& input_path) {
    namespace fs = std::filesystem;
    std::map <BlockHeader, char*> blockMap;
    if (!fs::exists(input_path)) {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }
    fs::path in_path(input_path);
    std::string filePath = in_path.stem().string() + "DE.txt";
    BufferedWriter writer(filePath, 4*1024);
    ContainerReader reader(input_path);

    blockMap = reader.readAllBlocks();

    for (auto it = blockMap.begin(); it != blockMap.end(); ++it){
        std::println("writing data from block: {} that has the following data: {}", it->first.block_seq_num, it->second);
        writer.write(it->second, it->first.compressed_size);
    }
    writer.flush();
    std::print("typ shit");
}
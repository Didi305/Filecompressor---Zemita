#include <zemita/zemita.hpp>
#include <print>
#include <filesystem>
#include <iostream>
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
    try {
        // Create writer and write gHeader
        std::cout << "✅ Wrote .zem gHeader for file: " << filePath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Compression failed: " << e.what() << "\n";
    }
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
        bHeader.data = Utils::getDataBlock(input_reader, gHeader.original_size - bHeader.uncompressed_size, bHeader.compressed_size, buffer);
        writer.writeBlock(bHeader);
        i++;
    }
    writer.finalize();
}   

void ZemitaApp::decompress(const std::string& input_path) {
    namespace fs = std::filesystem;

    if (!fs::exists(input_path)) {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }


    ContainerReader reader(input_path);
    try {
        std::cerr << "File found: " << input_path << "\n";
        GlobalHeader gHeader = reader.readGlobalHeader();
        auto numberOfBlocks = std::ceil((double)gHeader.original_size / (2 * 1024));
        Utils::printIntervalFromContainer(gHeader.magicBytes, 0 , 3);
        std::cout << "original size from reading: " << gHeader.original_size << "\n";
        std::cout << "block size from reading: " << gHeader.block_size << "\n";
        std::cout << "version from reading: " << gHeader.version << "\n";
        std::cout << "codec from reading: " << gHeader.codec_id << "\n";
        std::cout << "numer of blockers" << numberOfBlocks << "\n";
        std::vector<BlockHeader> blocks;
    
        BlockHeader* blockis = reader.readAllBlocks(blocks, numberOfBlocks);

        for (auto i = 0; i < numberOfBlocks; ++i) {
            auto& b = blockis[i];
            std::cout << "uncompressed size in the block: " << b.block_seq_num << " is " << b.uncompressed_size << " and the data is: " << b.data << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ DECompression failed: " << e.what() << "\n";
    }
    std::print("typ shit");
}
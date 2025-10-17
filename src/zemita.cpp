#include <zemita/zemita.hpp>

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
    std::string output_path = in_path.stem().string() + ".zem";

    // Initialize gHeader
    GlobalHeader gHeader;
    gHeader.original_size = fs::file_size(input_path);
    std::cout << "file size is: " << gHeader.original_size;
    gHeader.codec_id = 0; // for now, no compression yet
    ContainerWriter writer(output_path);
    try {
        // Create writer and write gHeader
        writer.writeGlobalHeader(gHeader);
        std::cout << "✅ Wrote .zem gHeader for file: " << output_path << "\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Compression failed: " << e.what() << "\n";
    }
    int i = 0;
    uint32_t uncompressed_size = gHeader.original_size;
    while (uncompressed_size > 0){
        std::cout << "hamid original \n " << gHeader.original_size;
        BlockHeader bHeader{};
        bHeader.block_seq_num = i;
        bHeader.compressed_size = std::min(uncompressed_size, gHeader.block_size);
        uncompressed_size -= bHeader.compressed_size;
        bHeader.uncompressed_size = uncompressed_size;
        auto data = Utils::getDataBlock(input_reader, gHeader.original_size - bHeader.uncompressed_size, bHeader.compressed_size);
        writer.writeBlock(bHeader, data);    
    }
}   

void ZemitaApp::decompress(const std::string& input_path) {
    namespace fs = std::filesystem;

    if (!fs::exists(input_path)) {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }


    // Initialize gHeader
    try {
        std::cerr << "File found: " << input_path << "\n";
        ContainerReader reader(input_path);
        GlobalHeader gHeader = reader.readGlobalHeader();
        std::cout << gHeader.magicBytes;
    } catch (const std::exception& e) {
        std::cerr << "❌ DECompression failed: " << e.what() << "\n";
    }
}
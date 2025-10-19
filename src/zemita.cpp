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
        std::cout << "compressed size: " << bHeader.compressed_size << " and uncompressed size: " << uncompressed_size << " and original size is: " << gHeader.original_size << std::endl;
        bHeader.uncompressed_size = uncompressed_size;
        uint32_t diff = gHeader.original_size - bHeader.uncompressed_size;
        uncompressed_size -= bHeader.compressed_size;
        std::cout << "compressed size: " << bHeader.compressed_size << " and uncompressed size: " << bHeader.uncompressed_size << " and original size is: " << gHeader.original_size << std::endl;
        
        std::vector<char> buffer(bHeader.compressed_size);
        bHeader.data = Utils::getDataBlock(input_reader, gHeader.original_size - bHeader.uncompressed_size, bHeader.compressed_size, buffer);
        std::cout << "data d zeb after getting it: " << bHeader.data << std::endl;
        writer.writeBlock(bHeader);
        i++;
    }
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
        Utils::printIntervalFromContainer(gHeader.magicBytes, 0 , 3);
        std::cout << "original size from reading: " << gHeader.original_size << "\n";
        std::cout << "block size from reading: " << gHeader.block_size << "\n";
        std::cout << "version from reading: " << gHeader.version << "\n";
        std::cout << "codec from reading: " << gHeader.codec_id << "\n";
        std::cout << "numer of blockers" << std::ceil((double)652392 / (64 * 1024)) << "\n";
        std::vector<BlockHeader> blocks = reader.readAllBlocks(std::ceil(std::ceil((double)gHeader.original_size / (64 * 1024))));
        for (auto b : blocks){
            std::cout << "uncompressed size in the block: " << b.block_seq_num << " is " << b.uncompressed_size << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ DECompression failed: " << e.what() << "\n";
    }
}
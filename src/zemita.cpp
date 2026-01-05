#include <cassert>
#include <filesystem>
#include <iostream>
#include <print>
#include <zemita.hpp>

#include "tracy/public/tracy/Tracy.hpp"

ZemitaApp::ZemitaApp(std::unique_ptr<ICodec> codec) : codec_(std::move(codec))
{
    std::println("Zemita App Initiated with Codec: ");
}

void ZemitaApp::compress(const std::string& input_path) const
{
    ZoneScoped;
    namespace fs = std::filesystem;
    std::ifstream input_reader(input_path, std::ios::binary);
    if (!fs::exists(input_path))
    {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }
    fs::path in_path(input_path);
    // Prepare output path
    std::string filePath = in_path.stem().string() + ".zem";

    // Initialize gHeader
    GlobalHeader gHeader;
    gHeader.original_size = fs::file_size(input_path);
    std::snprintf(gHeader.original_extension, sizeof(gHeader.original_extension), "%s",
                  in_path.extension().string().erase(0, 1).c_str());
    ContainerWriter writer(filePath, gHeader);
    std::cout << "Opening path: " << std::filesystem::absolute(input_path) << "\n";

    BufferedReader reader(input_path, READER_BUFFER_SIZE);
    int iterator = 0;
    uint32_t uncompressed_size = gHeader.original_size;
    while (uncompressed_size > 0)
    {
        BlockHeader bHeader{};
        bHeader.block_seq_num = iterator;
        bHeader.compressed_size = std::min(uncompressed_size, gHeader.block_size);

        bHeader.uncompressed_size = uncompressed_size;

        uncompressed_size -= bHeader.compressed_size;

        std::vector<char> buffer(bHeader.compressed_size);
        reader.read(buffer.data(), bHeader.compressed_size);
        writer.writeBlock(bHeader, codec_->compress(buffer));

        iterator++;
    }
    writer.finalize();
}

void ZemitaApp::decompress(const std::string& input_path) const
{
    namespace fs = std::filesystem;
    std::map<BlockHeader, char*> blockMap;
    if (!fs::exists(input_path))
    {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }
    fs::path in_path(input_path);

    ContainerReader reader(input_path);
    GlobalHeader gHeader = reader.readGlobalHeader(input_path);

    std::string filePath = in_path.stem().string() + gHeader.original_extension;
    BufferedWriter writer(filePath, WRITER_BUFFER_SIZE);
    blockMap = reader.readAllBlocks();
    std::println("extension in header: {}", gHeader.original_extension);
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it)
    {
        std::println("writing data from block: {} that has the following data: {}", it->first.block_seq_num,
                     it->second);
        writer.write(it->second, it->first.compressed_size);
    }
    writer.flush();
    std::print("typ shit");
}
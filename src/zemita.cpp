#include <cassert>
#include <filesystem>
#include <iostream>
#include <print>
#include <zemita.hpp>

#include "codecs/deflate.hpp"
#include "tracy/Tracy.hpp"

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
    std::string filePath = (in_path.parent_path() / (in_path.stem().string() + ".zem")).string();

    // Initialize gHeader
    GlobalHeader gHeader;
    gHeader.original_size = fs::file_size(input_path);

    std::snprintf(gHeader.original_extension, sizeof(gHeader.original_extension), "%s",
                  in_path.extension().string().c_str());
    ContainerWriter writer(filePath, gHeader);
    std::cout << "Opening path: " << std::filesystem::absolute(input_path) << "\n";

    BufferedReader reader(input_path);
    int iterator = 0;
    uint32_t uncompressed_size = gHeader.original_size;
    while (uncompressed_size > 0)
    {
        BlockHeader bHeader{};
        bHeader.block_seq_num = iterator;

        bHeader.uncompressed_size = uncompressed_size;
        auto chtari = std::min(BLOCK_SIZE, static_cast<int>(uncompressed_size));
        uncompressed_size -= chtari;

        std::vector<char> buffer(chtari);
        reader.read(buffer.data(), chtari);
        bHeader.compressed_size = codec_->compress(buffer, *writer.getWriter(), iterator);
        writer.writeBlock(bHeader);
        iterator++;
    }
    writer.finalize();
    static_cast<DeflateCodec*>(codec_.get())->afterCompressionInfo();
}

void ZemitaApp::decompress(const std::string& input_path) const
{
    namespace fs = std::filesystem;
    if (!fs::exists(input_path))
    {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }
    fs::path in_path(input_path);

    ContainerReader reader(input_path);
    GlobalHeader gHeader = reader.readGlobalHeader(input_path);

    std::string filePath =
        (in_path.parent_path() / (in_path.stem().string() + "_TEST" + gHeader.original_extension)).string();
    BufferedWriter writer(filePath);
    auto blockMap = reader.readAllBlocks();

    std::vector<char> fullFile;
    for (auto& [header, data] : blockMap)
    {
        auto decompressed = codec_->decompress(data, fullFile);
        writer.write(decompressed.data(), decompressed.size());
    }
    writer.flush();
}
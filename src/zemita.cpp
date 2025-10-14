#include <zemita/zemita.hpp>

#include <filesystem>
#include <iostream>

void ZemitaApp::compress(const std::string& input_path) {
    namespace fs = std::filesystem;

    if (!fs::exists(input_path)) {
        std::cerr << "❌ File not found: " << input_path << "\n";
        return;
    }

    // Prepare output path
    std::string output_path = input_path + ".zem";

    // Initialize header
    GlobalHeader header;
    header.original_size = fs::file_size(input_path);
    header.codec_id = 0; // for now, no compression yet

    try {
        // Create writer and write header
        ContainerWriter writer(output_path);
        writer.writeGlobalHeader(header);
        std::cout << "✅ Wrote .zem header for file: " << output_path << "\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Compression failed: " << e.what() << "\n";
    }
}

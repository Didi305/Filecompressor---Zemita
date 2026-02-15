#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include "codecs/deflate.hpp"
#include "zemita.hpp"

static constexpr std::string_view USAGE =
    "Usage: zemita <command> <file>\n"
    "\n"
    "Commands:\n"
    "  compress   <file>   Compress a file into .zem format\n"
    "  decompress <file>   Decompress a .zem file back to original\n"
    "\n"
    "Examples:\n"
    "  zemita compress   photo.png\n"
    "  zemita decompress photo.zem\n";

auto main(int argc, char** argv) -> int
{
    if (argc < 2)
    {
        std::cerr << USAGE;
        return 1;
    }

    std::string_view command = argv[1];

    if (command == "--help" || command == "-h")
    {
        std::cout << USAGE;
        return 0;
    }

    if (argc < 3)
    {
        std::cerr << "Error: missing file argument.\n\n" << USAGE;
        return 1;
    }

    std::string filePath = argv[2];

    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "Error: file not found: " << filePath << "\n";
        return 1;
    }

    try
    {
        auto codec = std::make_unique<DeflateCodec>();
        ZemitaApp app(std::move(codec));

        if (command == "compress" || command == "c")
        {
            app.compress(filePath);
        }
        else if (command == "decompress" || command == "d")
        {
            app.decompress(filePath);
        }
        else
        {
            std::cerr << "Error: unknown command '" << command << "'\n\n" << USAGE;
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

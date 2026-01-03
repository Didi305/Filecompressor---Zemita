#include "CLI/zemita_main.hpp"

#include "codecs/lz77.hpp"
#include "zemita.hpp"
#include "zemita/utils.hpp"

#ifdef _WIN32
#include <windows.h>   // ← first
#include <commdlg.h>   // ← second
#endif

std::string openFileDialog()
{
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};  // Use {} instead of {0}
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Select input file";
    if (GetOpenFileNameA(&ofn))
        return filename;
    else
        return "";
#else
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    return path;
#endif
}

int main(int argc, char** argv)
{
    auto codec_ptr = std::make_unique<LZ77Codec>(SEARCH_WINDOW_SIZE, LOOKAHEAD_BUFFER_SIZE);
    std::string filePath = openFileDialog();
    if (filePath.empty())
    {
        std::cerr << "❌ No file selected.\n";
        return 1;
    }

    try
    {
        ZemitaApp app(std::move(codec_ptr));
        app.compress(filePath);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

/* void test_lz77(const std::string& input)
{
    LZ77Codec codec(SEARCH_WINDOW_SIZE, LOOKAHEAD_BUFFER_SIZE);

    auto compressed = codec.compress(std::span<const char>(input.data(), input.size()));
};
// NOLINTNEXTLINE(bugprone-exception-escape)
auto main(int argc, char** argv) -> int
{
    std::print("Cpp version: {}", __cplusplus);

    /* test_lz77("A");               // one char
    test_lz77("ABCDEFG");         // no repeats
    test_lz77("AAAAAA");          // repeated
    test_lz77("ABCLKJPSABCABC");  // overlapping
    test_lz77("abracadabracbhnklaca");  // classic
    test_lz77("hello hello");           // common words
} */
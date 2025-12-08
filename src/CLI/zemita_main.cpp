#include "CLI/zemita_main.hpp"

#include <iostream>

#include "codecs/lz77.hpp"
#include "zemita.hpp"
#include "zemita/utils.hpp"

/*#ifdef _WIN32
#include <commdlg.h>
#include <windows.h>

#endif

std::string openFileDialog()
{
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Select input file";
    if (GetOpenCardNameA(&ofn))
        return filename;
    else
        return "";
#else
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    return path;
#endif
}*/

void test_lz77(const std::string& input)
{
    LZ77Codec codec(SEARCH_WINDOW_SIZE, LOOKAHEAD_BUFFER_SIZE);

    auto compressed = codec.compress(std::span<const char>(input.data(), input.size()));
    int iterator = 0;
    for (auto& m : compressed)
    {
        std::cout << "\n";
        std::print("Byte {}: {}", iterator, m.offset);
        std::print("{}", m.length);
        std::print("{}", m.next);
        std::cout << "\n";
        iterator++;
    }
};

int main(int argc, char** argv)
{
    std::print("Cpp version: {}", __cplusplus);
    /* test_lz77("");             // empty
    test_lz77("A");            // one char
    test_lz77("ABCDEFG");      // no repeats
    test_lz77("AAAAAA");       // repeated */
    test_lz77("ABCABCABC");    // overlapping
    test_lz77("abracadabra");  // classic
    test_lz77("hello hello");  // common words
}
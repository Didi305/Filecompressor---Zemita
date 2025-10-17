#include "zemita/zemita.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

std::string openFileDialog() {
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {0};
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

int main(int argc, char** argv) {
    std::string filePath = openFileDialog();
    if (filePath.empty()) {
        std::cerr << "❌ No file selected.\n";
        return 1;
    }

    try {
        ZemitaApp app;
        app.compress(filePath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
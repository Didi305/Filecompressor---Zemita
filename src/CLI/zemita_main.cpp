#include "zemita/zemita.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: zemita <file_path>\n";
        return 1;
    }

    try {
        ZemitaApp app;
        app.compress(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
#include "fileio.hpp"
#include <fstream>

/* Read a binary, unstructured asset such as a PNG sprite & return as a vector of bytes */
std::vector<uint8_t> readBinAsset(const std::string& filename) {
    std::ifstream file("assets/" + filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    size_t size = file.tellg();
    std::vector<uint8_t> buf(size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buf.data()), size);

    return buf;
}

char* readJsonAsset(const char* filename) {
    std::string fullPath = "assets/" + std::string(filename);
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buf = new char[size + 1];
    file.read(buf, size);
    buf[size] = '\0';

    return buf;
}


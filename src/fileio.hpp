#pragma once

#include <cstdint>
#include <vector>
#include <string>

std::vector<uint8_t> readBinAsset(const std::string& filename);
char* readJsonAsset(const char* filename);


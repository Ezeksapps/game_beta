#pragma once

// #include <string_view> // TODO: string view can optimise this system, but the variadic string can't use this feature
#include <string>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;

void initWordGlobalMappings(const json& saveDataJson);
void supplyExternalWordMap(std::unordered_map<std::string, std::string>& map);

void setSourceString(const std::string& str);
void setSourceStringf(const std::string& str, std::vector<std::string> replacements);

// register a callback that will be executed once OP_END_STRING is processed
void setOnStringEndCallback(const std::function<void()>& callback);

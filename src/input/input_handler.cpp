#include "../fileio.hpp"
#include "input_handler.hpp"

#include <queue>
#include <unordered_map>

#include <json.hpp>

using json = nlohmann::json;

std::queue<GameCmd*> cmdQueue;

std::unordered_map<int, GameCmd> keyMap = {};

/* Read keybind config & map all game commands to the respective enumerated GLFW keys */
void initInputHandler() {
    const char* keybindsJson = readJsonAsset("config/input_config.json"); // TODO: change to a better format (non C-str)

    json root = json::parse(keybindsJson);
    for (json::iterator it = root.begin(); it != root.end(); ++it) {
        keyMap.insert({it.value(), (GameCmd)(std::stoi(it.key()))});
    }
}

void handleInput(const int& keycode) {
    cmdQueue.push(&keyMap[keycode]);
}

void processCmds(void (*callback)(GameCmd* cmd)) {
    if (!cmdQueue.empty()) {
        GameCmd* cmd = cmdQueue.front();
        cmdQueue.pop();
        callback(cmd);
    }
}

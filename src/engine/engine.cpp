#include "engine.hpp"
#include "fileio.hpp"

#include <queue>
#include <unordered_map>

#include <json.hpp>

using json = nlohmann::json;

std::queue<GameCmd*> cmdQueue;
std::unordered_map<int, GameCmd> keyMap = {};

/* --- HELPER FUNCS --- */

/* Read keybind config & map all game commands to the respective enumerated GLFW keys */
void initInputHandler() {
    const char* keybindsJson = readJsonAsset("config/input_config.json"); // TODO: change to a better format (non C-str)

    json root = json::parse(keybindsJson);
    for (json::iterator it = root.begin(); it != root.end(); ++it) {
        keyMap.insert({it.value(), (GameCmd)(std::stoi(it.key()))});
    }
}

/* --- CONSTRUCTOR/DESTRUCTOR --- */

Engine::Engine(const EngineConfig& config) {
    // combine to one func?
    m_pRenderer = std::make_unique<Renderer>(config.viewportWidth, config.viewportHeight);
    m_pRenderer->initRenderer(config.window, config.renderBackend);
    // TODO: renderer needs to re-create swap chain when viewport size changed
    initInputHandler();
}

Engine::~Engine() {}

/* --- SCENES --- */

void Engine::setScene(const std::string& sceneDir) {
    m_pRenderer->setScene(sceneDir);
}

/* --- GETTERS --- */

std::vector<std::shared_ptr<Entity>>& Engine::getEntities() {
    return m_pScene->getEntities();
}

/* --- INPUT HANDLING --- */

void Engine::handleInput(const int& keycode) {
    cmdQueue.push(&keyMap[keycode]);
}

void Engine::processCmds(std::function<void(GameCmd* cmd)> callback) {
    if (!cmdQueue.empty()) {
        GameCmd* cmd = cmdQueue.front();
        cmdQueue.pop();
        callback(cmd);
    }
}

/* --- RENDERING --- */

void Engine::renderFrame() {
    m_pRenderer->renderFrame();
}

#include "engine.hpp"
#include "fileio.hpp"

#include <queue>
#include <unordered_map>
#include <set>

#include <json.hpp>

using json = nlohmann::json;

std::set<int> activeKeys;

// new input system is WIP, now mostly functional, but I sill can't move north-west?

/* Map key(s) to respective game commands */
std::unordered_map<uint32_t, MovementCmd> movementCmds;
std::unordered_map<uint32_t, GameCmd> singleCmds;

std::queue<GameCmd> pendingCmds; // general command queue (excludes continuous actions like movement)
MovementCmd activeMovement = ENTITY_STOP_MOVEMENT;
uint32_t activeMovementKey = 0; // the key(s) which produced the currently active movement

/* --- HELPER FUNCS --- */

/* Generates the single 32-bit ID for a dual-key combo by combining the 16-bit values of both input keycodes */
uint32_t combineKeys(std::vector<uint16_t> keys) {

    // sort keys in ascending order so that the keys' ID does not differ if the keycodes were entered into JSON in a different order
    std::sort(keys.begin(), keys.end());

    uint32_t packed = 0;
    for (int i = 0; i < keys.size(); ++i) {
        packed |= (static_cast<uint32_t>(keys[i]) << (16 * i));
    }
    return packed;
}


/* Read keybind config & map all game commands to their respective enumerated GLFW keys or dual-key combinations */
void initInputHandler() {
    const char* keybindsJson = readJsonAsset("assets/config/input_config.json");

    uint8_t currentCommand; // index/number of current command

    /*
     * keycode can either be a single int or an array of ints, depending on if the command is activated by a single
     * keypress or multiple simultaneous keypresses.
     * The commands and movement objects in the JSON must be in the same order as the MovementCmd and GameCmd enumerators.
     * If they are not, then commands will be assigned to incorrect keybinds.
     */

    json root = json::parse(keybindsJson);

    for (const json& command : root["movement"]) {
        std::vector<uint16_t> keys;
        if (command["keycode"].is_array()) {
            for (const uint16_t& key : command["keycode"]) keys.push_back(key);
        }
        else keys.push_back(command["keycode"]);

        if (keys.size() == 1) { // single-key commands
            movementCmds.emplace(static_cast<uint32_t>(keys[0]), (MovementCmd)currentCommand);
        }
        else { // multi-key commands
            uint32_t packedKey = combineKeys(keys);
            movementCmds.emplace(packedKey, (MovementCmd)currentCommand);
        }
        ++currentCommand;
    }

    currentCommand = 0;

    for (const json& command : root["commands"]) {
        std::vector<uint16_t> keys;
        if (command["keycode"].is_array()) {
            for (const uint16_t& key : command["keycode"]) keys.push_back(key);
        }
        else keys.push_back(command["keycode"]);


        if (keys.size() == 1) { // single-key commands
            singleCmds.emplace(static_cast<uint32_t>(keys[0]), (GameCmd)currentCommand);
        }
        else { // multi-key commands
            uint32_t packedKey = combineKeys(keys);
            singleCmds.emplace(packedKey, (GameCmd)currentCommand);
        }
        ++currentCommand;
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
    return m_pRenderer->getEntities();
}

/* --- INPUT HANDLING --- */

void Engine::signalKeyPress(const int& keycode) {

    // no need to check if this is a duplicate, since activeKeys is a set, it will reject any attempts to insert duplicate elements anyway
    activeKeys.insert(keycode);

    uint8_t numMovementKeys = 0;
    uint16_t firstMovementKey;

    for (const uint16_t& key : activeKeys) {
        if (movementCmds.contains(key)) {
            // ignore interference from any other movement key(s) user may have accidentally pressed
            // the first two movement keys you press are the ones that'll matter
            if (numMovementKeys > 1) continue;
            // first movement key in possible dual-key, don't issue any commands yet
            if (numMovementKeys == 0) {
                firstMovementKey = key;
                ++numMovementKeys;
                continue;
            }
            // second movement key, completing dual-key command
            else if (numMovementKeys == 1) {
                uint32_t packedKey = combineKeys({firstMovementKey, key});
                activeMovement = movementCmds[packedKey];
                activeMovementKey = packedKey;
                ++numMovementKeys;
            }
        } // needs to handle dual-key!
        else pendingCmds.push(singleCmds[key]);
    }
    // if no more than one movement key is currently active then issue the movement command bound to the key
    if (numMovementKeys == 1) {
        activeMovement = movementCmds[firstMovementKey];
        activeMovementKey = firstMovementKey;
    }
}

void Engine::signalKeyRelease(const int& keycode) {

    activeKeys.erase(keycode);

    if (movementCmds.contains(keycode)) {
        switch (activeMovement) {

            // if the active movement wasn't diagonal, then just stop movement
            case ENTITY_MOVE_SOUTH:
            case ENTITY_MOVE_EAST:
            case ENTITY_MOVE_NORTH:
            case ENTITY_MOVE_WEST:
                activeMovement = ENTITY_STOP_MOVEMENT;
                break;

            // don't stop movement immediately. If the active movement is diagonal, then the new movement command
            // needs to be set to whichever one the other active movement key maps to
            case ENTITY_MOVE_SOUTH_EAST:
            case ENTITY_MOVE_NORTH_EAST:
            case ENTITY_MOVE_NORTH_WEST:
            case ENTITY_MOVE_SOUTH_WEST:
                uint16_t key1 = activeMovementKey & 0xFFFF;
                uint16_t key2 = (activeMovementKey >> 16) & 0xFFFF;
                if (keycode == key1) activeMovement = movementCmds[key2];
                else activeMovement = movementCmds[key1];
                break;
        }
    }
}

void Engine::processCmds(std::function<void(GameCmd* cmd)> callback) {
    if (!pendingCmds.empty()) {
        GameCmd* cmd = &pendingCmds.front();
        pendingCmds.pop();
        callback(cmd);
    }
}

void Engine::handleMovement(std::function<void(MovementCmd* cmd)> callback) {
    callback(&activeMovement);
}

/* --- RENDERING --- */

void Engine::renderFrame() {
    m_pRenderer->renderFrame();
}

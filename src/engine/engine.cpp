#include "engine.hpp"
#include "fileio.hpp"

#include <queue>
#include <unordered_map>
#include <set>

#include <json.hpp>

using json = nlohmann::json;

std::set<int> activeKeys;

// new input system is WIP, movement is bugged at the moment, single commands work fine

/* Map key combos to respective game commands */
std::unordered_map<uint32_t, MovementCmd> movementCmds;
std::unordered_map<uint32_t, GameCmd> singleCmds;

/* Map a key to all candidate combos which contain it */
std::unordered_map<uint16_t, std::vector<uint32_t>> movementIndex;
std::unordered_map<uint16_t, std::vector<uint32_t>> singleIndex;

std::queue<GameCmd> pendingCmds; // general command queue (excludes continuous actions like movement)
MovementCmd activeMovement = ENTITY_STOP_MOVEMENT;

/* --- HELPER FUNCS --- */

/* Generates the single 32-bit ID for a dual-key combo by combining the 16-bit values of both input keycodes */
uint32_t combineKeys(std::vector<uint16_t>& keys) {

    // sort keys in ascending order so that the keys' ID does not differ if the keycodes were entered into JSON in a different order
    std::sort(keys.begin(), keys.end());

    uint32_t packed = 0;
    for (int i = 0; i < keys.size(); ++i) {
        packed |= (static_cast<uint32_t>(keys[i]) << (16 * i));
    }
    return packed;
}

/* Checks if a candidate command should be activated, returning true if the keys bound to the command are all pressed */
bool isCmdComplete(uint32_t cmdKey) {
    uint16_t key1 = cmdKey & 0xFFFF;
    uint16_t key2 = (cmdKey >> 16) & 0xFFFF;

    if (key2 == 0) return activeKeys.contains(key1); // if single key command, is the required key pressed?
    return activeKeys.contains(key1) && activeKeys.contains(key2); // if dual-key command, are both required keys pressed?
}

bool isDualKeyCmd(uint32_t cmdKey) {
    return ((cmdKey >> 16) & 0xFFFF) != 0;
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
     *
     * Also, since the key map currently identifies key combos with a uint32_t, only up to 2 keycodes may be assigned
     * to a command. If a need is later found for more, this will be expanded to uint64_t to allow up to 4 keycodes
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
            movementIndex[keys[0]].push_back(static_cast<uint32_t>(keys[0]));
        }
        else { // multi-key commands
            uint32_t packedKey = combineKeys(keys);
            movementCmds.emplace(packedKey, (MovementCmd)currentCommand);
            // for maps, operator [] will create the specified entry if it doesnt already exist
            for (const uint16_t& key : keys) movementIndex[key].push_back(packedKey);
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
            singleIndex[keys[0]].push_back(static_cast<uint32_t>(keys[0]));
        }
        else { // multi-key commands
            uint32_t packedKey = combineKeys(keys);
            singleCmds.emplace(packedKey, (GameCmd)currentCommand);
            // for maps, operator [] will create the specified entry if it doesnt already exist
            for (const uint16_t& key : keys) singleIndex[key].push_back(packedKey);
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

bool isMovementKey(uint32_t cmdKey) {
    return movementCmds.contains(cmdKey);
}

void Engine::signalKeyPress(const int& keycode) { // there is a noticeable lag before movement begins due to an unhandled case

    if (!activeKeys.contains(keycode)) {
        activeKeys.insert(keycode);

        /* movement keys are restricted to four. The diagonal movement commands must be bound to dual-key combos of the movement keys
         * The four keys cannot be used for any non-movement commands
         */

        // if the recieved key is not bound to any movement command, then handle this key as a GameCmd
        if (!movementCmds.contains(keycode)) {

            const std::vector<uint32_t>& candidates = singleIndex[keycode];
            // Check if this just-recieved keycode completes any candidate dual-key command
            bool resolved = false;
            for (uint32_t cmdKey : candidates) {
                if (isCmdComplete(cmdKey) && isDualKeyCmd(cmdKey)) {
                    pendingCmds.push(singleCmds[keycode]);
                    resolved = true;
                    break;
                }
            }
            // If no dual-key command was completed, enqueue whatever command this key only maps to
            if (!resolved) pendingCmds.push(singleCmds[keycode]);
        }
        // this is a movement key
        else {
            // check all candidate movement commands containing this keycode
            const std::vector<uint32_t>& candidates = movementIndex[keycode];
            // Check if this just-recieved keycode completes any candidate dual-key command
            bool resolved = false;
            for (uint32_t cmdKey : candidates) {
                if (isCmdComplete(cmdKey) && isDualKeyCmd(cmdKey)) {
                    activeMovement = movementCmds[keycode];
                    resolved = true;
                    break;
                }
            }
            // If no dual-key command was completed, enqueue whatever command this key only maps to
            if (!resolved) activeMovement = movementCmds[keycode];
        }
    }
}

void Engine::signalKeyRelease(const int& keycode) {

    if (movementCmds.contains(keycode)) {
        // Find all continuous commands that use this key
        const std::vector<uint32_t>& candidates = movementIndex[keycode];
        if (!candidates.empty()) {
            for (uint32_t cmdKey : candidates) {
                MovementCmd cmd = movementCmds[cmdKey];

                // if command no longer has its bound keys pressed, remove it
                if (!isCmdComplete(cmdKey)) {
                    activeMovement = ENTITY_STOP_MOVEMENT;
                }
            }
        }
    }
    else activeKeys.erase(keycode);

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

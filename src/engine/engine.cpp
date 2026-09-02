#include "engine.hpp"
#include "fileio.hpp"

#include <queue>
#include <unordered_map>
#include <set>

#include <json.hpp>

using json = nlohmann::json;

std::set<int> activeKeys;

/* Map key combos to respective game commands */
std::unordered_map<uint32_t, GameCmd> continuousCmds;
std::unordered_map<uint32_t, GameCmd> singleCmds;

/* Map a key to all candidate combos which contain it */
std::unordered_map<uint16_t, std::vector<uint32_t>> continuousIndex;
std::unordered_map<uint16_t, std::vector<uint32_t>> singleIndex;

uint16_t prevKey = 0; // previously-pressed key

std::queue<GameCmd> pendingCmds; // command queue
std::set<GameCmd> activeContinuousCmds; // active continuous actions

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

/* Read keybind config & map all game commands to their respective enumerated GLFW keys or dual-key combinations */
void initInputHandler() {
    const char* keybindsJson = readJsonAsset("assets/config/input_config.json");

    uint8_t currentCommand; // index/number of current command

    json commands = json::parse(keybindsJson)["commands"];
    for (const json& command : commands) {
        /* Each command has two fields: keycode and keypressType
         * keycode can either be a single int or an array of ints, depending on if the command is activated by a single
         * keypress or multiple simultaneous keypresses. keypressType defines if this command is activated
         * through a single keypress (0), or a continuous keypress (1)
         * The command object in the JSON must be in the same order as the GameCmds struct. If they are not, then
         * commands will be assigned to incorrect keybinds.
         *
         * Also, since the key map currently identifies key combos with a uint32_t, only up to 2 keycodes may be assigned
         * to a command. If a need is later found for more, this will be expanded to uint64_t to allow up to 4 keycodes
         */
        std::vector<uint16_t> keys;
        if (command["keycode"].is_array()) {
            for (const uint16_t& key : command["keycode"]) keys.push_back(key);
        }
        else keys.push_back(command["keycode"]);

        bool continuous = command["keypressType"] == 1;

        if (continuous) {
            if (keys.size() == 1) { // single-key commands
                continuousCmds.emplace(static_cast<uint32_t>(keys[0]), (GameCmd)currentCommand);
                continuousIndex[keys[0]].push_back(static_cast<uint32_t>(keys[0]));
            }
            else { // multi-key commands
                uint32_t packedKey = combineKeys(keys);
                continuousCmds.emplace(packedKey, (GameCmd)currentCommand);
                // for maps, operator [] will create the specified entry if it doesnt already exist
                for (const uint16_t& key : keys) continuousIndex[key].push_back(packedKey);
            }
        }
        else {
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

void Engine::signalKeyPress(const int& keycode) { // FIX: dual-key combos still don't work properly

    if (!activeKeys.contains(keycode)) {
        activeKeys.insert(keycode);

        if (prevKey != 0) {
            // combine the keycodes of this key and the previous
            std::vector<uint16_t> keys = {prevKey, static_cast<uint16_t>(keycode)};
            uint32_t combinedKeys = combineKeys(keys);
            // if the combination of this key and the previous key are a registered dual-key combo, push
            // the command mapped to it to the command queue
            if (singleCmds.contains(combinedKeys)) pendingCmds.push(singleCmds[combinedKeys]);
            // otherwise push the command which maps to the previous key to the command queue and reset prevKey
            else {
                pendingCmds.push(continuousCmds[prevKey]);
                prevKey = 0;
            }
        }

        // initially push whatever command just this key maps to
        // (This is so that single key press commands which are not dual-key combos will show an instant response)
        pendingCmds.push(singleCmds[keycode]);

        // check all candidate single game commands containing this keycode
        const std::vector<uint32_t>& candidates = singleIndex[keycode];
        // if more than one candidate, store this keycode so the next call of signalKeyPress() can
        // check if the next keypress completes a dual-key command or if the single-key command
        // should be dispatched
        if (candidates.size() > 1) prevKey = keycode;
    }

    // else the GameCmd associated with this key is already registered
    // since this is a repeat press, assume the user wishes to perform a continuous command
    else {
        if (prevKey != 0) {
            // combine the keycodes of this key and the previous
            std::vector<uint16_t> keys = {prevKey, static_cast<uint16_t>(keycode)};
            uint32_t combinedKeys = combineKeys(keys);
            // if the combination of this key and the previous key are a registered dual-key combo, push
            // the command mapped to it to the command queue
            if (continuousCmds.contains(combinedKeys)) {
                GameCmd pendingCmd = continuousCmds[combinedKeys];
                if (!activeContinuousCmds.contains(pendingCmd)) {
                    pendingCmds.push(continuousCmds[combinedKeys]);
                    activeContinuousCmds.insert(continuousCmds[combinedKeys]);
                }
            }
            // otherwise push the command which maps to the previous key to the command queue and reset prevKey
            else {
                GameCmd pendingCmd = continuousCmds[prevKey];
                if (!activeContinuousCmds.contains(pendingCmd)) {
                    pendingCmds.push(continuousCmds[prevKey]);
                    activeContinuousCmds.insert(continuousCmds[combinedKeys]);
                    prevKey = 0;
                }
            }
        }

        // check all candidate continuous game commands containing this keycode
        const std::vector<uint32_t>& candidates = continuousIndex[keycode];
        // if only one candidate exists, push that GameCmd to the queue
        if (candidates.size() == 1) pendingCmds.push(continuousCmds[candidates[0]]);
        // if more than one candidate, store this keycode so the next call of signalKeyPress() can
        // check if the next keypress completes a dual-key command or if the single-key command
        // should be dispatched
        prevKey = keycode;
    }
}

void Engine::signalKeyRelease(const int& keycode) {
    activeKeys.erase(keycode);
    // if the key being released cancels a continuous action, first determine
    // if another continuous action should be activated in its place (for example, if ENTITY_MOVE_NORTH_WEST was
    // previously active, and A was released, then check if the other key (W) in that combo is still active,
    // if it is, then switch the continuous movement to ENTITY_MOVE_NORTH)
    // Skip this check if the current continuous action is not mapped to more than a singular key
    //if (activeContinuousCmds.empty()) return;
    pendingCmds.push(ENTITY_STOP_MOVEMENT); // this works, implement the full func later
}

void Engine::processCmds(std::function<void(GameCmd* cmd)> callback) {
    if (!pendingCmds.empty()) {
        GameCmd* cmd = &pendingCmds.front();
        pendingCmds.pop();
        callback(cmd);
    }
}

/* --- RENDERING --- */

void Engine::renderFrame() {
    m_pRenderer->renderFrame();
}

#include "entity.hpp"
#include "../fileio.hpp"
#include "../graphics/sprite.hpp"
#include <json.hpp>

using json = nlohmann::json;

Entity::Entity(const std::string& animJsonFilepath, const int& index) {

    m_index = index;
    m_animJsonFilepath = animJsonFilepath;

    // TODO: use JSON from first param to load overworld events/interactions and behaviour

    const char* spriteJson = readJsonAsset(animJsonFilepath.c_str());

    json anims = json::parse(spriteJson)["anims"];

    for (const json& anim : anims) {

        int typeValue = anim["type"];

        Sprite sprite {
            .filepath       = anim["filepath"],
            .frameDurations = anim["durations"],
            .frameWidth     = anim["frameWidth"],
            .frameHeight    = anim["frameHeight"]
        };

        m_spriteMap.insert({(AnimEvent)anim["type"], std::make_shared<Sprite>(sprite)});
    }

    // bboxes will eventually move to separate JSON
    json bbox = json::parse(spriteJson)["bbox"];
    m_dxHalf = (bbox[0].get<int>() / 2) * 0.04;
    m_dyHalf = (bbox[1].get<int>() / 2) * 0.04;

    doAnimEvent(ANIM_EVENT_IDLE); // default idle anim
    m_direction = DIRECTION_WEST; // default direction
}

void Entity::loadEntityJSON(const std::string& filepath) {
    if (m_index > 0) { // the player has no dialogue or pre-defined behaviour
        const char* entityJson = readJsonAsset(filepath.c_str());

        json dialogStrings = json::parse(entityJson)["dialog"];

        for (json::iterator it = dialogStrings.begin(); it != dialogStrings.end(); ++it) {
            m_dialogStrings.emplace(it.key(), it.value());
        }
    }
}

void Entity::setOverlay(const std::shared_ptr<Overlay> overlay, const uint16_t& numCycles, const uint16_t& numFlashes) {
    m_overlay = overlay;

    // Note: must be cast to enum type explicitly to convert from int
    m_overlayType = static_cast<OverlayType>((numFlashes != 0 ? OVERLAY_TYPE_FLASHING : 0) | (numCycles != 0 ? OVERLAY_TYPE_ANIMATED : 0));
}

std::shared_ptr<Overlay> Entity::getOverlay() { return m_overlay; }

bool Entity::hasOverlay() { return m_overlayType != OVERLAY_TYPE_NONE; }

// remove the current overlay if one is set
// the overlay doesn't actually need to be removed, resetting the enum value is enough for the system to ignore its existence
void Entity::removeOverlay() { m_overlayType = OVERLAY_TYPE_NONE; }

bool Entity::hasDialogString(const std::string& key) { return m_dialogStrings.contains(key); }

std::string Entity::getDialogString(const std::string& key) { return m_dialogStrings[key]; }

void updateAnim(std::shared_ptr<Sprite>& sprite, float& timer, const float& deltaTime) {
    timer += deltaTime;

    // intended duration of current frame of sprite sheet in frames
    float duration = sprite->frameDurations[sprite->frame];

    // if the timer shows that the currently rendered frame has been rendered for a time exceeding the intended duration,
    // reset the timer and advance to next frame
    if (timer >= duration) {
        timer         = 0.0f;
        sprite->frame = (sprite->frame + 1) % sprite->framesPerRow;
    }
}

void updateFlash(Flash& flash, const float& deltaTime) {
    flash.x += flash.perFrameDx;
    flash.alpha = (flash.startAlpha - flash.amplitude) + flash.amplitude * cos(flash.x);
}

void Entity::update(const float& deltaTime) { // called every time a new frame is drawn

    updateAnim(m_pActiveSprite, m_frameTimer, deltaTime); // update main sprite animation

    switch (m_overlayType) {
        case OVERLAY_TYPE_NONE: // there is no overlay to update
            return;
        case OVERLAY_TYPE_FLASHING:
            updateFlash(m_overlay->flash, deltaTime);
            break;
        case OVERLAY_TYPE_ANIMATED:
            updateAnim(m_overlay->sprite, m_overlay->frameTimer, deltaTime);
            break;
        case OVERLAY_TYPE_FLASH_AND_ANIM:
            updateFlash(m_overlay->flash, deltaTime);
            updateAnim(m_overlay->sprite, m_overlay->frameTimer, deltaTime);
            break;
    }
}

void Entity::doAnimEvent(const AnimEvent& event) {
    m_event = event;
    int oldSpriteIndex = 0;
    if (m_pActiveSprite) oldSpriteIndex = m_pActiveSprite->index;
    m_pActiveSprite = m_spriteMap[event];
    // also called to set default anim before setting callback
    if (m_spriteChangeCallback) m_spriteChangeCallback(oldSpriteIndex, m_pActiveSprite);
}

const std::shared_ptr<Sprite>& Entity::getActiveSprite() {
    return m_pActiveSprite;
}

void Entity::setSpriteChangeCallback(std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> callback) {
    m_spriteChangeCallback = callback;
}

void Entity::setOverlayCallback(std::function<void(const Overlay& overlay)> callback) {
    m_setOverlayCallback = callback;
}

void Entity::setStartMovementCallback(std::function<void(const int& index, vec3 translVec)> callback) {
    m_startMovementCallback = callback;
}

void Entity::setChangeMovementDirectionCallback(std::function<void(const int& index, vec3 translVec)> callback) {
    m_changeMovementDirectionCallback = callback;
}

void Entity::setEndMovementCallback(std::function<void(const int& index)> callback) {
    m_endMovementCallback = callback;
}

std::string Entity::getCacheKey() {
    return m_animJsonFilepath;
}

const std::unordered_map<AnimEvent, std::shared_ptr<Sprite>>& Entity::getSpriteMap() {
    return m_spriteMap;
}

AABB Entity::getAABB() {
    return AABB {
        .min = vec3(m_pos.x - m_dxHalf, m_pos.y - m_dyHalf, 0.0f),
        .max = vec3(m_pos.x + m_dxHalf, m_pos.y + m_dyHalf, 0.0f)
    };
}

// helper function, calculates per-frame translation vector for a movement in a particular direction
vec3 getTranslVec(const Direction& direction, const int& animFrames) { // TODO: Should eventually consider mode, so that running increases the translVec
    vec3 translVec;

    switch (direction) {
        case DIRECTION_SOUTH:
            //m_pos += vec3(0.0f, -1.0f, 0.0f);
            translVec = vec3(0.0f, -1.0 / animFrames, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_SOUTH_EAST:
            //m_pos += vec3(1.0f, -1.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, -1.0f / animFrames, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_EAST:
            //m_pos += vec3(1.0f, 0.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, 0.0f, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_NORTH_EAST:
            //m_pos += vec3(1.0f, 1.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_NORTH:
            //m_pos += vec3(0.0f, 1.0f, 0.0f);
            translVec = vec3(0.0f, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_NORTH_WEST:
            //m_pos += vec3(-1.0f, 1.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_WEST:
            //m_pos += vec3(-1.0f, 0.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, 0.0f, 0.0f /* Z-axis ignored */);
            break;
        case DIRECTION_SOUTH_WEST:
            //m_pos += vec3(-1.0f, -1.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, -1.0f / animFrames, 0.0f /* Z-axis ignored */);
            break;
    }
    return translVec;
}

void Entity::move(const Direction& direction, const AnimEvent& mode) {

    m_direction = direction;

    doAnimEvent(mode);

    // find total number of frames accompanying movement animation runs for
    m_animFrames = 0;
    for (const int& i : m_pActiveSprite->frameDurations) m_animFrames += i;

    m_startMovementCallback(m_index, getTranslVec(direction, m_animFrames));
}

// NOTE: Running will eventually add further complication, as Renderer also needs to be signalled of a larger translVec
// in that situation, possibly requiring another similar callback
void Entity::changeMovementDirection(const Direction& direction) {
    m_direction = direction;
    m_changeMovementDirectionCallback(m_index, getTranslVec(direction, m_animFrames));
}

void Entity::endMovement() {
    m_endMovementCallback(m_index);
    doAnimEvent(ANIM_EVENT_IDLE);
}

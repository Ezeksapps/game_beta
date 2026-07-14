#include "entity.hpp"

// DEFINITE TODO: Figure out the game state system

Entity::Entity(const SpriteSheetInfo& spriteSheetInfo) {
    for (int i = 0; i < spriteSheetInfo.entityEvents.size(); ++i) {
        Sprite sprite {
            .filepath = spriteSheetInfo.spriteSheets[i],
            .pagesPerAnim = spriteSheetInfo.eventPages[i]
        };
        m_spriteMap.insert({spriteSheetInfo.entityEvents[i], std::make_shared<Sprite>(sprite)});
    }
}
Entity::~Entity() {}

void Entity::doAnimEvent(const AnimEvent& event) {
    m_pActiveSprite = m_spriteMap[event];

   // g_state->g_renderer->swapSprite(, )

    /* TEMP PSEUDOCODE IMPL
     * --------------------
     * renderer->swapSprite(m_pActiveSprite.)

    // <Sprite>, <pageIdx> (within sprite),
}

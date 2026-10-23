#include "game_event.hpp"
#include "../game.hpp"

void triggerEvent(const GameEvent& event) {
    switch (event) {
        case GAME_EVENT_BATTLE_START:
            // TODO: Need proper way to get opponent entity for constructor
            //std::unique_ptr<Scene> battleScene = std::make_unique<BattleScene>("assets/scenes/battleScene", g_pPlayer, );
            //g_pRenderer->setScene(battleScene, texMapping);
            break;
        default:
            break;
    }
}

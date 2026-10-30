#include "game_event.hpp"
#include "../ui/dialog.hpp"
#include "../game.hpp"

void triggerBattleStart() {
    std::unique_ptr<BattleScene> battleScene = std::make_unique<BattleScene>("assets/scenes/battleScene", g_pPlayer, getInteractingEntity(), "plains");

    auto texMapping = battleScene->getTexMapping();
    std::unique_ptr<Scene> scene = std::move(battleScene);
    g_pRenderer->setScene(scene, texMapping);
}

void triggerEvent(const GameEvent& event) {
    switch (event) {
        case GAME_EVENT_BATTLE_START:
            triggerBattleStart();
            break;
        default:
            break;
    }
}

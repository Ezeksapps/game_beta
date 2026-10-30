#include "game_event.hpp"
#include "../ui/dialog.hpp"
#include "../ui/ui.h" // TEMP
#include "../game.hpp"

// should become part of battle manager init
void triggerBattleStart() {
    std::shared_ptr<Entity> opponentEntity = getInteractingEntity();
    std::unique_ptr<BattleScene> battleScene = std::make_unique<BattleScene>("assets/scenes/battleScene", g_pPlayer, opponentEntity, "plains");

    std::unordered_map<std::string, std::string> texMapping = battleScene->getTexMapping();
    std::unique_ptr<Scene> scene = std::move(battleScene);
    g_pRenderer->setScene(scene, texMapping);

    g_pPlayer->m_pos = getNodePosition("player_stage");
    opponentEntity->m_pos = getNodePosition("opponent_stage");

    popFromStack();// TEMP
    battleDashboard();
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

#include "battle_manager.hpp"

#include "../scene/battle_scene.hpp"
#include "../entity/entity.hpp"
#include "../ui/dialog.hpp"
#include "../ui/ui.h"
#include "../game.hpp"

BattleManager::BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent) {
    m_player = player;
    m_opponent = opponent;
}

BattleManager::~BattleManager() {}

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

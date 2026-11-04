#include "battle_manager.hpp"

#include "../scene/battle_scene.hpp"
#include "../pokemon/pokemon.hpp"
#include "../entity/entity.hpp"
#include "../ui/dialog.hpp"
#include "../ui/ui.h"
#include "../game.hpp"

BattleManager::BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent, BattleScene* scene) {
    m_player = player;
    m_opponent = opponent;
    m_scene = scene;
}

void BattleManager::prepareStage(std::shared_ptr<Pokemon> playerPkmn, std::shared_ptr<Pokemon> opponentPkmn) {
    playerPkmn->m_pos = getNodePosition("player_stage");
    opponentPkmn->m_pos = getNodePosition("opponent_stage");

    m_scene->pushPokemonToScene(playerPkmn);
    m_scene->pushPokemonToScene(opponentPkmn);
}

BattleManager::~BattleManager() {}

void triggerBattleStart() { // creates the battle scene, which will then recieve its own instance battle manager to take over processing
    std::shared_ptr<Entity> opponentEntity = getInteractingEntity();
    std::unique_ptr<BattleScene> battleScene = std::make_unique<BattleScene>("assets/scenes/battleScene", g_pPlayer, opponentEntity, "plains");

    BattleScene* battleSceneTemp = battleScene.get(); // needed for initial stage prep only

    std::unordered_map<std::string, std::string> texMapping = battleScene->getTexMapping();
    std::unique_ptr<Scene> scene = std::move(battleScene);

    g_pRenderer->setScene(scene, texMapping);

    // NOTE: This can only be called after setScene, as prepareStage depends on the glTF asset being loaded for its getNodePosition() calls
    battleSceneTemp->m_battleManager->prepareStage(createPokemonFromDesc(dynamic_pointer_cast<Trainer>(g_pPlayer)->getTeam()[0], 0),
                                                   createPokemonFromDesc(dynamic_pointer_cast<Trainer>(opponentEntity)->getTeam()[0], 1));
}



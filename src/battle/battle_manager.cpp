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

    std::cout << "Basic vars initialised\n";

    // the first operation should be to bring out the first pokemon in both the player's and the opponent's teams
    std::shared_ptr<Pokemon> playerPkmn = createPokemonFromDesc(m_player->getTeam()[0], 0);
    std::shared_ptr<Pokemon> opponentPkmn = createPokemonFromDesc(m_opponent->getTeam()[0], 1);

    std::cout << "Created both Pokemon objs\n";
}

void BattleManager::prepareStage(std::shared_ptr<Pokemon> playerPkmn, std::shared_ptr<Pokemon> opponentPkmn) {
    playerPkmn->m_pos = getNodePosition("player_stage");
    opponentPkmn->m_pos = getNodePosition("opponent_stage");

    m_scene->pushPokemonToScene(playerPkmn);
    std::cout << "push op1\n";
    m_scene->pushPokemonToScene(opponentPkmn);
    std::cout << "push op2\n";
}

BattleManager::~BattleManager() {}

void triggerBattleStart() { // creates the battle scene, which will then recieve its own instance battle manager to take over processing
    std::shared_ptr<Entity> opponentEntity = getInteractingEntity();
    std::unique_ptr<BattleScene> battleScene = std::make_unique<BattleScene>("assets/scenes/battleScene", g_pPlayer, opponentEntity, "plains");

    BattleScene* battleSceneTemp = battleScene.get(); // needed for initial stage prep only

    std::unordered_map<std::string, std::string> texMapping = battleScene->getTexMapping();
    std::unique_ptr<Scene> scene = std::move(battleScene);

    battleSceneTemp->m_battleManager->prepareStage(createPokemonFromDesc(dynamic_pointer_cast<Trainer>(g_pPlayer)->getTeam()[0], 0),
                                                   createPokemonFromDesc(dynamic_pointer_cast<Trainer>(opponentEntity)->getTeam()[0], 0));


    g_pRenderer->setScene(scene, texMapping);
}



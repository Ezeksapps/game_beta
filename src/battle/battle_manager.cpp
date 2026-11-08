#include "battle_manager.hpp"

#include "move_interface.h"

void useMove(void* _this, int index) {
    static_cast<BattleManager*>(_this)->playerUseMove(index);
}

void prepareMoveSelector(void* _this, int index) {
    static_cast<BattleManager*>(_this)->setupMoves();
}

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
    // player has first turn
    m_isPlayerTurn = true;

    popFromStack();
    pushToStackWithData(battleDashboard, this);
}

void BattleManager::prepareStage(std::shared_ptr<Pokemon> playerPkmn, std::shared_ptr<Pokemon> opponentPkmn) {
    playerPkmn->m_pos = getNodePosition("player_stage");
    opponentPkmn->m_pos = getNodePosition("opponent_stage");

    playerPkmn->m_direction = DIRECTION_EAST;
    opponentPkmn->m_direction = DIRECTION_WEST;

    m_scene->pushPokemonToScene(playerPkmn);
    m_scene->pushPokemonToScene(opponentPkmn);

    m_playerPkmn = playerPkmn, m_opponentPkmn = opponentPkmn;
}

void BattleManager::setupMoves() {

    const std::array<Move, 4>& moveset = m_playerPkmn->getMoveset();

    for (int i = 0; i < moveset.size(); ++i) {
        m_movesetUI[i] = (MoveC) {
            .name     = moveset[i].name.c_str(),
            .type     = moveset[i].type,
            .category = moveset[i].category,
            .pp       = moveset[i].pp,
            .power    = moveset[i].power,
            .accuracy = moveset[i].accuracy,
            .priority = moveset[i].priority
        };
    }

    m_playerMovesetDataUI = (MovesetDataC) { .moveset = &m_movesetUI[0], .battleManager = this };
    // setup the Fight UI menu with the player's moveset, this way, the player can make their turn
    pushToStackWithData(moveSelector, &m_playerMovesetDataUI);
}

void BattleManager::playerUseMove(const int& index) {
    m_playerAction = UseMove {index};
    m_hasPlayerSetAction = true;
}

void BattleManager::opponentUseMove(const int& index) {
    m_opponentAction = UseMove {index};
    m_hasOpponentSetAction = true;
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

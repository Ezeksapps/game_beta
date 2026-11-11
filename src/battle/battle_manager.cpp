#include "battle_manager.hpp"

#include "move_interface.h"
#include <cstdio>

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

BattleManager::BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent, BattleScene* scene) : m_rng(std::time(0)) {
    m_player = player;
    m_opponent = opponent;
    m_scene = scene;

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
    m_turn.playerAction = Action { ACTION_TYPE_USE_MOVE, index };
    m_state = STATE_AWAITING_OPPONENT_ACTION;
}

void BattleManager::opponentUseMove(const int& index) {
    m_turn.opponentAction = Action { ACTION_TYPE_USE_MOVE, index };
    m_state = STATE_ACTIONS_SET;
}

void BattleManager::update() {
    switch (m_state) {
        case STATE_AWAITING_PLAYER_ACTION:
            break;
        case STATE_AWAITING_OPPONENT_ACTION:
            // TEMP -------
            m_turn.opponentAction = Action { ACTION_TYPE_USE_MOVE, 0 };
            m_state = STATE_ACTIONS_SET;
            // END TEMP ---
            break;
        case STATE_ACTIONS_SET:
            processTurn();
            m_state = STATE_PROCESSING_TURN;
            break;
        case STATE_PROCESSING_TURN:
            // nothing to update
            return;
    }
}

/* -- HELPERS (to be moved into separate file(s) later) --- */


// Source: https://bulbapedia.bulbagarden.net/wiki/Damage, see subheading "Generation V onward"
int BattleManager::calculateMoveDamage(const Move& move, std::shared_ptr<Pokemon>& user, std::shared_ptr<Pokemon>& target) {

    SpcData userSpcData = user->getSpcData();
    SpcData targetSpcData = target->getSpcData();

    float typeEffect = getMoveEffectivenessMultiplier(move.type, targetSpcData.types);
    if (typeEffect == 0.0f) return typeEffect; // because performing the rest of the calculation is pointless if this is zero

    int a, d;

    if (move.category == MOVE_TYPE_PHYSICAL) a = user->m_stats.attack, d = target->m_stats.defense;
    // MOVE_TYPE_STATUS not handled, because this func shouldn't be called for status moves (since they don't inflict damage)
    else a = user->m_stats.spAttack, d = target->m_stats.spDefense;

    float num = (((2.0f * user->m_level) / 5.0f) + 2) * move.power * ( static_cast<float>(a) / static_cast<float>(d) );
    float frac = (num / 50.0f) + 2;

    float crit, rand, STAB, burn;

    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit, 'stages' not implemented
    std::uniform_int_distribution<int> critDist(1, 24);
    bool moveIsCritical = critDist(m_rng) == 1;
    if (moveIsCritical) crit = 1.5f;
    else crit = 1.0f;

    // STAB is same-type attack bonus
    bool hasSTAB = (move.type == userSpcData.types[0] || move.type == userSpcData.types[1]);
    if (hasSTAB) STAB = (userSpcData.abilities[0] == ABILITY_ADAPTABILITY || userSpcData.abilities[1] == ABILITY_ADAPTABILITY) ? 2.0f : 1.5f;
    else STAB = 1.0f;

    // if the user is burned and is using a physical move (and its ability is not guts, which raises attack by 50% when afflicted by a status condition)
    // then damp damage by a factor of 0.5
    if (user->m_statusCondition == STATUS_CONDITION_BURNED  &&
        move.category           == MOVE_TYPE_PHYSICAL       &&
        std::find(userSpcData.abilities.begin(), userSpcData.abilities.end(), ABILITY_GUTS) == userSpcData.abilities.end()) burn = 0.5f;

    else burn = 1.0f;


    static constexpr int RAND_BOUND_LOWER = 85;
    static constexpr int RAND_BOUND_UPPER = 100;

    std::uniform_int_distribution<int> distr(RAND_BOUND_LOWER, RAND_BOUND_UPPER);
    rand = distr(m_rng) / 100.0f;

    // TODO: an 'other' float val is also needed to process the effects of some abilities

    float result = frac * crit * rand * burn * STAB * typeEffect;

    return std::floor(result);
}

void BattleManager::sortActionsByPriority() {

    // NOTE: The large majority of moves have a priority of 0 and thus who goes first will be decided by the speed stat
    // an example of a move with a non-zero priority is quick attack

    const int& priorityPlayer = m_playerPkmn->getMoveset()[m_turn.playerAction.index].priority;
    const int& priorityOpponent = m_opponentPkmn->getMoveset()[m_turn.opponentAction.index].priority;

    if (priorityPlayer != priorityOpponent) { // if priorities differ
        if (priorityPlayer > priorityOpponent) m_turn.priority = PRIORITISE_PLAYER; // if player's priority is greater
        else m_turn.priority = PRIORITISE_OPPONENT; // else opponent's must be greater
    }
    else {
        // if priorities are equal, whoever has the highest speed stat goes first
        if (m_playerPkmn->m_stats.speed > m_opponentPkmn->m_stats.speed) m_turn.priority = PRIORITISE_PLAYER;
        else if (m_playerPkmn->m_stats.speed < m_opponentPkmn->m_stats.speed) m_turn.priority = PRIORITISE_OPPONENT;
        // if speed stat is equal, then who goes first is up to RNG
        else {
            std::uniform_int_distribution<int> dist(0, 1);
            switch (dist(m_rng)) {
                case 0: m_turn.priority = PRIORITISE_PLAYER; break;
                case 1: m_turn.priority = PRIORITISE_OPPONENT; break;
            }
        }
    }
}

void BattleManager::sortActions(const bool playerUsesMove, const bool& opponentUsesMove) {
    /* This is called if both parties are not simultaneously using a move
     * Using an item is always given priority over moves
     * Switching out pokemon is also always given priority over a move UNLESS that move is pursuit
     *
     * NOTE: Additional checks on the validity of actions like for example blocking item usage when
     * a Pokemon is subjected to embargo are performed after sorting actions
     */

    if (playerUsesMove) {
        // NOTE: should check internal move ID, NOT display name
        if (m_playerPkmn->getMoveset()[m_turn.playerAction.index].name == "Pursuit") m_turn.priority = PRIORITISE_PLAYER;
        else m_turn.priority = PRIORITISE_OPPONENT;
    }
    else if (opponentUsesMove) {
        if (m_playerPkmn->getMoveset()[m_turn.opponentAction.index].name == "Pursuit") m_turn.priority = PRIORITISE_OPPONENT;
        else m_turn.priority = PRIORITISE_OPPONENT;
    }
    else { // nobody is using a move, which means the order is technically irrelevant, so just return the same order every time
        m_turn.priority = PRIORITISE_PLAYER;
    }
}

// NEXT TODO: Make UI components and apply damage visually

void BattleManager::execPlayerAction() {
    switch (m_turn.playerAction.type) {
        case ACTION_TYPE_USE_MOVE:
            std::cout << "Executing player action, which is using a move that deals "
            << calculateMoveDamage(m_playerPkmn->getMoveset()[m_turn.playerAction.index], m_playerPkmn, m_opponentPkmn)
            << " HP of damage\n";
            // apply damage, also when HP display is created, inform UI
            break;
            // TODO: Make exhaustive
    }

}

void BattleManager::execOpponentAction() {
    switch (m_turn.playerAction.type) {
        case ACTION_TYPE_USE_MOVE:
            std::cout << "Executing opponent action, which is using a move that deals "
            << calculateMoveDamage(m_opponentPkmn->getMoveset()[m_turn.opponentAction.index], m_opponentPkmn, m_playerPkmn)
            << " HP of damage\n";
            break;
            // TODO: Make exhaustive
    }
}

void BattleManager::processTurn() {

    const bool playerUsesMove = m_turn.playerAction.type == ACTION_TYPE_USE_MOVE;
    const bool opponentUsesMove = m_turn.opponentAction.type == ACTION_TYPE_USE_MOVE;

    // Both parties use a move: priority determines execution order.
    if (playerUsesMove && opponentUsesMove) sortActionsByPriority();
    // Only one party uses a move, no need to consider move priority/speed
    else sortActions(playerUsesMove, opponentUsesMove);

    switch (m_turn.priority) {
        case PRIORITISE_PLAYER:
            execPlayerAction();
            execOpponentAction();
            // reset state
            break;
        case PRIORITISE_OPPONENT:
            execOpponentAction();
            execPlayerAction();
            // reset state
            break;
        case PRIORITISE_NONE:
            throw std::runtime_error("Action sorting has failed and turn processing cannot continue");
            break;
    }
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

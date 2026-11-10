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
    m_playerAction = Action { ACTION_TYPE_USE_MOVE, index };
    m_state = STATE_AWAITING_OPPONENT_ACTION;
}

void BattleManager::opponentUseMove(const int& index) {
    m_opponentAction = Action { ACTION_TYPE_USE_MOVE, index };
    m_state = STATE_ACTIONS_SET;
}

void BattleManager::update() {
    switch (m_state) {
        case STATE_AWAITING_PLAYER_ACTION:
            break;
        case STATE_AWAITING_OPPONENT_ACTION:
            // TEMP -------
            m_opponentAction = Action { ACTION_TYPE_USE_MOVE, 0 };
            m_state = STATE_ACTIONS_SET;
            // END TEMP ---
            break;
        case STATE_ACTIONS_SET:
            processTurn();
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

    float typeEffect;
    // typeEffect is type effectiveness of the move, check this before anything else,
    // because if effectiveness is immune, then func should immediately return 0
    Effectiveness effectiveness = getMoveEffectiveness(move.type, targetSpcData.types);
    switch (effectiveness) {
        case EFFECTIVENESS_IMMUNE:
            return 0;
        case EFFECTIVENESS_MILDLY_EFFECTIVE:
            typeEffect = 0.5f;
            break;
        case EFFECTIVENESS_NEUTRAL:
            typeEffect = 1.0f;
            break;
        case EFFECTIVENESS_SUPER_EFFECTIVE:
            typeEffect = 2.0f;
            break;
    }
    // NOTE: There is an additional case where a mildly effective move could be 0.125, 0.25 or 0.5
    // and a super effective move 2.0, 4.0 or 8.0 depending on other factors. TODO: Handle this

    int a, d;

    if (move.category == MOVE_TYPE_PHYSICAL) a = user->m_stats.attack, d = user->m_stats.defense;
    // MOVE_TYPE_STATUS not handled, because this func shouldn't be called for status moves (since they don't inflict damage)
    else a = user->m_stats.spAttack, d = user->m_stats.spDefense;

    float num = (((2.0f * user->m_level) / 5.0f) + 2) * move.power * ( static_cast<float>(a) / static_cast<float>(d) );
    float frac = (num / 50.0f) + 2;

    float crit, rand, STAB, burn;

    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit, 'stages' not implemented
    bool moveIsCritical = (m_rng() % 100) < (1 / 24.0f);
    if (moveIsCritical) crit = 1.25f;
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

    std::uniform_int_distribution<> distr(RAND_BOUND_LOWER, RAND_BOUND_UPPER);
    rand = std::floor(distr(m_rng) / 100.0f);

    // TODO: an 'other' float val is also needed to process the effects of some abilities

    float result = frac * crit * rand * STAB * typeEffect;

    return std::floor(result);
}

std::array<Action, 2> BattleManager::sortActionsByPriority() {

    std::array<Action, 2> actions;

    // NOTE: The large majority of moves have a priority of 0 and thus who goes first will be decided by the speed stat
    // an example of a move with a non-zero priority is quick attack

    int& priorityPlayer = m_playerPkmn->getMoveset()[m_playerAction.index].priority;
    int& priorityOpponent = m_opponentPkmn->getMoveset()[m_opponentAction.index].priority;

    if (priorityPlayer != priorityOpponent) {
        if (priorityPlayer > priorityOpponent) actions = { m_playerAction, m_opponentAction };
        else actions = { m_opponentAction, m_playerAction };
    }
    else {
        // if priorities are equal, whoever has the highest speed stat goes first
        if (m_playerPkmn->m_stats.speed > m_opponentPkmn->m_stats.speed) actions = { m_playerAction, m_opponentAction };
        else actions = { m_opponentAction, m_playerAction };
    }

    return actions;
}

std::array<Action, 2> BattleManager::sortActions(const bool playerUsesMove, const bool& opponentUsesMove) {
    /* This is called if both parties are not simultaneously using a move
     * Using an item is always given priority over moves
     * Switching out pokemon is also always given priority over a move UNLESS that move is pursuit
     *
     * NOTE: Additional checks on the validity of actions like for example blocking item usage when
     * a Pokemon is subjected to embargo are performed after sorting actions
     */

    std::array<Action, 2> actions;

    if (playerUsesMove || opponentUsesMove) { // a move must be considered
        if (playerUsesMove) {
            // TODO: should check against a strID instead of the display name, strIDs are all lower case
            if (m_playerPkmn->getMoveset()[m_playerAction.index].name == "Pursuit") actions = { m_playerAction, m_opponentAction };
            else actions = { m_opponentAction, m_playerAction };
        }
        if (opponentUsesMove) {
            if (m_opponentPkmn->getMoveset()[m_opponentAction.index].name == "Pursuit") actions = { m_opponentAction, m_playerAction };
            else actions = { m_playerAction, m_opponentAction };
        }
    }
    else { // nobody is using a move, which means the order is technically irrelevant, so just return the same order every time
        actions = { m_playerAction, m_opponentAction };
    }

    return actions;
}

void BattleManager::processTurn() {

    std::array<Action, 2> actions; // temp array, used to determine order of actions

    const bool playerUsesMove = m_playerAction.type == ACTION_TYPE_USE_MOVE;
    const bool opponentUsesMove = m_opponentAction.type == ACTION_TYPE_USE_MOVE;

    // Both parties use a move: priority determines execution order.
    if (playerUsesMove && opponentUsesMove) actions = sortActionsByPriority();
    // Only one party uses a move, no need to consider move priority/speed
    else actions = sortActions(playerUsesMove, opponentUsesMove);

    // TODO: The order of execution is now determined, run the actions
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

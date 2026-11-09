#pragma once

#include "../entity/trainer.hpp"
#include "../scene/battle_scene.hpp"
#include "../pokemon/pokemon_common.h"

#include <variant> // type-safe union system

/* NOTE: A 'turn' is the single processing unit that processes the two actions from the player and the opponent */

struct UseMove { int moveIndex; };
struct SwitchPokemon { int pokemonIndex; /* aka. index of pokemon in team */ };

struct UseItem {
    // TODO
};

using Action = std::variant<UseMove, SwitchPokemon, UseItem>;

enum BattleState { // to avoid ridiculous numbers of boolean checks
    STATE_AWAITING_PLAYER_ACTION,
    STATE_AWAITING_OPPONENT_ACTION,
    STATE_ACTIONS_SET,
    STATE_PROCESSING_TURN
};

class BattleManager {

public:

    // the non-owning pointer to battle scene is necessary to switch out the active Pokemon objects
    BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent, BattleScene* scene);
    ~BattleManager();

    // prepare the stage with both players' pokemon (not done on init, as this requires the scene to be set in the renderer to work)
    // this should be called every time any pokemon is changed, the reason it takes the Pokemon objects as parameters instead of using
    // the class members is because this also needs to be used by triggerBattleStart()
    void prepareStage(std::shared_ptr<Pokemon> playerPkmn, std::shared_ptr<Pokemon> opponentPkmn);

    // recieved from UI via C interface (move_interface.h)
    void playerUseMove(const int& index);
    // this can either be recieved via opponent AI or multiplayer turn packet
    void opponentUseMove(const int& index);

    // TODO: add funcs for other turn actions (switch Pokemon & use item)
    void setupMoves();

    void update(); // checks if the manager should begin to process a turn, called by battle scene's update()

private:

    void processTurn();
    int calculateMoveDamage(const Move& move, std::shared_ptr<Pokemon>& user, std::shared_ptr<Pokemon>& target);

    BattleState m_state;

    Action m_playerAction;
    Action m_opponentAction;

    // the moveset data packet, and associated data, that will be sent to the move selection menu
    MoveC m_movesetUI[4];
    MovesetDataC m_playerMovesetDataUI;

    // TODO: Add UI data packets and views for Pokemon and items

    std::shared_ptr<Pokemon> m_playerPkmn;
    std::shared_ptr<Pokemon> m_opponentPkmn;

    std::shared_ptr<Trainer> m_player;
    std::shared_ptr<Trainer> m_opponent;
    BattleScene* m_scene;
};

// used by game event system
void triggerBattleStart();

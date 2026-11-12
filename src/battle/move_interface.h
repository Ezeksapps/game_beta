#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void useMove(void* _this, int index);
void prepareMoveSelector(void* _this, int index);

/* --- C Data packets --- */

#include "../pokemon/pokemon_common.h"

// A version of the C++-side Move structure for the UI system
typedef struct MoveC {
    const char*  name;
    Type         type;
    MoveCategory category;
    int          pp;
    int          power;
    int          accuracy;
    int          priority;
} MoveC;

// user data for move selection screen
typedef struct MovesetDataC {
    MoveC* moveset;
    void* battleManager;
} MovesetDataC;

typedef struct DashboardDataC {
    void* battleManager;
    const char* displayText;
} DashboardDataC;

#ifdef __cplusplus
}
#endif


#include "dialog.hpp"
#include "ui.h"


std::shared_ptr<Entity> g_entity = {};

std::shared_ptr<Entity> getInteractingEntity() { return g_entity; }
void setInteractableEntity(std::shared_ptr<Entity> entity) { g_entity = entity; }

void initiateDialog() {
    if (g_entity == nullptr) return; // there is no colliding entity and this was called on empty space
    // if (!entity->hasDialogString(getCurrentStoryEvent())) {} // ignore other string until they are implemented

    //state.pos = 0;

    setDisplayString(g_entity->getDialogString("default"));
    advanceText(); // print initial piece of dialogue
    pushToStack(entityDialogBox);
}

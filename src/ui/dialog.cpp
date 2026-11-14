#include "dialog.hpp"
#include "ui.h"
#include "text_display.hpp"

std::shared_ptr<Entity> g_entity = {};

std::shared_ptr<Entity> getInteractingEntity() { return g_entity; }
void setInteractableEntity(std::shared_ptr<Entity> entity) { g_entity = entity; }

void initiateDialog() {
    if (g_entity == nullptr) return; // there is no colliding entity and this was called on empty space
    // if (!entity->hasDialogString(getCurrentStoryEvent())) {} // ignore other strings until they are implemented

    setSourceString(g_entity->getDialogString("default"));
    pushToStack(textDisplay1);
}

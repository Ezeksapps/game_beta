#pragma once

#include "../entity/entity.hpp"

std::shared_ptr<Entity> getInteractingEntity();
void setInteractableEntity(std::shared_ptr<Entity> entity);

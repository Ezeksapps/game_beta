#pragma once

#include "entity.hpp"

struct PokemonTemplate {

};

class Trainer : public Entity {

public:

    Trainer(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index);
    ~Trainer();

private:

};

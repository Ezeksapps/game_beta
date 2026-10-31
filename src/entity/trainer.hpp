#pragma once

#include "entity.hpp"

struct PokemonTemplate {

};

class Trainer : public Entity {

public:

    Trainer(const std::string& animJsonFilepath, const int& index);
    ~Trainer();

    void loadEntityJSON(const std::string& filepath) override;

private:

};

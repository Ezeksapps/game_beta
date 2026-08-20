#include "pokemon.hpp"

Pokemon::Pokemon(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index)
                : Entity(entityJsonFilepath, animJsonFilepath, index) {

}

Pokemon::~Pokemon() {}

#include "trainer.hpp"

Trainer::Trainer(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index)
: Entity(animJsonFilepath, index) {


}

Trainer::~Trainer() {}

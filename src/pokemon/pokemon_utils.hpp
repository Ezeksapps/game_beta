#pragma once

#include "pokemon_common.h"

/* Pre-loader functions for JSON data needed for handling Pokemon-type Entity instances */

Ability strToAbility(const std::string& str);
Nature strToNature(const std::string& str);
// may also need reverse funcs


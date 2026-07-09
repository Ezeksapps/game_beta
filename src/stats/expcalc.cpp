#include <cmath>

const int MAX_LEVEL = 100;

typedef enum ExpClass {
    EXP_CLASS_ERRATIC,
    EXP_CLASS_FLUCTUATING,
    EXP_CLASS_FAST,
    EXP_CLASS_MEDIUM_FAST,
    EXP_CLASS_MEDIUM_SLOW,
    EXP_CLASS_SLOW
} ExpClass;

float calcGainedExpErratic(const int& level) {
    if (level < 50) {
        return (std::pow(level, 3) * (100 - level)) / 50;
    }
    else if (level < 68) {
        return (std::pow(level, 3) * (150 - level)) / 100;
    }
    else if (level < 98) {
        return (std::pow(level, 3) * ((1911 - (10 * level)) / 3)) / 500;
    }
    else if (level <= 100) {
        return (std::pow(level, 3) * (160 - level)) / 100;
    }
    else return -1; // error, level cannot exceed 100
}

float calcGainedExpFluctuating(const int& level) {
    if (level < 15) {
        return (std::pow(level, 3) * (((level + 1) / 3) + 24)) / 50;
    }
    else if (level < 36) {
        return (std::pow(level, 3) * (level + 14)) / 50;
    }
    else if (level <= 100) {
        return (std::pow(level, 3) * ((level >> 1) + 32)) / 50;
    }
    else return -1; // error, level cannot exceed 100
}

float calcGainedExpFast(const int& level) {
    return (4 * std::pow(level, 3)) / 5;
}

float calcGainedExpMediumFast(const int& level) {
    return std::pow(level, 3);
}

float calcGainedExpMediumSlow(const int& level) {
    return (1.2 * std::pow(level, 3)) - (15 * std::pow(level, 2)) + (level * 100) - 140;
}

float calcExpGainedSlow(const int& level) {
    return (std::pow(level, 3) * 5) / 4;
}

float calcGainedExp(ExpClass* expClass, const int& level) {
    switch(*expClass) {
        case EXP_CLASS_ERRATIC:
            calcGainedExpErratic(level);
            break;
        case EXP_CLASS_FLUCTUATING:
            calcGainedExpFluctuating(level);
            break;
        case EXP_CLASS_FAST:
            calcGainedExpFast(level);
            break;
        case EXP_CLASS_MEDIUM_FAST:
            break;
        case EXP_CLASS_MEDIUM_SLOW:
            break;
        case EXP_CLASS_SLOW:
            break;
    }
    return -1;
}

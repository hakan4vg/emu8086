//
// Created by xmrhk on 22.12.2024.
//

#include "utils.h"

bool cpu::UTILS::calculateParity(uint16_t value) {
    value ^= value >> 8;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (~value) & 1;
}
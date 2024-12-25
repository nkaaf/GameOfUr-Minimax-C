#ifndef SIM_H
#define SIM_H

#include "state.h"

state_t *simulate(const state_t *state_current, short piece_index, short dice);

float evaluate(const state_t *state_current, const state_t *state_new);

#endif //SIM_H

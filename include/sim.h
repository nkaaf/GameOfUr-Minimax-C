#ifndef SIM_H
#define SIM_H

#include "state.h"

// Public only for testing!
state_t* simulate(const state_t* state_current, short piece_index, short dice);

char get_best_move(const state_t* state_root);

float evaluate(const state_t* state_current, const state_t* state_new);

#endif // SIM_H

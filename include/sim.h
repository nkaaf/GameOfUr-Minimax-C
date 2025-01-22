#ifndef MINIMAX_SIM_H
#define MINIMAX_SIM_H

#include "config.h"
#include "state.h"

// Public only for testing!
state_t* simulate(const state_t* state_current, short piece_index, short dice, const minimax_config_t* config);

// Public only for testing!
void cleanup(state_t* state_root);

char get_best_move(state_t* state_root, const short* dice_first, const minimax_config_t* config);

float evaluate(const state_t* state_current, const state_t* state_new, const minimax_config_t* config);

#endif // MINIMAX_SIM_H

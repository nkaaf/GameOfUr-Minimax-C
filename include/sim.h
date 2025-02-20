#ifndef MINIMAX_SIM_H
#define MINIMAX_SIM_H

#include "config.h"
#include "state.h"

#ifdef BENCHMARK_MINIMAX
#include "stdio.h"
#endif

// Public only for testing!
void cleanup_children(const state_t *state_root);

// Public only for testing!
state_t *simulate(const state_t *state_current, short piece_index, short dice,
                  const minimax_config_t *config);

size_t get_best_piece(state_t *state_root, const short *dice_first,
                      minimax_config_t *config
#ifdef BENCHMARK_MINIMAX
                      ,
                      FILE **logs
#endif
);

float evaluate(const state_t *state_current, const state_t *state_new,
               const minimax_config_t *config);

#endif // MINIMAX_SIM_H

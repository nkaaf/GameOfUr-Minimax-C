#ifndef MINIMAX_VISUALIZE_H
#define MINIMAX_VISUALIZE_H

#include "config.h"
#include "state.h"

void visualize_graph(state_t* state_root, const minimax_config_t* config);

void visualize_path(state_t* state_root, const minimax_config_t* config);

#endif // MINIMAX_VISUALIZE_H

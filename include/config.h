#ifndef MINIMAX_CONFIG_H
#define MINIMAX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    const bool enable;
    const size_t depth;
    const char* graph_path;
} minimax_config_visualizer_t;

#define MINIMAX_GAMEOFUR_STRATEGY_EXPECTED_VALUE 0
#define MINIMAX_GAMEOFUR_STRATEGY_WORST_CASE 1

typedef struct
{
    const float *points_base, *points_rosette;

    const float multiplier_rosette, multiplier_killable, multiplier_attacker, *multiplier_kill_distance;
    const float adder_kill_happens;

    const size_t strategy_evaluation_propagation;
} minimax_config_eval_t;

typedef struct
{
    const size_t depth;
    const bool rosette_middle_safe;
    const uint32_t pieces_player_0, pieces_player_1;
    const short score_player_0, score_player_1;
    const size_t num_of_pieces_per_player;
    const bool player_0_maximize;

    const bool alpha_beta_pruning_enable;

    const minimax_config_eval_t eval_config;
    const minimax_config_visualizer_t visualize_config;
} minimax_config_t;

#endif // MINIMAX_CONFIG_H

#ifndef MINIMAX_CONFIG_H
#define MINIMAX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    bool enable;
    size_t depth;
    const char* graph_path;
} minimax_config_visualizer_t;

#define MINIMAX_GAMEOFUR_STRATEGY_EXPECTED_VALUE 0
#define MINIMAX_GAMEOFUR_STRATEGY_WORST_CASE 1

typedef struct
{
    float* points_base;

    float multiplier_killable, multiplier_attacker, *multiplier_kill_distance;
    float adder_rosette, adder_kill_happens;

    size_t strategy_evaluation_propagation;
} minimax_config_eval_t;

typedef struct
{
    size_t depth;
    bool rosette_middle_safe;
    uint32_t pieces_player_0, pieces_player_1;
    short score_player_0, score_player_1;
    size_t num_of_pieces_per_player;
    bool player_0_maximize;

    bool alpha_beta_pruning_enable;

    minimax_config_eval_t eval_config;
    minimax_config_visualizer_t visualize_config;
} minimax_config_t;

#endif // MINIMAX_CONFIG_H

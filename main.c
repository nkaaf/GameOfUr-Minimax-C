#include <stdio.h>

#include "common.h"
#include "config.h"
#include "sim.h"

int main(void)
{
    float points_base[] = {100, -.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    float multiplier_kill_distance[] = {-1, 1.0f / 4.0f, 3.0f / 8.0f, 1.0f / 4.0f, 1.0f / 16.0f};

    const minimax_config_t config = {
        .depth = 1,
        .rosette_middle_safe = true,
        .pieces_player_0 =
            (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, 2) | PIECE_FIELD_SET(2, 3) |
             PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
             PIECE_FIELD_SET(6, FIELD_START)),
        .pieces_player_1 =
            (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |
             PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
             PIECE_FIELD_SET(6, FIELD_START)),
        .score_player_0 = 0,
        .score_player_1 = 0,
        .num_of_pieces_per_player = 3,
        .player_0_maximize = true,
        .alpha_beta_pruning_enable = true,
        .eval_config = {.points_base = points_base,
                        .adder_rosette = 30,
                        .multiplier_killable = 10,
                        .multiplier_attacker = -1.5,
                        .multiplier_kill_distance = multiplier_kill_distance,
                        .adder_kill_happens = 100,
                        // TODO: Implement
                        .strategy_evaluation_propagation = MINIMAX_GAMEOFUR_STRATEGY_EXPECTED_VALUE},
        .visualize_config = {.enable = true, .depth = 7, .graph_path = "Graph.gv"}};

    state_t* state_root = state_init(config.score_player_0, config.score_player_1, config.pieces_player_0,
                                     config.pieces_player_1, 0, 1, &config);

    short first_dice = 1;
    printf("Move piece: %d", get_best_move(state_root, NULL, &config));

    return 0;
}

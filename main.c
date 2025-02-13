#include <stdio.h>

#include "common.h"
#include "config.h"
#include "sim.h"

int main(void) {
  float points_base[] = {100, 0, 5, 5, 5, 5, 5, 5, 5, 8, 9, 10, 11, 12, 13, 14};

  const minimax_config_t config = {
      .depth = 6,
      .rosette_middle_safe = true,
      .pieces_player_0 =
          (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) |
           PIECE_FIELD_SET(2, FIELD_START) | PIECE_FIELD_SET(3, FIELD_START) |
           PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
           PIECE_FIELD_SET(6, FIELD_START)),
      .pieces_player_1 =
          (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) |
           PIECE_FIELD_SET(2, FIELD_START) | PIECE_FIELD_SET(3, FIELD_START) |
           PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
           PIECE_FIELD_SET(6, FIELD_START)),
      .score_player_0 = 0,
      .score_player_1 = 0,
      .num_of_pieces_per_player = 7,
      .player_to_maximize = 0,
      .alpha_beta_pruning_enable = false,
      .eval_config =
          {
              .points_base = points_base,
              .adder_attack_possible = -100,
              .adder_kill_possible = 2,
              .adder_lose = -100,
              .adder_kill_happens = 10,
          },
      .visualize_config = {.enable = true, .graph_path = "Graph.gv"}};

  state_t *state_root =
      state_init(config.score_player_0, config.score_player_1,
                 config.pieces_player_0, config.pieces_player_1, 0, 1, &config);

  printf("Move piece: %lu", get_best_piece(state_root, NULL, &config));

  return 0;
}

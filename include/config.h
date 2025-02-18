#ifndef MINIMAX_CONFIG_H
#define MINIMAX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  bool enable;
  const char *graph_path;
} minimax_config_visualizer_t;

typedef struct {
  float *points_base;

  float adder_kill_possible, adder_attack_possible, adder_kill_happens;
} minimax_config_eval_t;

typedef struct {
  size_t depth;
  bool rosette_middle_safe;
  uint32_t pieces_player_0, pieces_player_1;
  short score_player_0, score_player_1;
  size_t num_of_pieces_per_player;
  short player_to_maximize;

  bool alpha_beta_pruning_enable;

  minimax_config_eval_t eval_config;
  minimax_config_visualizer_t visualize_config;
} minimax_config_t;

#endif // MINIMAX_CONFIG_H

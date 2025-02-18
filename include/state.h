#ifndef MINIMAX_STATE_H
#define MINIMAX_STATE_H

#include "config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct state_s state_t;

struct state_s {
  size_t id;

  uint8_t score_0, score_1;
  uint32_t pieces_0, pieces_1;
  uint8_t player_current, player_other;
  uint8_t dice;
  uint8_t moved_piece;
  bool second_throw;

  float eval;

  state_t *parent;
  state_t **children;
  size_t child_count;
};

state_t *state_init(short score_0, short score_1, uint32_t pieces_0,
                    uint32_t pieces_1, short player_current, short player_other,
                    const minimax_config_t *config);

void state_free(state_t *state);

void state_reset_ids();

bool state_check_win(const state_t *state, const minimax_config_t *config,
                     short player);

void state_swap_player(state_t *state);

void state_piece_move(state_t *state, short player, size_t piece_index,
                      uint8_t dest);

void state_add_child(state_t *state, state_t *child);

bool state_equals(const state_t *state1, const state_t *state2);

void state_iterate_over_all_children_and_execute(const state_t *state,
                                                 void (*func)(state_t *));

#endif // MINIMAX_STATE_H

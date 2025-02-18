#include "state.h"

#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

size_t id = -1;

state_t *state_init(const short score_0, const short score_1,
                    const uint32_t pieces_0, const uint32_t pieces_1,
                    const short player_current, const short player_other,
                    const minimax_config_t *config) {
  state_t *state = malloc(sizeof(state_t));
  if (!state) {
    assert(false && "malloc failed");
  }

  state->id = ++id;

  state->score_0 = score_0;
  state->score_1 = score_1;

  state->pieces_0 = pieces_0;
  state->pieces_1 = pieces_1;

  state->player_current = player_current;
  state->player_other = player_other;

  state->dice = -1;

  state->moved_piece = -1;

  state->second_throw = false;

  state->eval = 0;

  state->parent = NULL;

  state->children = calloc(config->num_of_pieces_per_player * DICE_RANGE_TRUE,
                           sizeof(state_t *));
  if (!state->children) {
    state_free(state);
    assert(false && "calloc failed");
  }

  state->child_count = 0;

  return state;
}

void state_free(state_t *state) {
  if (state) {
    if (state->children) {
      free(state->children);
    }
    free(state);
  }
}

void state_reset_ids() { id = -1; }

bool state_check_win(const state_t *state, const minimax_config_t *config,
                     const short player) {
  assert(state != NULL && "state is null");
  assert(player == 0 || player == 1 && "Player is incorrect");

  return (player == 0 ? state->score_0 : state->score_1) ==
         config->num_of_pieces_per_player;
}

void state_swap_player(state_t *state) {
  assert(state != NULL && "state is null");

  const short tmp = state->player_current;
  state->player_current = state->player_other;
  state->player_other = tmp;
}

void state_piece_move(state_t *state, const short player,
                      const size_t piece_index, const uint8_t dest) {
  assert(state != NULL && "state is null");
  assert(player == 0 || player == 1 && "player is incorrect");

  uint32_t *pieces = player == 0 ? &state->pieces_0 : &state->pieces_1;
  uint8_t *score = player == 0 ? &state->score_0 : &state->score_1;

  GET_PIECE_MASK(piece_mask_clear, piece_index);
  piece_mask_clear = ~piece_mask_clear;

  *pieces &= piece_mask_clear;
  *pieces |= PIECE_FIELD_SET(piece_index, dest);

  if (dest == FIELD_FINISH) {
    /* Player has finished */
    *score += 1;
  }
}

void state_add_child(state_t *state, state_t *child) {
  assert(state != NULL && "state is null");
  assert(child != NULL && "child is null");

  state->children[state->child_count++] = child;

  child->parent = state;
}

bool state_equals(const state_t *state1, const state_t *state2) {
  return state1->score_0 == state2->score_0 &&
         state1->score_1 == state2->score_1 &&
         state1->pieces_0 == state2->pieces_0 &&
         state1->pieces_1 == state2->pieces_1 &&
         state1->player_current == state2->player_current &&
         state1->player_other == state2->player_other &&
         state1->dice == state2->dice &&
         state1->moved_piece == state2->moved_piece &&
         state1->second_throw == state2->second_throw &&
         state1->parent == state1->parent &&
         state1->children == state1->children && state1->eval == state1->eval &&
         state1->child_count == state1->child_count;
}

void state_iterate_over_all_children_and_execute(const state_t *state,
                                                 void (*func)(state_t *)) {
  if (state->child_count == 0) {
    return;
  }

  for (size_t child_index = 0; child_index < state->child_count;
       child_index++) {
    state_t *child = state->children[child_index];
    func(child);
  }
}

#include "sim.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "graphviz.h"

#include <float.h>

void cleanup_children(const state_t *state_root) {
  state_iterate_over_all_children_and_execute(state_root, state_free);
}

state_t *simulate(const state_t *state_current, const short piece_index,
                  const short dice, const minimax_config_t *config) {
  state_t *state_new = NULL;

  assert((state_current->player_current == 0 ||
          state_current->player_current == 1) &&
         "Incorrect player");

  if (dice == 0) {
    // No state change.
    return NULL;
  }

  const uint32_t pieces_current_player = state_current->player_current == 0
                                             ? state_current->pieces_0
                                             : state_current->pieces_1;
  const uint32_t pieces_other_player = state_current->player_current == 0
                                           ? state_current->pieces_1
                                           : state_current->pieces_0;

  PIECE_FIELD_GET(piece_field, pieces_current_player, piece_index);

  if (piece_field == FIELD_FINISH) {
    // Piece is already in finish

    return NULL;
  }

  const uint8_t piece_field_dest = piece_field + dice;

  if (PIECE_CANNOT_FINISH(piece_field_dest)) {
    // Move cannot be done, because the piece has to finish perfect

    return NULL;
  }

  if (piece_field_dest != FIELD_FINISH &&
      any_piece_on_field(pieces_current_player, piece_field_dest, NULL,
                         config)) {
    // Move cannot be done, because the current player is already on the
    // destination field.

    return NULL;
  }

  if (config->rosette_middle_safe && piece_field_dest == ROSETTE_SAFE &&
      any_piece_on_field(pieces_other_player, piece_field_dest, NULL, config)) {
    // Move cannot be done, because the other player is on the safe field.

    return NULL;
  }

  // Only valid moves from here

  state_new = state_init(state_current->score_0, state_current->score_1,
                         state_current->pieces_0, state_current->pieces_1,
                         state_current->player_current,
                         state_current->player_other, config);

  state_new->dice = dice;
  state_new->moved_piece = piece_index;

  if (PIECE_CAN_FINISH(piece_field_dest)) {
    // Piece is in finish with next move

    state_piece_move(state_new, state_current->player_current, piece_index,
                     FIELD_FINISH);
  } else {
    // Move is valid, but piece is not in finish

    size_t piece_index_other;
    if (6 <= piece_field_dest && piece_field_dest <= 13 &&
        any_piece_on_field(pieces_other_player, piece_field_dest,
                           &piece_index_other, config)) {
      // This can only be reached, if next field on war zone
      // Piece of other player is caught and returned to start
      state_piece_move(state_new, state_current->player_other,
                       piece_index_other, FIELD_START);
    }

    if (piece_field_dest == 5 || piece_field_dest == ROSETTE_SAFE ||
        piece_field_dest == 15) {
      // Destination field is a rosette
      state_new->second_throw = true;
    }

    state_piece_move(state_new, state_current->player_current, piece_index,
                     piece_field_dest);
  }

  if (!state_new->second_throw) {
    // Swap player if there is no second throw by current player
    state_swap_player(state_new);
  }

  return state_new;
}

void calculate_all_children_of_piece(state_t *state, const short piece_index,
                                     const short *dice_first,
                                     const minimax_config_t *config) {
  if (dice_first) {
    // Known dice throw

    state_t *state_new = simulate(state, piece_index, *dice_first, config);

    if (state_new) {
      state_add_child(state, state_new);
    }
  } else {
    // Simulate all dices

    for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++) {
      state_t *state_new = simulate(state, piece_index, dice, config);
      if (state_new) {
        state_add_child(state, state_new);
      }
    }
  }
}

void minimax(state_t *state_current, const size_t depth, const bool maximize,
             const short *dice, size_t *piece_index_to_move,
             const minimax_config_t *config) {
  if (depth == 0) {
    state_current->eval =
        evaluate(state_current->parent, state_current, config);
  } else {
    float evaluation_score = maximize ? -FLT_MAX : +FLT_MAX;
    for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player;
         piece_index++) {
      const size_t child_count_before = state_current->child_count;
      calculate_all_children_of_piece(state_current, (uint8_t)piece_index, dice,
                                      config);
      const size_t child_count_after = state_current->child_count;
      if (child_count_after == child_count_before) {
        // This piece has no possible move
        continue;
      }

      // Array to check, which dices where possible moves
      // This array will be altered in for loop below
      // After: 1 in array = dice was not possible; 0 in array = dice was
      // possible
      uint8_t *dices_of_child = malloc(sizeof(uint8_t) * DICE_RANGE_TRUE);
      assert(dices_of_child != NULL && "malloc failed");
      memset(dices_of_child, 1, DICE_RANGE_TRUE * sizeof(uint8_t));

      float expecting_value_acc = 0;
      for (size_t index_child = child_count_before;
           index_child < child_count_after; index_child++) {
        state_t *child = state_current->children[index_child];

        const bool maximize_next_level =
            config->player_to_maximize == child->player_current;

        minimax(child, depth - 1, maximize_next_level, NULL,
                piece_index_to_move, config);

        expecting_value_acc += child->eval * throw_probability[child->dice];
        dices_of_child[child->dice] == 0;
      }

      // Add evaluation of unchanged states ("not possible dices)
      // This is important to preserve the sum of 1 for all probabilities
      const float eval_current = evaluate(state_current, state_current, config);
      for (size_t i = 0; i < DICE_RANGE_TRUE; i++) {
        if (dices_of_child[i] != 0) {
          expecting_value_acc +=
              eval_current * throw_probability[MIN_DICE_THROW + i];
        }
      }

      // This is used to check if evaluation score has
      // changed; this prevents that same
      // evaluation scores changed the piece to move
      const float evaluation_score_tmp = evaluation_score;

      evaluation_score = maximize
                             ? fmaxf(expecting_value_acc, evaluation_score)
                             : fminf(expecting_value_acc, evaluation_score);
      if (depth == config->depth && evaluation_score_tmp != evaluation_score &&
          evaluation_score == expecting_value_acc) {
        // This piece should be moved (current knowledge)
        *piece_index_to_move = piece_index;
      }

      if (config->alpha_beta_pruning_enable) {
        //printf("Expecting: %f - Max: %i - cur a: %f - cur b: %f\n", expecting_value_acc,maximize,
        //       state_current->alpha, state_current->beta);
        if (maximize) {
          state_current->alpha =
              fmaxf(expecting_value_acc, state_current->alpha);
        } else {
          state_current->beta = fminf(expecting_value_acc, state_current->beta);
        }
        if (state_current->beta <= state_current->alpha) {
          printf("Prune\n");
          break;
        }
      }
    }

    if (state_current->child_count == 0) {
      // Edge Case: No piece can be moved
      // Create unchanged state
      state_t *state_unchanged = state_init(
          state_current->score_0, state_current->score_1,
          state_current->pieces_0, state_current->pieces_1,
          state_current->player_current, state_current->player_other, config);
      state_swap_player(state_unchanged);
      state_add_child(state_current, state_unchanged);

      // Check if the next level is to maximize or to minimize ("fix" for second
      // throw)
      const bool maximize_next_level =
          config->player_to_maximize == state_current->player_current;

      minimax(state_unchanged, depth - 1, maximize_next_level, NULL,
              piece_index_to_move, config);

      evaluation_score = state_unchanged->eval;
    }

    if (config->visualize_config.enable) {
      visualize_add_node(state_current->id, state_current->eval,
                         state_current->player_current,
                         config->player_to_maximize);

      for (size_t child_index = 0; child_index < state_current->child_count;
           child_index++) {
        const state_t *child = state_current->children[child_index];
        visualize_add_node(child->id, child->eval, child->player_current,
                           config->player_to_maximize);
        visualize_add_edge(state_current->id, child->id, child->dice,
                           child->moved_piece);
      }
    }
    cleanup_children(state_current);

    state_current->child_count = 0;
    state_current->eval = evaluation_score;
  }
}

size_t get_best_piece(state_t *state_root, const short *dice_first,
                      const minimax_config_t *config) {
  if (config->visualize_config.enable) {
    visualize_init(config->visualize_config.graph_path);
  }

  size_t piece_index_to_move;
  (void)minimax(state_root, config->depth, true, dice_first,
                &piece_index_to_move, config);

  if (config->visualize_config.enable) {
    visualize_finalize();
    visualize_free();
  }

  state_reset_ids();

  return piece_index_to_move;
}

float evaluate(const state_t *state_current, const state_t *state_new,
               const minimax_config_t *config) {
  const uint32_t pieces_new_player_max = config->player_to_maximize == 0
                                             ? state_new->pieces_0
                                             : state_new->pieces_1;
  const uint32_t pieces_new_player_min = config->player_to_maximize == 0
                                             ? state_new->pieces_1
                                             : state_new->pieces_0;
  const uint32_t pieces_current_player_min =
      state_new->second_throw && config->player_to_maximize == 0
          ? state_current->pieces_1
      : state_new->second_throw         ? state_current->pieces_0
      : config->player_to_maximize == 0 ? state_current->pieces_1
                                        : state_current->pieces_0;

  float points_total = 0.0f;

  for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player;
       piece_index++) {
    PIECE_FIELD_GET(piece_field, pieces_new_player_max, piece_index);

    // Base points
    points_total += config->eval_config.points_base[piece_field];

    // Killed piece of min player
    short count_pieces_other_player_start_current = 0;
    for (size_t i = 0; i < config->num_of_pieces_per_player; i++) {
      PIECE_FIELD_GET(piece_field, pieces_current_player_min, i);
      if (piece_field == FIELD_START) {
        count_pieces_other_player_start_current += 1;
      }
    }
    short count_pieces_other_player_start_new = 0;
    for (size_t i = 0; i < config->num_of_pieces_per_player; i++) {
      PIECE_FIELD_GET(piece_field, pieces_new_player_min, i);
      if (piece_field == FIELD_START) {
        count_pieces_other_player_start_new += 1;
      }
    }
    const bool kill_happens = count_pieces_other_player_start_new !=
                              count_pieces_other_player_start_current;
    points_total +=
        (float)kill_happens * config->eval_config.adder_kill_happens;

    // Kill to min player piece is possible
    if (piece_field > 1 && piece_field < 13) {
      double probability = 0;
      for (short i = 1; i <= MAX_DICE_THROW; i++) {
        if (any_piece_on_field(pieces_new_player_min, piece_field + i, NULL,
                               config)) {
          probability += throw_probability[i];
        }
      }
      points_total +=
          (float)config->eval_config.adder_kill_possible * probability;
    }

    // Kill to max player piece is possible
    if (piece_field > 5 && piece_field < 14) {
      double probability = 0;
      for (short i = 1; i <= MAX_DICE_THROW; i++) {
        if (any_piece_on_field(pieces_new_player_min, piece_field - i, NULL,
                               config)) {
          probability += throw_probability[i];
        }
      }
      points_total +=
          (float)config->eval_config.adder_attack_possible * probability;
    }
  }

  if (state_check_win(state_new, config,
                      config->player_to_maximize == 0 ? 1 : 0)) {
    // Min won
    // https://de.wikipedia.org/wiki/Expectiminimax-Algorithmus#Bewertungsfunktion
    points_total += config->eval_config.adder_lose;
  }

  return points_total;
}

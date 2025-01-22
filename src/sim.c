#include "sim.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "visualize.h"


state_t* simulate(const state_t* state_current, const short piece_index, const short dice,
                  const minimax_config_t* config)
{
    state_t* state_new = NULL;

    assert((state_current->player_current == 0 || state_current->player_current == 1) && "Incorrect player");

    if (dice == 0)
    {
        // No state change.
        return NULL;
    }

    const uint32_t pieces_current_player =
        state_current->player_current == 0 ? state_current->pieces_0 : state_current->pieces_1;
    const uint32_t pieces_other_player =
        state_current->player_current == 0 ? state_current->pieces_1 : state_current->pieces_0;

    PIECE_FIELD_GET(piece_field, pieces_current_player, piece_index);

    if (piece_field == FIELD_FINISH)
    {
        // Piece is already in finish

        return NULL;
    }

    const uint8_t piece_field_dest = piece_field + dice;

    if (PIECE_CANNOT_FINISH(piece_field_dest))
    {
        // Move cannot be done, because the piece has to finish perfect

        return NULL;
    }

    if (piece_field_dest != FIELD_FINISH && any_piece_on_field(pieces_current_player, piece_field_dest, NULL, config))
    {
        // Move cannot be done, because the current player is already on the destination field.

        return NULL;
    }

    if (config->rosette_middle_safe && piece_field_dest == ROSETTE_SAFE &&
        any_piece_on_field(pieces_other_player, piece_field_dest, NULL, config))
    {
        // Move cannot be done, because the other player is on the safe field.

        return NULL;
    }

    // Only valid moves from here

    state_new = state_init(state_current->score_0, state_current->score_1, state_current->pieces_0,
                           state_current->pieces_1, state_current->player_current, state_current->player_other, config);

    const short dices[] = {dice};
    state_dices_setter(state_new, dices, 1);
    state_new->moved_piece = piece_index;

    if (PIECE_CAN_FINISH(piece_field_dest))
    {
        // Piece is in finish with next move

        state_piece_move(state_new, state_current->player_current, piece_index, FIELD_FINISH);
    }
    else
    {
        // Move is valid, but piece is not in finish

        size_t piece_index_other;
        if (6 <= piece_field_dest && piece_field_dest <= 13 &&
            any_piece_on_field(pieces_other_player, piece_field_dest, &piece_index_other, config))
        {
            // This can only be reached, if next field on war zone
            // Piece of other player is caught and returned to start
            state_piece_move(state_new, state_current->player_other, piece_index_other, FIELD_START);
        }

        if (piece_field_dest == 5 || piece_field_dest == ROSETTE_SAFE || piece_field_dest == 15)
        {
            // Destination field is a rosette
            state_new->second_throw = true;
        }

        state_piece_move(state_new, state_current->player_current, piece_index, piece_field_dest);
    }

    if (!state_new->second_throw)
    {
        // Swap player if there is no second throw by current player
        state_swap_player(state_new);
    }

    return state_new;
}

state_t* simulate_wrapper(state_t* state_current, const short piece_index, const short dice,
                          const minimax_config_t* config)
{
    state_t* state_new = simulate(state_current, piece_index, dice, config);
    if (!state_new)
    {
        // Movement is not possible
        return NULL;
    }

    state_add_child(state_current, state_new);
    return state_new;
}

void cleanup(state_t* state_root) { state_iterate_over_all_children_and_execute(state_root, 0, state_free); }

void reset_all_state_child_iters(state_t* state_root)
{
    state_iterate_over_all_children_and_execute(state_root, 0, state_reset_child_iter);
}

size_t calculate_all_children_by_piece(state_t* state, const short piece_index, const short* dice_first,
                                       short* dices_with_no_state_change, const minimax_config_t* config)
{
    size_t dice_with_no_state_change_len = 0;
    if (dice_first == NULL)
    {
        for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
        {
            if (!simulate_wrapper(state, piece_index, dice, config))
            {
                dices_with_no_state_change[dice_with_no_state_change_len++] = dice;
            }
        }
    }
    else
    {
        // Known dice throw
        if (!simulate_wrapper(state, piece_index, *dice_first, config))
        {
            dices_with_no_state_change[dice_with_no_state_change_len] = *dice_first;
        }
    }
    return dice_with_no_state_change_len;
}

float minimax(state_t* state_current, const size_t depth, const bool maximize, const short* dice,
              size_t* piece_index_to_move, const minimax_config_t* config)
{
    if (depth == 0)
    {
        state_current->eval = evaluate(state_current->parent, state_current, config);
    }
    else
    {
        float evaluation_score = maximize ? -FLT_MAX : FLT_MAX;
        for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player; piece_index++)
        {
            short* dices_with_no_state_change = calloc(DICE_RANGE_TRUE, sizeof(short));

            // Calculate all children of piece
            const size_t index_child_start = state_current->child_iter_max + 1;
            const size_t dice_with_no_state_change_len = calculate_all_children_by_piece(
                state_current, (short)piece_index, dice, dices_with_no_state_change, config);
            const size_t index_child_end = state_current->child_iter_max + 1;

            if (index_child_end - index_child_start != DICE_RANGE_TRUE)
            {
                // Not all children states are valid
                // Create one state for all dice throws which result in no state change.
                state_t* state_unchanged = state_init(
                    state_current->score_0, state_current->score_1, state_current->pieces_0, state_current->pieces_1,
                    state_current->player_current, state_current->player_other, config);
                state_swap_player(state_unchanged);
                state_dices_setter(state_unchanged, dices_with_no_state_change, dice_with_no_state_change_len);
                state_unchanged->moved_piece = (short)piece_index;
                state_add_child(state_current, state_unchanged);
            }
            free(dices_with_no_state_change);

            float expecting_value_acc = 0;
            for (size_t index_child = index_child_start; index_child <= index_child_end; index_child++)
            {
                state_t* child = state_current->children[index_child];

                // Check if the next level is to maximize or to minimize ("fix" for second throw)
                const bool maximize_next_level = (config->player_0_maximize && child->player_current == 0) ||
                    (!config->player_0_maximize && child->player_current == 1);

                const float eval = minimax(child, depth - 1, maximize_next_level, NULL, piece_index_to_move, config);

                for (size_t dice_index = 0; dice_index < child->dices_count; dice_index++)
                {
                    expecting_value_acc += eval * throw_probability[child->dices[dice_index]];
                }
            }

            evaluation_score =
                maximize ? fmaxf(expecting_value_acc, evaluation_score) : fminf(expecting_value_acc, evaluation_score);
            if (evaluation_score == expecting_value_acc)
            {
                // This piece should be moved (current knowledge)
                *piece_index_to_move = piece_index;
            }

            if (config->alpha_beta_pruning_enable)
            {
                if (maximize)
                {
                    state_current->alpha = fmaxf(expecting_value_acc, state_current->alpha);
                }
                else
                {
                    state_current->beta = fmaxf(expecting_value_acc, state_current->beta);
                }
                if (state_current->beta <= state_current->alpha)
                {
                    break;
                }
            }
        }
        state_current->eval += evaluation_score;
    }
    return state_current->eval;


    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------
    //---------------------------------------------------------------


    // if (maximize)
    //{
    //     for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player; piece_index++)
    //     {
    //         // Calculate all children from piece_index
    //         const size_t index_child_start = state_current->child_iter_max + 1;
    //         calculate_all_children_by_piece(state_current, (short)piece_index, dice_first, config);
    //         const size_t index_child_end = state_current->child_iter_max + 1;
    //
    //        if (index_child_end == index_child_start)
    //        {
    //            // No state was created
    //            continue;
    //        }
    //
    //        // get worst case of children evaluation
    //        float eval_min = +FLT_MAX;
    //        for (size_t index_child = index_child_start; index_child < index_child_end; index_child++)
    //        {
    //            state_t* child = state_current->children[index_child];
    //
    //            // Check if the next level is to maximize or to minimize ("fix" for second throw)
    //            const bool maximize_next_level = (config->player_0_maximize && child->player_current == 0) ||
    //                (!config->player_0_maximize && child->player_current == 1);
    //
    //            const float eval = minimax(child, depth - 1, maximize_next_level, NULL, config);
    //
    //            if (state_current->children[index_child]->dice != 0)
    //            {
    //                // ignore child with dice == 0
    //
    //                eval_min = fminf(eval_min, eval);
    //            }
    //        }
    //
    //        if (state_current->child_iter_max == -1 ||
    //            (state_current->child_iter_max == 0 && state_current->children[0]->dice == 0))
    //        {
    //            // Compensation, if no move, or only one move (dice == 0) is possible
    //            eval_min = -FLT_MAX;
    //        }
    //
    //        state_current->eval = state_current->alpha = fmaxf(state_current->alpha, eval_min);
    //        if (state_current->beta <= state_current->alpha)
    //        {
    //            break;
    //        }
    //    }
    //
    //    return state_current->eval;
    //}
    // else
    //{
    //    state_current->eval = +FLT_MAX;
    //
    //    for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player; piece_index++)
    //    {
    //        // Calculate all children from piece_index
    //        const size_t index_child_start = state_current->child_iter_max + 1;
    //        calculate_all_children_by_piece(state_current, (short)piece_index, dice_first, config);
    //        const size_t index_child_end = state_current->child_iter_max + 1;
    //
    //        if (index_child_end == index_child_start)
    //        {
    //            // No state was created
    //            continue;
    //        }
    //
    //        // get worst case of children evaluation
    //        float eval_min = +FLT_MAX;
    //        for (size_t index_child = index_child_start; index_child < index_child_end; index_child++)
    //        {
    //            state_t* child = state_current->children[index_child];
    //            const bool maximize_next_level = (config->player_0_maximize && child->player_current == 0) ||
    //                (!config->player_0_maximize && child->player_current == 1);
    //
    //            const float eval = minimax(child, depth - 1, maximize_next_level, NULL, config);
    //
    //            if (state_current->children[index_child]->dice != 0)
    //            {
    //                // ignore child with dice == 0
    //
    //                eval_min = fminf(eval_min, eval);
    //            }
    //        }
    //
    //        if (state_current->child_iter_max == -1 ||
    //            (state_current->child_iter_max == 0 && state_current->children[0]->dice == 0))
    //        {
    //            // Compensation, if no move, or only one move (dice == 0) is possible
    //            eval_min = -FLT_MAX;
    //        }
    //
    //        state_current->eval = state_current->beta = fminf(state_current->beta, eval_min);
    //        if (state_current->beta <= state_current->alpha)
    //        {
    //            break;
    //        }
    //    }
    //    return state_current->eval;
    //}
    //
    // assert(false);
}

char get_best_move(state_t* state_root, const short* dice_first, const minimax_config_t* config)
{
    size_t piece_index_to_move;
    const float i = minimax(state_root, config->depth, true, dice_first, &piece_index_to_move, config);
    assert(i == state_root->eval);

    const char moved_piece = piece_index_to_move;

    if (config->visualize_config.enable)
    {
        reset_all_state_child_iters(state_root);
        visualize_graph(state_root, config);

        reset_all_state_child_iters(state_root);
        visualize_path(state_root, config);
    }

    cleanup(state_root);
    state_reset_ids();

    return moved_piece;
}

float evaluate(const state_t* state_current, const state_t* state_new, const minimax_config_t* config)
{
    if (state_check_win(state_new, config))
    {
        return +FLT_MAX;
    }

    const uint32_t pieces_new_player_max = config->player_0_maximize ? state_new->pieces_0 : state_new->pieces_1;
    const uint32_t pieces_new_player_min = config->player_0_maximize ? state_new->pieces_1 : state_new->pieces_0;
    const uint32_t pieces_current_player_min = state_new->second_throw && config->player_0_maximize
        ? state_current->pieces_1
        : state_new->second_throw   ? state_current->pieces_0
        : config->player_0_maximize ? state_current->pieces_1
                                    : state_current->pieces_0;
    const uint32_t pieces_current_player_max = state_new->second_throw && config->player_0_maximize
        ? state_current->pieces_0
        : state_new->second_throw   ? state_current->pieces_1
        : config->player_0_maximize ? state_current->pieces_0
                                    : state_current->pieces_1;

    float points_total = 0.0f;

    for (size_t piece_index = 0; piece_index < config->num_of_pieces_per_player; piece_index++)
    {
        PIECE_FIELD_GET(piece_field, pieces_new_player_max, piece_index);

        // Base points
        points_total += config->eval_config.points_base[piece_field];
        // Rosette Bonus
        const bool piece_on_rosette =
            piece_field == 5 || piece_field == 15 || (config->rosette_middle_safe && piece_field == 9);
        const bool piece_is_moved = state_current->moved_piece == piece_index;
        const bool state_changed = pieces_current_player_max != pieces_new_player_max;

        points_total +=
            (float)(state_changed && piece_is_moved && piece_on_rosette) * config->eval_config.adder_rosette;

        if (piece_field == FIELD_START || piece_field == 14 || piece_field == 15 || piece_field == FIELD_FINISH)
        {
            // These pieces cannot kill any piece of other player and cannot be killed by other player

            continue;
        }

        // Kill piece of other player
        short count_killable_pieces_of_other_player = 0;
        for (short i = 1; i <= MAX_DICE_THROW; i++)
        {
            if (any_piece_on_field(pieces_new_player_min, piece_field + i, NULL, config))
            {
                count_killable_pieces_of_other_player += 1;
            }
        }
        points_total += (float)count_killable_pieces_of_other_player * config->eval_config.multiplier_killable;

        // Killed by other player
        if (6 <= piece_field)
        {
            short count_attacker_pieces_of_other_player = 0;
            for (short i = 1; i <= MAX_DICE_THROW; i++)
            {
                if (any_piece_on_field(pieces_new_player_min, piece_field - i, NULL, config))
                {
                    count_attacker_pieces_of_other_player += 1;
                }
            }
            points_total += (float)count_attacker_pieces_of_other_player * config->eval_config.multiplier_attacker;
        }
    }

    if ((config->player_0_maximize && state_current->player_current == 0) ||
        (!config->player_0_maximize && state_current->player_current == 1))
    {
        // Improvement of state
        short count_pieces_other_player_start_current = 0;
        for (size_t i = 0; i < config->num_of_pieces_per_player; i++)
        {
            PIECE_FIELD_GET(piece_field, pieces_current_player_min, i);
            if (piece_field == FIELD_START)
            {
                count_pieces_other_player_start_current += 1;
            }
        }
        short count_pieces_other_player_start_new = 0;
        for (size_t i = 0; i < config->num_of_pieces_per_player; i++)
        {
            PIECE_FIELD_GET(piece_field, pieces_new_player_min, i);
            if (piece_field == FIELD_START)
            {
                count_pieces_other_player_start_new += 1;
            }
        }

        const bool kill_happens = count_pieces_other_player_start_new != count_pieces_other_player_start_current;
        points_total += (float)kill_happens * config->eval_config.adder_kill_happens;
    }

    return points_total;
}

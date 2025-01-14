#include "sim.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

#include "common.h"
#include "config.h"

#if VISUALIZE
#include "visualize.h"
#endif /* VISUALIZE */


state_t* simulate(const state_t* state_current, const short piece_index, const short dice)
{
    state_t* state_new = NULL;

    assert((state_current->player_current == 1 || state_current->player_current == 2) && "Incorrect player");

    if (dice != 0)
    {
        const uint32_t pieces_current_player =
            state_current->player_current == 1 ? state_current->pieces_1 : state_current->pieces_2;
        const uint32_t pieces_other_player =
            state_current->player_current == 1 ? state_current->pieces_2 : state_current->pieces_1;

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

        if (piece_field_dest != FIELD_FINISH && any_piece_on_field(pieces_current_player, piece_field_dest, NULL))
        {
            // Move cannot be done, because the current player is already on the destination field.

            return NULL;
        }

        if (ROSETTE_9_IS_SAFE && piece_field_dest == ROSETTE_SAFE &&
            any_piece_on_field(pieces_other_player, piece_field_dest, NULL))
        {
            // Move cannot be done, because the other player is on the safe field.

            return NULL;
        }

        // Only valid moves from here

        state_new = state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
                               state_current->pieces_2, state_current->player_current, state_current->player_other);

        state_new->dice = dice;
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
                any_piece_on_field(pieces_other_player, piece_field_dest, &piece_index_other))
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
    }
    else
    {
        state_new = state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
                               state_current->pieces_2, state_current->player_current, state_current->player_other);

        state_new->dice = dice;
        state_new->moved_piece = -1; // no piece was moved
    }

    if (!state_new->second_throw)
    {
        // Swap player if there is no second throw by current player
        state_swap_player(state_new);
    }

    return state_new;
}

void simulate_wrapper(state_t* state_current, const short piece_index, const short dice)
{
    printf("Simulate dice: %hd\n", dice);

    state_t* state_new = simulate(state_current, piece_index, dice);
    if (!state_new)
    {
        // Movement is not possible
        return;
    }

    printf("Simulated id: %lu\n", state_new->id);

    state_add_child(state_current, state_new);

    // TODO: Behandlung
    // state_check_win(state_new);
}

void cleanup(state_t* state_root) { state_iterate_over_all_children_and_execute(state_root, 0, state_free); }

void reset_all_state_child_iters(state_t* state_root)
{
    state_iterate_over_all_children_and_execute(state_root, 0, state_reset_child_iter);
}

float minimax(state_t* state_current, const size_t depth, const bool maximize)
{
    if (depth == 0)
    {
        state_current->eval = evaluate(state_current->parent, state_current);
        return state_current->eval;
    }

    {
        // Simulate all children of this state
        printf("Simulate step: %lu\n", STEPS_IN_FUTURE - depth + 1);
        for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
        {
            printf("Simulate piece: %hd\n", piece_index);
            for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
            {
                simulate_wrapper(state_current, piece_index, dice);
            }
            printf("\n");

            if (state_get_children_count(state_current) == 0)
            {
                // No move was possible

                state_t* state_new =
                    state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
                               state_current->pieces_2, state_current->player_current, state_current->player_other);
                state_swap_player(state_new);

                state_add_child(state_current, state_new);
            }
        }
    }

    if (maximize)
    {
        state_current->eval = -FLT_MAX;

        for (size_t index_current_child = 0; index_current_child <= state_current->child_iter_max;
             index_current_child++)
        {
            const float eval = minimax(state_current->children[index_current_child], depth - 1, false);
            state_current->eval = fmaxf(state_current->eval, eval);
            state_current->alpha = fmaxf(state_current->alpha, eval);
            if (state_current->beta <= state_current->alpha)
            {
                break;
            }
        }

        return state_current->eval;
    }
    else
    {
        state_current->eval = +FLT_MAX;

        for (size_t index_current_child = 0; index_current_child <= state_current->child_iter_max;
             index_current_child++)
        {
            const float eval = minimax(state_current->children[index_current_child], depth - 1, true);
            state_current->eval = fminf(state_current->eval, eval);
            state_current->beta = fminf(state_current->beta, eval);
            if (state_current->beta <= state_current->alpha)
            {
                break;
            }
        }
        return state_current->eval;
    }

    assert(false);
}

char get_best_move(state_t* state_root, const short dice_first)
{
    float i = minimax(state_root, STEPS_IN_FUTURE, true);

#if VISUALIZE
    reset_all_state_child_iters(state_root);
    visualize_graph(state_root);

    reset_all_state_child_iters(state_root);
    visualize_path(state_root);
#endif /* VISUALIZE */

    cleanup(state_root);
}

// char get_best_move(state_t* state_root, const short dice_first)
//{
//     // dice_first == -1 -> all dice throws are calculated on first level
//
//     state_t* state_current = state_root;
//
//     size_t step_current = 0;
//
//     while (state_current)
//     {
//         for (size_t step = step_current; step < STEPS_IN_FUTURE; step++)
//         {
//             const bool should_evaluate = step == STEPS_IN_FUTURE - 1;
//
//             printf("Simulate step: %lu\n", step);
//             for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
//             {
//                 printf("Simulate piece: %hd\n", piece_index);
//                 if (dice_first != -1 && step_current == 0)
//                 {
//                     // If first dice throw is known, simulate only this.
//                     simulate_wrapper(state_current, piece_index, dice_first, should_evaluate);
//                 }
//                 else
//                 {
//                     // Simulate all possible dice throws
//                     for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
//                     {
//                         simulate_wrapper(state_current, piece_index, dice, should_evaluate);
//                     }
//                 }
//                 printf("\n");
//             }
//
//             if (state_get_children_count(state_current) == 0)
//             {
//                 // No move was possible
//
//                 state_t* state_new =
//                     state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
//                                state_current->pieces_2, state_current->player_current, state_current->player_other);
//                 state_swap_player(state_new);
//
//                 state_new->eval = evaluate(state_current, state_new);
//
//                 state_add_child(state_current, state_new);
//             }
//
//             if (step != STEPS_IN_FUTURE - 1)
//             {
//                 state_current = state_get_next_child(state_current);
//             } else if (step == STEPS_IN_FUTURE - 1)
//             {
//                 int i = 0;
//             }
//         }
//
//         step_current = STEPS_IN_FUTURE - 1;
//         state_current = state_get_next_child_of_parent_recursive(state_current, &step_current);
//     }
//
//     // TODO: To implement
//     //  Return value is the index of the piece (zero-based)
//
//
// #if VISUALIZE
//     reset_all_state_child_iters(state_root);
//     visualize_graph(state_root);
//
//     reset_all_state_child_iters(state_root);
//     visualize_path(state_root);
// #endif /* VISUALIZE */
//
//     // Clean Up
//     cleanup(state_root);
//
//     return 0;
// }

float evaluate(const state_t* state_current, const state_t* state_new)
{
    // The current player is the player that has done the move.
    const short player_current = state_current->player_current;
    assert((player_current == 1 || player_current == 2) && "Incorrect player");

    // Evaluation of the new piece positions; pov of current player

    const uint32_t pieces_new_player_current = player_current == 1 ? state_new->pieces_1 : state_new->pieces_2;
    const uint32_t pieces_new_player_other = player_current == 1 ? state_new->pieces_2 : state_new->pieces_1;
    const uint32_t pieces_current_player_other = state_new->second_throw && player_current == 1
        ? state_current->pieces_2
        : state_new->second_throw ? state_current->pieces_1
        : player_current == 1     ? state_current->pieces_2
                                  : state_current->pieces_1;

    float points_total = 0.0f;

    for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
    {
        PIECE_FIELD_GET(piece_field, pieces_new_player_current, piece_index);

        // Base points
        points_total += evaluation_base_points[piece_field];
        // Rosette Bonus
        points_total += evaluation_rosette_bonus[piece_field] * (float)EVAL_MULTIPLIER_ROSETTE;

        if (piece_field == FIELD_START || piece_field == 14 || piece_field == 15 || piece_field == FIELD_FINISH)
        {
            // These pieces cannot kill any piece of other player and cannot be killed by other player

            continue;
        }

        // Kill piece of other player
        short count_killable_pieces_of_other_player = 0;
        for (short i = 1; i <= MAX_DICE_THROW; i++)
        {
            if (any_piece_on_field(pieces_new_player_other, piece_field + i, NULL))
            {
                count_killable_pieces_of_other_player += 1;
            }
        }
        points_total += (float)(count_killable_pieces_of_other_player * EVAL_MULTIPLIER_KILLABLE);

        // Killed by other player
        if (6 <= piece_field)
        {
            short count_attacker_pieces_of_other_player = 0;
            for (short i = 1; i <= MAX_DICE_THROW; i++)
            {
                if (any_piece_on_field(pieces_new_player_other, piece_field - i, NULL))
                {
                    count_attacker_pieces_of_other_player += 1;
                }
            }
            points_total += (float)(count_attacker_pieces_of_other_player * EVAL_MULTIPLIER_ATTACKER);
        }
    }

    // Improvement of state
    short count_pieces_other_player_start_current = 0;
    for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
    {
        PIECE_FIELD_GET(piece_field, pieces_current_player_other, i);
        if (piece_field == FIELD_START)
        {
            count_pieces_other_player_start_current += 1;
        }
    }
    short count_pieces_other_player_start_new = 0;
    for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
    {
        PIECE_FIELD_GET(piece_field, pieces_new_player_other, i);
        if (piece_field == FIELD_START)
        {
            count_pieces_other_player_start_new += 1;
        }
    }

    const bool kill_happens = count_pieces_other_player_start_new != count_pieces_other_player_start_current;

    points_total += (float)kill_happens * EVAL_ADDER_KILL_HAPPENS;

    return points_total;
}

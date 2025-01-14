#include "sim.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
        state_new->moved_piece = piece_index;
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

void calculate_all_children_by_piece(state_t* state, const short piece_index, const short* dice_first)
{
    printf("Simulate piece: %hd\n", piece_index);
    if (dice_first == NULL)
    {
        for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
        {
            simulate_wrapper(state, piece_index, dice);
        }
    }
    else
    {
        // Known dice throw
        simulate_wrapper(state, piece_index, *dice_first);
    }
    printf("\n");

    if (state_get_children_count(state) == 0)
    {
        // No move was possible

        state_t* state_new = state_init(state->score_1, state->score_2, state->pieces_1, state->pieces_2,
                                        state->player_current, state->player_other);
        state_swap_player(state_new);

        state_add_child(state, state_new);
    }
}

float minimax(state_t* state_current, const size_t depth, const bool maximize, const short* dice_first,
              const int player_to_maximize)
{
    if (depth == 0)
    {
        state_current->eval = evaluate(state_current->parent, state_current, player_to_maximize);
        return state_current->eval;
    }

    if (maximize)
    {
        for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
        {
            // Calculate all children from piece_index
            const size_t index_child_start = state_current->child_iter_max + 1;
            calculate_all_children_by_piece(state_current, piece_index, dice_first);
            const size_t index_child_end = state_current->child_iter_max + 1;

            // get worst case of children evaluation
            float eval_min = +FLT_MAX;
            for (size_t index_child = index_child_start; index_child < index_child_end; index_child++)
            {
                state_t* child = state_current->children[index_child];
                const bool maximize_next_level = child->player_current == player_to_maximize;

                const float eval = minimax(child, depth - 1, maximize_next_level, NULL, player_to_maximize);

                if (state_current->children[index_child]->dice != 0)
                {
                    // ignore child with dice == 0

                    eval_min = fminf(eval_min, eval);
                }
            }

            if (state_current->child_iter_max == -1 ||
                (state_current->child_iter_max == 0 && state_current->children[0]->dice == 0))
            {
                // Compensation, if no move, or only one move (dice == 0) is possible
                eval_min = -FLT_MAX;
            }

            state_current->eval = state_current->alpha = fmaxf(state_current->alpha, eval_min);
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

        for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
        {
            // Calculate all children from piece_index
            const size_t index_child_start = state_current->child_iter_max + 1;
            calculate_all_children_by_piece(state_current, piece_index, dice_first);
            const size_t index_child_end = state_current->child_iter_max + 1;

            // get worst case of children evaluation
            float eval_min = +FLT_MAX;
            for (size_t index_child = index_child_start; index_child < index_child_end; index_child++)
            {
                state_t* child = state_current->children[index_child];
                const bool maximize_next_level = child->player_current == player_to_maximize;

                const float eval = minimax(child, depth - 1, maximize_next_level, NULL, player_to_maximize);

                if (state_current->children[index_child]->dice != 0)
                {
                    // ignore child with dice == 0

                    eval_min = fminf(eval_min, eval);
                }
            }

            if (state_current->child_iter_max == -1 ||
                (state_current->child_iter_max == 0 && state_current->children[0]->dice == 0))
            {
                // Compensation, if no move, or only one move (dice == 0) is possible
                eval_min = -FLT_MAX;
            }

            state_current->eval = state_current->beta = fminf(state_current->beta, eval_min);
            if (state_current->beta <= state_current->alpha)
            {
                break;
            }
        }
        return state_current->eval;
    }

    assert(false);
}

char get_best_move(state_t* state_root, const short* dice_first, const int player_to_maximize)
{
    const float i = minimax(state_root, STEPS_IN_FUTURE, true, dice_first, player_to_maximize);
    assert(i == state_root->eval);

    char moved_piece = -1;
    for (size_t index_child = 0; index_child <= state_root->child_iter_max; index_child++)
    {
        const state_t* child = state_root->children[index_child];
        if (child->dice == 0)
        {
            continue;
        }

        if (child->eval == state_root->eval)
        {
            moved_piece = (char)child->moved_piece;
            break;
        }
    }

#if VISUALIZE
    reset_all_state_child_iters(state_root);
    visualize_graph(state_root);

    reset_all_state_child_iters(state_root);
    visualize_path(state_root);
#endif /* VISUALIZE */

    cleanup(state_root);

    return moved_piece;
}

float evaluate(const state_t* state_current, const state_t* state_new, const int player_to_maximize)
{
    assert((player_to_maximize == 1 || player_to_maximize == 2) && "Incorrect player");

    if (state_check_win(state_new))
    {
        return +FLT_MAX;
    }

    const uint32_t pieces_new_player_max = player_to_maximize == 1 ? state_new->pieces_1 : state_new->pieces_2;
    const uint32_t pieces_new_player_min = player_to_maximize == 1 ? state_new->pieces_2 : state_new->pieces_1;
    const uint32_t pieces_current_player_min = state_new->second_throw && player_to_maximize == 1
        ? state_current->pieces_2
        : state_new->second_throw ? state_current->pieces_1
        : player_to_maximize == 1 ? state_current->pieces_2
                                  : state_current->pieces_1;

    float points_total = 0.0f;

    for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
    {
        PIECE_FIELD_GET(piece_field, pieces_new_player_max, piece_index);

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
            if (any_piece_on_field(pieces_new_player_min, piece_field + i, NULL))
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
                if (any_piece_on_field(pieces_new_player_min, piece_field - i, NULL))
                {
                    count_attacker_pieces_of_other_player += 1;
                }
            }
            points_total += (float)(count_attacker_pieces_of_other_player * EVAL_MULTIPLIER_ATTACKER);
        }
    }

    if (player_to_maximize == state_current->player_current)
    {
        // Improvement of state
        short count_pieces_other_player_start_current = 0;
        for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
        {
            PIECE_FIELD_GET(piece_field, pieces_current_player_min, i);
            if (piece_field == FIELD_START)
            {
                count_pieces_other_player_start_current += 1;
            }
        }
        short count_pieces_other_player_start_new = 0;
        for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
        {
            PIECE_FIELD_GET(piece_field, pieces_new_player_min, i);
            if (piece_field == FIELD_START)
            {
                count_pieces_other_player_start_new += 1;
            }
        }

        const bool kill_happens = count_pieces_other_player_start_new != count_pieces_other_player_start_current;
        points_total += (float)kill_happens * EVAL_ADDER_KILL_HAPPENS;
    }

    // scale points_total with throw_probabilty to strengthen dice throws with high probability
    // negative points_total will always be < 0
    return points_total;
    // return points_total * throw_probability[state_new->dice];
}

#include "sim.h"

#include <stdio.h>

#include "common.h"
#include "config.h"

#if VISUALIZE
#include "visualize.h"
#endif /* VISUALIZE */


state_t* simulate(const state_t* state_current, const short piece_index, const short dice)
{
    state_t* state_new = NULL;

    if (dice != 0)
    {
        uint32_t pieces_current_player, pieces_other_player;
        if (state_current->player_current == 1)
        {
            pieces_current_player = state_current->pieces_1;
            pieces_other_player = state_current->pieces_2;
        }
        else if (state_current->player_current == 2)
        {
            pieces_current_player = state_current->pieces_2;
            pieces_other_player = state_current->pieces_1;
        }
        else
        {
            assert(false && "Incorrect player");
        }

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

    state_check_win(state_new);

    const float score = evaluate(state_current, state_new);
    state_new->eval = score;
}

void cleanup_child(state_t* state, int index_current_child)
{
    while (index_current_child <= state->child_iter_max && state->children[index_current_child])
    {
        cleanup_child(state->children[index_current_child], 0);
        index_current_child++;
    }

    state_free(state);
}

void cleanup(state_t* state_root) { cleanup_child(state_root, 0); }

void reset_child_iter(state_t* state, int index_current_child)
{
    while (index_current_child <= state->child_iter_max && state->children[index_current_child])
    {
        reset_child_iter(state->children[index_current_child], 0);
        index_current_child++;
    }

    state->child_iter = -1;
}

void reset_all_state_child_iters(state_t* state_root) { reset_child_iter(state_root, 0); }

char get_best_move(state_t* state_root, const short dice_first)
{
    // dice_first == -1 -> all dice throws are calculated on first level

    state_t* state_current = state_root;

    size_t step_current = 0;

    while (state_current)
    {
        for (size_t step = step_current; step < STEPS_IN_FUTURE; step++)
        {
            printf("Simulate step: %lu\n", step);
            for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
            {
                printf("Simulate piece: %hd\n", piece_index);
                if (dice_first != -1 && step_current == 0)
                {
                    // If first dice throw is known, simulate only this.
                    simulate_wrapper(state_current, piece_index, dice_first);
                }
                else
                {
                    // Simulate all possible dice throws
                    for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
                    {
                        simulate_wrapper(state_current, piece_index, dice);
                    }
                }
                printf("\n");
            }


            if (state_get_children_count(state_current) == 0)
            {
                // No move was possible

                state_t* state_new =
                    state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
                               state_current->pieces_2, state_current->player_current, state_current->player_other);
                state_swap_player(state_new);

                state_new->eval = evaluate(state_current, state_new);

                state_add_child(state_current, state_new);
            }

            if (step != STEPS_IN_FUTURE - 1)
            {
                state_current = state_get_next_child(state_current);
            }
        }

        step_current = STEPS_IN_FUTURE - 1;
        state_current = state_get_next_child_of_parent_recursive(state_current, &step_current);
    }

    // TODO: To implement
    //  Return value is the index of the piece (zero-based)


#if VISUALIZE
    reset_all_state_child_iters(state_root);
    visualize_graph(state_root);

    reset_all_state_child_iters(state_root);
    visualize_path(state_root);
#endif /* VISUALIZE */

    // Clean Up
    cleanup(state_root);

    return 0;
}

float evaluate(const state_t* state_current, const state_t* state_new)
{
    short player_current;
    if (state_new->second_throw)
    {
        player_current = state_new->player_current;
    }
    else
    {
        player_current = state_new->player_other;
    }

    uint32_t pieces_current_player, pieces_other_player;
    if (player_current == 1)
    {
        pieces_current_player = state_new->pieces_1;
        pieces_other_player = state_new->pieces_2;
    }
    else if (player_current == 2)
    {
        pieces_current_player = state_new->pieces_2;
        pieces_other_player = state_new->pieces_1;
    }
    else
    {
        assert(false && "Incorrect player");
    }

    float points_total = 0.0f;

    for (short piece_index = 0; piece_index < NUM_OF_PIECES_PER_PLAYER; piece_index++)
    {
        PIECE_FIELD_GET(piece_field, pieces_current_player, piece_index);

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
            if (any_piece_on_field(pieces_other_player, piece_field + i, NULL))
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
                if (any_piece_on_field(pieces_other_player, piece_field - i, NULL))
                {
                    count_attacker_pieces_of_other_player += 1;
                }
            }
            points_total += (float)(count_attacker_pieces_of_other_player * EVAL_MULTIPLIER_ATTACKER);
        }
    }

    // Improvement of state
    uint32_t pieces_other_player_source, pieces_other_player_new;
    if (player_current == 1)
    {
        pieces_other_player_source = state_current->pieces_2;
        pieces_other_player_new = state_new->pieces_2;
    }
    else if (player_current == 2)
    {
        pieces_other_player_source = state_current->pieces_1;
        pieces_other_player_new = state_new->pieces_1;
    }
    else
    {
        assert(false && "Incorrect player");
    }

    short count_pieces_other_player_start_source = 0;
    for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
    {
        PIECE_FIELD_GET(piece_field, pieces_other_player_source, i);
        if (piece_field == FIELD_START)
        {
            count_pieces_other_player_start_source += 1;
        }
    }
    short count_pieces_other_player_start_new = 0;
    for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
    {
        PIECE_FIELD_GET(piece_field, pieces_other_player_new, i);
        if (piece_field == FIELD_START)
        {
            count_pieces_other_player_start_new += 1;
        }
    }

    const bool kill_happens = count_pieces_other_player_start_new != count_pieces_other_player_start_source;

    points_total += (float)kill_happens * EVAL_ADDER_KILL_HAPPENS;

    return points_total;
}

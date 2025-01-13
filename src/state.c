#include "state.h"

#include <stdlib.h>

#include "common.h"
#include "config.h"

size_t id = -1;

state_t* state_init(const short score_1, const short score_2, const uint32_t pieces_1, const uint32_t pieces_2,
                    const short player_current, const short player_other)
{
    state_t* state = malloc(sizeof(state_t));
    if (!state)
    {
        assert(false && "malloc failed");
    }

    state->children = malloc(NUM_OF_PIECES_PER_PLAYER * DICE_RANGE_TRUE * sizeof(state_t*));
    if (!state->children)
    {
        state_free(state);
        assert(false && "malloc failed");
    }

    state->score_1 = score_1;
    state->score_2 = score_2;
    state->player_current = player_current;
    state->player_other = player_other;
    state->pieces_1 = pieces_1;
    state->pieces_2 = pieces_2;

    state->parent = NULL;

    state->second_throw = false;
    state->child_iter = -1;
    state->child_iter_max = -1;

    state->dice = -1;
    state->moved_piece = -1;

    state->id = ++id;

    return state;
}

void state_free(state_t* state)
{
    if (state)
    {
        if (state->children)
        {
            free(state->children);
        }
        free(state);
    }
}

bool state_check_win(const state_t* state)
{
    assert(state != NULL && "state is null");

    bool won = false;
    if (state->player_current == 1)
    {
        won = state->score_1 == NUM_OF_PIECES_PER_PLAYER;
    }
    else if (state->player_current == 2)
    {
        won = state->score_2 == NUM_OF_PIECES_PER_PLAYER;
    }
    return won;
}

void state_swap_player(state_t* state)
{
    assert(state != NULL && "state is null");

    const short tmp = state->player_current;
    state->player_current = state->player_other;
    state->player_other = tmp;
}

void state_piece_move(state_t* state, const short player, const size_t piece_index, const uint8_t dest)
{
    assert(state != NULL && "state is null");

    uint32_t* pieces;
    short* score;

    if (player == 1)
    {
        pieces = &state->pieces_1;
        score = &state->score_1;
    }
    else if (player == 2)
    {
        pieces = &state->pieces_2;
        score = &state->score_2;
    }
    else
    {
        assert(false && "Incorrect player");
    }

    GET_PIECE_MASK(piece_mask_clear, piece_index);
    piece_mask_clear = ~piece_mask_clear;

    *pieces &= piece_mask_clear;
    *pieces |= PIECE_FIELD_SET(piece_index, dest);

    if (dest == FIELD_FINISH)
    {
        /* Player has finished */
        *score += 1;
    }
}

void state_add_child(state_t* state, state_t* child)
{
    assert(state != NULL && "state is null");
    assert(child != NULL && "child is null");

    state->child_iter_max++;
    state->children[state->child_iter_max] = child;

    child->parent = state;
}

state_t* state_get_parent(const state_t* state)
{
    assert(state != NULL && "state is null");

    return state->parent;
}

state_t* state_get_next_child(state_t* state)
{
    assert(state != NULL && "state is null");

    state->child_iter++;

    state_t* child;
    if (state->child_iter <= state->child_iter_max)
    {
        child = state->children[state->child_iter];
    }
    else
    {
        child = NULL;
    }


    return child;
}

size_t state_get_children_count(const state_t* state)
{
    assert(state != NULL && "state is null");

    return state->child_iter_max + 1;
}

bool state_has_next_child(const state_t* state)
{
    assert(state != NULL && "state is null");

    return state->child_iter_max >= 0 && state->child_iter < state->child_iter_max;
}

bool state_equals(const state_t* state1, const state_t* state2)
{
    return state1->score_1 == state2->score_1 && state1->score_2 == state2->score_2 &&
        state1->pieces_1 == state2->pieces_1 && state1->pieces_2 == state2->pieces_2 &&
        state1->player_current == state2->player_current && state1->player_other == state2->player_other &&
        state1->dice == state2->dice && state1->moved_piece == state2->moved_piece &&
        state1->second_throw == state2->second_throw && state1->parent == state1->parent &&
        state1->children == state1->children && state1->eval == state1->eval &&
        state1->child_iter == state1->child_iter && state1->child_iter_max == state1->child_iter_max;
}

state_t* state_get_next_child_of_parent_recursive(state_t* state, size_t* step_count)
{
    size_t tmp;
    if (!step_count)
    {
        step_count = &tmp;
    }

    // Determine parent state, which has another child
    do
    {
        state = state_get_parent(state);
        (*step_count)--;
    }
    while (state && !state_has_next_child(state));
    if (state)
    {
        // Get the new child from the parent
        state = state_get_next_child(state);
        (*step_count)++;
    }
    else
    {
        // Every node has been visualize => state_current is now the parent of state_root
        state = NULL;
    }

    return state;
}

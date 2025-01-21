#include "state.h"

#include <assert.h>
#include <float.h>
#include <stdlib.h>

#include "common.h"

size_t id = -1;

state_t* state_init(const short score_0, const short score_1, const uint32_t pieces_0, const uint32_t pieces_1,
                    const short player_current, const short player_other, const minimax_config_t* config)
{
    state_t* state = malloc(sizeof(state_t));
    if (!state)
    {
        assert(false && "malloc failed");
    }

    state->children = calloc(config->num_of_pieces_per_player * DICE_RANGE_TRUE, sizeof(state_t*));
    if (!state->children)
    {
        state_free(state);
        assert(false && "calloc failed");
    }

    state->score_0 = score_0;
    state->score_1 = score_1;
    state->player_current = player_current;
    state->player_other = player_other;
    state->pieces_0 = pieces_0;
    state->pieces_1 = pieces_1;

    state->parent = NULL;

    state->second_throw = false;
    state->child_iter = -1;
    state->child_iter_max = -1;

    state->dice = -1;
    state->moved_piece = -1;

    state->id = ++id;

    state->alpha = -FLT_MAX;
    state->beta = +FLT_MAX;

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

void state_reset_ids() { id = -1; }

bool state_check_win(const state_t* state, const minimax_config_t* config)
{
    assert(state != NULL && "state is null");
    assert(state->player_current == 0 || state->player_current == 1 && "Player is incorrect");

    return (state->player_current == 0 ? state->score_0 : state->score_1) == config->num_of_pieces_per_player;
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
    assert(player == 0 || player == 1 && "player is incorrect");

    uint32_t* pieces = player == 0 ? &state->pieces_0 : &state->pieces_1;
    short* score = player == 0 ? &state->score_0 : &state->score_1;

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
    return state1->score_0 == state2->score_0 && state1->score_1 == state2->score_1 &&
        state1->pieces_0 == state2->pieces_0 && state1->pieces_1 == state2->pieces_1 &&
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

void state_reset_child_iter(state_t* state) { state->child_iter = -1; }

void state_iterate_over_all_children_and_execute(state_t* state, size_t index_current_child, void (*func)(state_t*))
{
    while (index_current_child <= state->child_iter_max && state->children[index_current_child])
    {
        state_iterate_over_all_children_and_execute(state->children[index_current_child], 0, func);
        index_current_child++;
    }

    func(state);
}

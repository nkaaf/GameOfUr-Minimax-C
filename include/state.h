#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct state_s state_t;

struct state_s
{
    short score_1, score_2;
    uint32_t pieces_1, pieces_2;
    short player_current, player_other;
    short dice;
    short moved_piece;
    bool second_throw;
    state_t* parent;
    state_t** children;
    float eval, alpha, beta;
    int child_iter, child_iter_max;
    size_t id;
};

state_t* state_init(short score_1, short score_2, uint32_t pieces_1, uint32_t pieces_2, short player_current,
                    short player_other);

void state_free(state_t* state);

void state_reset_ids();

bool state_check_win(const state_t* state);

void state_swap_player(state_t* state);

void state_piece_move(state_t* state, short player, size_t piece_index, uint8_t dest);

void state_add_child(state_t* state, state_t* child);

state_t* state_get_parent(const state_t* state);

state_t* state_get_next_child(state_t* state);

size_t state_get_children_count(const state_t* state);

bool state_has_next_child(const state_t* state);

bool state_equals(const state_t* state1, const state_t* state2);

state_t* state_get_next_child_of_parent_recursive(state_t* state, size_t* step_count);

void state_reset_child_iter(state_t* state);

void state_iterate_over_all_children_and_execute(state_t* state, size_t index_current_child, void (*func)(state_t*));

#endif // STATE_H

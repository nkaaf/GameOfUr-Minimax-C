#include <stddef.h>

#include "common.h"
#include "config.h"
#include "sim.h"
#include "state.h"

state_t* set_up_state(void)
{
    const short score_default = 0;
    const uint32_t pieces_default =
        (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |
         PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START));

    const short player_current_default = 1, player_other_default = 2;

    return state_init(score_default, score_default, pieces_default, pieces_default, player_current_default,
                      player_other_default);
}

void test0()
{
    // No movement

    const short piece_index = 0, dice = 0;

    const state_t* state_current = set_up_state();

    state_t* state_expected = set_up_state();
    state_swap_player(state_expected);
    state_expected->dice = dice;

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_equals(state_new, state_expected));
}

void test1()
{
    // Piece is already in finish, no movement => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, FIELD_FINISH);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_new == NULL);
}

void test2()
{
    // Piece cannot finish, dice is too high => No new state

    const short piece_index = 0, dice = 3;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 14);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_new == NULL);
}

void test3()
{
    // Piece finishes

    const short piece_index = 0, dice = 2;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 14);

    state_t* state_expected = set_up_state();
    state_expected->score_1 = 1;
    state_expected->pieces_1 &= piece_mask_clear;
    state_expected->pieces_1 |= PIECE_FIELD_SET(piece_index, FIELD_FINISH);
    state_expected->dice = dice;
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_equals(state_expected, state_new));
}

void test4()
{
    // Piece cannot move, because same player is on this field => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index + 1);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index + 1, 2);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_new == NULL);
}

#if ROSETTE_9_IS_SAFE
void test5()
{
    // Piece cannot be moved, because other player is on safe spot => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 8);
    state_current->pieces_2 &= piece_mask_clear;
    state_current->pieces_2 |= PIECE_FIELD_SET(piece_index, 9);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_new == NULL);
}
#endif /* ROSETTE_9_IS_SAFE */

void test6()
{
    // Normal move

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 6);

    state_t* state_expected = set_up_state();
    state_expected->pieces_1 &= piece_mask_clear;
    state_expected->pieces_1 |= PIECE_FIELD_SET(piece_index, 7);
    state_expected->dice = dice;
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_equals(state_expected, state_new));
}

void test7()
{
    // Catch other player
    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 7);
    state_current->pieces_2 &= piece_mask_clear;
    state_current->pieces_2 |= PIECE_FIELD_SET(piece_index, 8);

    state_t* state_expected = set_up_state();
    state_expected->pieces_1 &= piece_mask_clear;
    state_expected->pieces_1 |= PIECE_FIELD_SET(piece_index, 8);
    state_expected->pieces_2 &= piece_mask_clear;
    state_expected->pieces_2 |= PIECE_FIELD_SET(piece_index, FIELD_START);
    state_expected->dice = dice;
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_equals(state_expected, state_new));
}

void test8()
{
    // Move on Rosette Field
    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 4);

    state_t* state_expected = set_up_state();
    state_expected->pieces_1 &= piece_mask_clear;
    state_expected->pieces_1 |= PIECE_FIELD_SET(piece_index, 5);
    state_expected->dice = dice;
    state_expected->moved_piece = piece_index;
    state_expected->second_throw = true;

    const state_t* state_new = simulate(state_current, piece_index, dice);

    assert(state_equals(state_expected, state_new));
}

void test9()
{
    // Arithmetic Test

    state_t* state = state_init(0, 0, 0xA4FDA, 0x0, 1, 2);

    state_piece_move(state, 1, 0, 0x9);
    assert(state->pieces_1 == 0xA4FD9);
    state_piece_move(state, 1, 1, 0x9);
    assert(state->pieces_1 == 0xA4F99);
    state_piece_move(state, 1, 2, 0x9);
    assert(state->pieces_1 == 0xA4999);
    state_piece_move(state, 1, 3, 0x9);
    assert(state->pieces_1 == 0xA9999);
    state_piece_move(state, 1, 4, 0x9);
    assert(state->pieces_1 == 0x99999);
}

int main()
{
    test0();
    test1();
    test2();
    test3();
    test4();
#if ROSETTE_9_IS_SAFE
    test5();
#endif /* ROSETTE_9_IS_SAFE */
    test6();
    test7();
    test8();
    test9();
}

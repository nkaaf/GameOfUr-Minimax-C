#include <assert.h>
#include <stddef.h>

#include "common.h"
#include "config.h"
#include "sim.h"
#include "state.h"

static float points_base[] = {100, -.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static float multiplier_kill_distance[] = {-1, 1.0f / 4.0f, 3.0f / 8.0f, 1.0f / 4.0f, 1.0f / 16.0f};

const static minimax_config_t config = {
    .depth = 4,
    .rosette_middle_safe = true,
    .pieces_player_0 =
        (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |
         PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
         PIECE_FIELD_SET(6, FIELD_START)),
    .pieces_player_1 =
        (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |
         PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START) | PIECE_FIELD_SET(5, FIELD_START) |
         PIECE_FIELD_SET(6, FIELD_START)),
    .score_player_0 = 0,
    .score_player_1 = 0,
    .num_of_pieces_per_player = 7,
    .player_0_maximize = true,
    .alpha_beta_pruning_enable = true,
    .eval_config = {.points_base = points_base,
                    .adder_rosette = 30,
                    .multiplier_killable = 10,
                    .multiplier_attacker = -1.5,
                    .multiplier_kill_distance = multiplier_kill_distance,
                    .adder_kill_happens = 100,
                    .strategy_evaluation_propagation = MINIMAX_GAMEOFUR_STRATEGY_EXPECTED_VALUE},
    .visualize_config = {.enable = true, .depth = 7, .graph_path = "Graph.gv"}};

state_t* set_up_state(void)
{
    const short player_current_default = 0, player_other_default = 1;

    return state_init(config.score_player_0, config.score_player_1, config.pieces_player_0, config.pieces_player_1,
                      player_current_default, player_other_default, &config);
}

void test0()
{
    // No movement

    const short piece_index = 0, dice = 0;

    const state_t* state_current = set_up_state();

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(!state_new);
}

void test1()
{
    // Piece is already in finish, no movement => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, FIELD_FINISH);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(!state_new);
}

void test2()
{
    // Piece cannot finish, dice is too high => No new state

    const short piece_index = 0, dice = 3;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 14);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(!state_new);
}

void test3()
{
    // Piece finishes

    const short piece_index = 0, dice = 2;
    const short dices[] = {dice};

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 14);

    state_t* state_expected = set_up_state();
    state_expected->score_0 = 1;
    state_expected->pieces_0 &= piece_mask_clear;
    state_expected->pieces_0 |= PIECE_FIELD_SET(piece_index, FIELD_FINISH);
    state_dices_setter(state_expected, dices, 1);
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(state_equals(state_expected, state_new));
}

void test4()
{
    // Piece cannot move, because same player is on this field => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index + 1);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index + 1, 2);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(!state_new);
}

void test5()
{
    // Piece cannot be moved, because other player is on safe spot => No new state

    const short piece_index = 0, dice = 1;

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 8);
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 9);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(!state_new);
}

void test6()
{
    // Normal move

    const short piece_index = 0, dice = 1;
    const short dices[] = {dice};

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 6);

    state_t* state_expected = set_up_state();
    state_expected->pieces_0 &= piece_mask_clear;
    state_expected->pieces_0 |= PIECE_FIELD_SET(piece_index, 7);
    state_dices_setter(state_expected, dices, 1);
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(state_equals(state_expected, state_new));
}

void test7()
{
    // Catch other player
    const short piece_index = 0, dice = 1;
    const short dices[] = {dice};

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 7);
    state_current->pieces_1 &= piece_mask_clear;
    state_current->pieces_1 |= PIECE_FIELD_SET(piece_index, 8);

    state_t* state_expected = set_up_state();
    state_expected->pieces_0 &= piece_mask_clear;
    state_expected->pieces_0 |= PIECE_FIELD_SET(piece_index, 8);
    state_expected->pieces_1 &= piece_mask_clear;
    state_expected->pieces_1 |= PIECE_FIELD_SET(piece_index, FIELD_START);
    state_dices_setter(state_expected, dices, 1);
    state_expected->moved_piece = piece_index;
    state_swap_player(state_expected);

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(state_equals(state_expected, state_new));
}

void test8()
{
    // Move on Rosette Field
    const short piece_index = 0, dice = 1;
    const short dices[] = {dice};

    state_t* state_current = set_up_state();
    GET_PIECE_MASK(piece_mask, piece_index);
    const uint32_t piece_mask_clear = ~piece_mask;
    state_current->pieces_0 &= piece_mask_clear;
    state_current->pieces_0 |= PIECE_FIELD_SET(piece_index, 4);

    state_t* state_expected = set_up_state();
    state_expected->pieces_0 &= piece_mask_clear;
    state_expected->pieces_0 |= PIECE_FIELD_SET(piece_index, 5);
    state_dices_setter(state_expected, dices, 1);
    state_expected->moved_piece = piece_index;
    state_expected->second_throw = true;

    const state_t* state_new = simulate(state_current, piece_index, dice, &config);

    assert(state_equals(state_expected, state_new));
}

void test9()
{
    // Arithmetic Test

    state_t* state = state_init(0, 0, 0xA4FDA, 0x0, 1, 2, &config);

    state_piece_move(state, 0, 0, 0x9);
    assert(state->pieces_0 == 0xA4FD9);
    state_piece_move(state, 0, 1, 0x9);
    assert(state->pieces_0 == 0xA4F99);
    state_piece_move(state, 0, 2, 0x9);
    assert(state->pieces_0 == 0xA4999);
    state_piece_move(state, 0, 3, 0x9);
    assert(state->pieces_0 == 0xA9999);
    state_piece_move(state, 0, 4, 0x9);
    assert(state->pieces_0 == 0x99999);
}

int main()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
}

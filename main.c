#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "sim.h"
#include "state.h"
#include "visualize.h"

size_t count = 0;
int main(void)
{
    state_t** state_list = calloc(1000000, sizeof(state_t*));
    assert(state_list != NULL);

    state_t* state_root = state_init(SCORE_1_START, SCORE_2_START, PIECES_1_START, PIECES_2_START, PLAYER_CURRENT_START,
                                     PLAYER_OTHER_START);

    state_list[state_root->id] = state_root;
    count++;

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
                for (short dice = MIN_DICE_THROW; dice <= MAX_DICE_THROW; dice++)
                {
                    printf("Simulate dice: %hd\n", dice);

                    state_t* state_new = simulate(state_current, piece_index, dice);
                    if (!state_new)
                    {
                        // Movement is not possible
                        continue;
                    }

                    printf("Simulated id: %lu\n", state_new->id);

                    state_list[state_new->id] = state_new;
                    count++;

                    state_add_child(state_current, state_new);

                    state_check_win(state_new);

                    const float score = evaluate(state_current, state_new);
                    state_new->eval = score;
                }
                printf("\n");
            }


            if (state_get_children_count(state_current) == 0)
            {
                // No move was possible

                // TODO: What evaluation score should this move have
                state_t* state_new =
                    state_init(state_current->score_1, state_current->score_2, state_current->pieces_1,
                               state_current->pieces_2, state_current->player_current, state_current->player_other);
                state_swap_player(state_new);

                state_add_child(state_current, state_new);
            }

            if (step != STEPS_IN_FUTURE - 1)
            {
                state_current = state_get_next_child(state_current);
            }
        }

        step_current = STEPS_IN_FUTURE;

        const state_t* state_parent = state_get_parent(state_current);
        step_current -= 1;
        while (state_parent && !state_has_next_child(state_parent))
        {
            step_current -= 1;
            state_parent = state_get_parent(state_parent);
        }
        if (!state_parent)
        {
            // state_parent is root
            // Simulation should end
            state_current = NULL;
        }
        else
        {
            state_current = state_get_next_child((state_t*)state_parent);
        }
    }

    printf("Count: %lu\n", count);

    if (VISUALIZE)
    {
        visualize_init("/home/niklas/Schreibtisch/GameOfUr-C/Graph.gv");

        for (size_t id = 0; id < count; id++)
        {
            const state_t* state = state_list[id];
            assert(state && "state is NULL");

            short player_current;
            if (id == 0)
            {
                player_current = 1;
            }
            else if (state->second_throw)
            {
                player_current = state->player_current;
            }
            else
            {
                player_current = state->player_other;
            }

            visualize_add_node(state->id, state->eval, state->dice, state->moved_piece, player_current);
            if (state->child_iter_max == -1)
            {
                continue;
            }

            for (size_t iter = 0; iter <= state->child_iter_max; iter++)
            {
                const state_t* child = state->children[iter];
                assert(child && "child is NULL");

                visualize_add_edge(state->id, child->id);
            }
        }

        visualize_finalize();
        visualize_free();
    }

    for (size_t id = 0; id < count; id++)
    {
        state_t* state = state_list[id];
        state_free(state);
    }

    free(state_list);

    return 0;
}

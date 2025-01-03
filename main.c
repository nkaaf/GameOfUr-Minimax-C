#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "sim.h"
#include "state.h"

#if VISUALIZE
#include "visualize.h"
#endif /* VISUALIZE */


size_t count = 0;
int main(void)
{
#if VISUALIZE
    int max_count_states = (int)pow(NUM_OF_PIECES_PER_PLAYER * DICE_RANGE_TRUE, STEPS_IN_FUTURE);
    state_t* state_list[max_count_states];
#endif /* VISUALIZE */

    state_t* state_root = state_init(SCORE_1_START, SCORE_2_START, PIECES_1_START, PIECES_2_START, PLAYER_CURRENT_START,
                                     PLAYER_OTHER_START);

#if VISUALIZE
    state_list[state_root->id] = state_root;
#endif /* VISUALIZE */
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

#if VISUALIZE
                    state_list[state_new->id] = state_new;
#endif /* VISUALIZE */
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

#if VISUALIZE
    {
        visualize_init("Graph.gv");

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

        visualize_init("Graph_Path.gv");

        size_t state_count_to_visualize = 0;
        const state_t* states_to_visualize[count];
        states_to_visualize[state_count_to_visualize++] = state_root;

        size_t current_nodes_size = 0;
        const state_t** current_nodes = calloc(current_nodes_size, sizeof(state_t*));
        if (!current_nodes)
        {
            assert(false && "calloc failed");
        }
        current_nodes[current_nodes_size++] = state_root;
        for (size_t steps_to_visualize = 0; steps_to_visualize < VISUALIZE_THROWS_COUNT; steps_to_visualize++)
        {

            size_t new_nodes_size = 0;
            const state_t** new_nodes =
                calloc(current_nodes_size * DICE_RANGE_TRUE * NUM_OF_PIECES_PER_PLAYER, sizeof(state_t*));

            const short dice = visualize_throws[steps_to_visualize];
            for (size_t i = 0; i < current_nodes_size; i++)
            {
                const state_t* current_node = current_nodes[i];
                if (current_node->child_iter_max == -1)
                {
                    continue;
                }
                const state_t** children = (const state_t**)current_node->children;

                size_t children_with_throws_count = 0;
                const state_t** children_with_throws = calloc(current_node->child_iter_max + 1, sizeof(state_t*));
                if (!children_with_throws)
                {
                    assert(false && "calloc failed");
                }
                for (size_t child_iter = 0; child_iter <= current_node->child_iter_max; child_iter++)
                {
                    const state_t* child = children[child_iter];
                    if (child->dice == dice)
                    {
                        children_with_throws[children_with_throws_count++] = child;
                    }
                }

                for (size_t child_with_throw_iter = 0; child_with_throw_iter < children_with_throws_count;
                     child_with_throw_iter++)
                {
                    const state_t* child_with_throw = children_with_throws[child_with_throw_iter];
                    states_to_visualize[state_count_to_visualize++] = child_with_throw;
                    new_nodes[new_nodes_size++] = child_with_throw;
                }
                free(children_with_throws);
            }

            free(current_nodes);

            current_nodes_size = 0;
            current_nodes = calloc(new_nodes_size, sizeof(state_t*));
            for (size_t i = 0; i < new_nodes_size; i++)
            {
                current_nodes[current_nodes_size++] = new_nodes[i];
            }

            free(new_nodes);
        }
        free(current_nodes);

        for (size_t state_to_visualize_iter = 0; state_to_visualize_iter < state_count_to_visualize;
             state_to_visualize_iter++)
        {
            const state_t* state = states_to_visualize[state_to_visualize_iter];
            short player_current;
            if (state->id == 0)
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

            for (size_t child_iter = 0; child_iter <= state->child_iter_max; child_iter++)
            {
                const state_t* child = state->children[child_iter];

                bool should_visualize_child = false;
                for (size_t state_to_visualize_iter2 = 0; state_to_visualize_iter2 < state_count_to_visualize;
                     state_to_visualize_iter2++)
                {
                    const state_t* state_check = states_to_visualize[state_to_visualize_iter2];
                    if (state_check->id == child->id)
                    {
                        should_visualize_child = true;
                        break;
                    }
                }

                if (should_visualize_child)
                {
                    visualize_add_edge(state->id, child->id);
                }
            }
        }

        visualize_finalize();
        visualize_free();
    }

    // Clean Up
    for (size_t id = 0; id < count; id++)
    {
        state_t* state = state_list[id];
        state_free(state);
    }
#endif /* VISUALIZE */

    return 0;
}

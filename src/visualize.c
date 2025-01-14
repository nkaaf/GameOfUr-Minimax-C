#include "visualize.h"

#include "config.h"
#include "graphviz.h"

#include <assert.h>

void visualize_graph(state_t* state_root)
{
    visualize_init(VISUALIZE_GRAPH_FILE_PATH);

    state_t* state_current = state_root;

    while (state_current)
    {
        const short player_current = state_current->player_current;

        visualize_add_node(state_current->id, state_current->eval, state_current->alpha, state_current->beta,
                           state_current->dice, state_current->moved_piece, player_current);

        if (state_current->child_iter_max != -1)
        {
            // Node has children

            for (size_t iter = 0; iter <= state_current->child_iter_max; iter++)
            {
                const state_t* child = state_current->children[iter];
                assert(child && "child is NULL");

                visualize_add_edge(state_current->id, child->id);
            }
        }

        if (state_has_next_child(state_current))
        {
            // Current state has another child
            state_current = state_get_next_child(state_current);
        }
        else
        {
            state_current = state_get_next_child_of_parent_recursive(state_current, NULL);
        }
    }

    visualize_finalize();
    visualize_free();
}

void visualize_path(state_t* state_root)
{
    visualize_init(VISUALIZE_GRAPH_PATH_FILE_PATH);

    state_t* state_current = state_root;

    size_t step_current = -1;
    while (state_current)
    {
        if (step_current < VISUALIZE_THROWS_COUNT && state_current->dice == visualize_throws[step_current])
        {
            const short player_current = state_current->player_current;

            visualize_add_node(state_current->id, state_current->eval, state_current->alpha, state_current->beta,
                               state_current->dice, state_current->moved_piece, player_current);

            if (state_get_parent(state_current))
            {
                visualize_add_edge(state_get_parent(state_current)->id, state_current->id);
            }

            if (state_has_next_child(state_current))
            {
                // Current state has another child
                state_current = state_get_next_child(state_current);
                step_current++;
            }
            else
            {
                state_current = state_get_next_child_of_parent_recursive(state_current, &step_current);
            }
        }
        else
        {
            state_current = state_get_next_child_of_parent_recursive(state_current, &step_current);
        }
    }

    visualize_finalize();
    visualize_free();
}

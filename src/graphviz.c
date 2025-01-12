#include "visualize.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

FILE* file;

void visualize_init(const char* filename)
{
    file = fopen(filename, "w");
    if (!file)
    {
        assert(false && "fopen failed");
    }

    fputs("graph Graph_Minimax {\n", file);
}

void visualize_free()
{
    if (file)
    {
        fclose(file);
    }
}

void visualize_add_node(const size_t id, const float eval, const short dice, const short moved_piece,
                        const short player_current)
{
    assert(file && "file is NULL");

    char* color;
    if (player_current == 1)
    {
        color = "green";
    }
    else if (player_current == 2)
    {
        color = "red";
    }
    else
    {
        assert(false && "player_current is invalid");
    }

    fprintf(file, "\t%lu [label=\"ID: %lu\nE: %f\nD: %d\nMP: %d\" color=%s]\n", id, id, eval, dice, moved_piece, color);
}

void visualize_add_edge(const size_t id_start, const size_t id_end)
{
    fprintf(file, "\t%lu -- %lu\n", id_start, id_end);
}

void visualize_finalize()
{
    assert(file && "file is NULL");

    fputs("}\n", file);
}

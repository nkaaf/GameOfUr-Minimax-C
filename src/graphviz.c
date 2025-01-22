#include "visualize.h"

#include <assert.h>
#include <common.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

bool determine_float_str(const float val, char** str)
{
    bool dyn_alloc = false;
    if (val == -FLT_MAX)
    {
        *str = "-∞";
    }
    else if (val == +FLT_MAX)
    {
        *str = "+∞";
    }
    else
    {
        const int size = snprintf(NULL, 0, "%.3f", val);
        *str = malloc((size + 1) * sizeof(char));
        snprintf(*str, size + 1, "%.3f", val);
        dyn_alloc = true;
    }

    return dyn_alloc;
}

void visualize_add_node(const size_t id, const float eval, const float alpha, const float beta,
                        const short player_current)
{
    assert(file && "file is NULL");
    assert(player_current == 0 || player_current == 1 && "Invalid player");

    const char* color = player_current == 0 ? "green" : "red";

    char *alpha_val, *beta_val, *eval_val;
    const bool alpha_dyn_alloc = determine_float_str(alpha, &alpha_val);
    const bool beta_dyn_alloc = determine_float_str(beta, &beta_val);
    const bool eval_dyn_alloc = determine_float_str(eval, &eval_val);

    fprintf(file, "\t%lu [label=\"ID: %lu\nE: %s\nα: %s\nβ: %s\n\" color=%s]\n", id, id, eval_val, alpha_val, beta_val,
            color);

    if (alpha_dyn_alloc)
    {
        free(alpha_val);
    }
    if (beta_dyn_alloc)
    {
        free(beta_val);
    }
    if (eval_dyn_alloc)
    {
        free(eval_val);
    }
}

void visualize_add_edge(const size_t id_start, const size_t id_end, const short* dices, const size_t dices_len,
                        const short moved_piece)
{
    assert(MAX_DICE_THROW <= 9 && "Implementation can only allow dices below 10");

    const size_t string_len =
        (dices_len - 1) * dices_len + dices_len + 1; // count of ',' + count of dices + Null-Terminator
    char* dices_str = calloc(string_len, sizeof(char));
    assert(dices_str && "malloc failed");

    sprintf(dices_str, "%d", dices[0]);
    for (size_t i = 1; i < dices_len; i++)
    {
        sprintf(dices_str, "%s,%d", dices_str, dices[i]);
    }
    dices_str[string_len -1] = '\0';
    fprintf(file, "\t%lu -- %lu\n [label=\"D: %s\nMP: %d\"]\n", id_start, id_end, dices_str, moved_piece);
    free(dices_str);
}

void visualize_finalize()
{
    assert(file && "file is NULL");

    fputs("}\n", file);
}

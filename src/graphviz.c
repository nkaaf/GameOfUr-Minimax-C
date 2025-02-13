#include <assert.h>
#include <common.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

FILE *file;

void visualize_init(const char *filename) {
  file = fopen(filename, "w");
  if (!file) {
    assert(false && "fopen failed");
  }

  fputs("graph Graph_Minimax {\n", file);
}

void visualize_free() {
  if (file) {
    fclose(file);
  }
}

bool determine_float_str(const float val, char **str) {
  bool dyn_alloc = false;
  if (val == -FLT_MAX) {
    *str = "-∞";
  } else if (val == +FLT_MAX) {
    *str = "+∞";
  } else {
    const int size = snprintf(NULL, 0, "%.3f", val);
    *str = malloc((size + 1) * sizeof(char));
    snprintf(*str, size + 1, "%.3f", val);
    dyn_alloc = true;
  }

  return dyn_alloc;
}

void _visualize_add_node(const size_t id, const float eval, const float *alpha,
                         const float *beta, const short player_current,
                         const short player_to_maximize) {
  assert(file && "file is NULL");
  assert(player_current == 0 || player_current == 1 && "Invalid player");
  assert(player_to_maximize == 0 ||
         player_to_maximize == 1 && "Invalid player_to_maximize");

  const char *color = player_current == player_to_maximize ? "green" : "red";

  char *alpha_val, *beta_val, *eval_val;
  const bool eval_dyn_alloc = determine_float_str(eval, &eval_val);

  if (alpha && beta) {
    const bool alpha_dyn_alloc = determine_float_str(*alpha, &alpha_val);
    const bool beta_dyn_alloc = determine_float_str(*beta, &beta_val);

    fprintf(file, "\t%lu [label=\"ID: %lu\nE: %s\nα: %s\nβ: %s\n\" color=%s]\n",
            id, id, eval_val, alpha_val, beta_val, color);

    if (alpha_dyn_alloc) {
      free(alpha_val);
    }
    if (beta_dyn_alloc) {
      free(beta_val);
    }
  } else {
    fprintf(file, "\t%lu [label=\"ID: %lu\nE: %s\n\" color=%s]\n", id, id,
            eval_val, color);
  }

  if (eval_dyn_alloc) {
    free(eval_val);
  }
}

void visualize_add_node_alpha_beta(const size_t id, const float eval,
                                   const float alpha, const float beta,
                                   const short player_current,
                                   const short player_to_maximize) {
  _visualize_add_node(id, eval, &alpha, &beta, player_current,
                      player_to_maximize);
}

void visualize_add_node(const size_t id, const float eval,
                        const short player_current,
                        const short player_to_maximize) {
  _visualize_add_node(id, eval, NULL, NULL, player_current, player_to_maximize);
}

void visualize_add_edge(const size_t id_start, const size_t id_end,
                        const short dice, const short moved_piece) {
  fprintf(file, "\t%lu -- %lu\n [label=\"D: %d\nMP: %d\"]\n", id_start, id_end,
          dice, moved_piece);
}

void visualize_finalize() {
  assert(file && "file is NULL");

  fputs("}\n", file);
}

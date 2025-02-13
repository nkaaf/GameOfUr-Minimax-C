#ifndef MINIMAX_GRAPHVIZ_H
#define MINIMAX_GRAPHVIZ_H

#include <stddef.h>

void visualize_init(const char *filename);

void visualize_free();

void visualize_add_node_alpha_beta(size_t id, float eval, float alpha,
                                   float beta, short player_current,
                                   short player_to_maximize);

void visualize_add_node(size_t id, float eval, short player_current,
                        short player_to_maximize);

void visualize_add_edge(size_t id_start, size_t id_end, short dice,
                        short moved_piece);

void visualize_finalize();

#endif // MINIMAX_GRAPHVIZ_H

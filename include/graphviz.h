#ifndef GRAPHVIZ_H
#define GRAPHVIZ_H

#include <stddef.h>

void visualize_init(const char* filename);

void visualize_free();

void visualize_add_node(size_t id, float eval, float alpha, float beta, short player_current);

void visualize_add_edge(size_t id_start, size_t id_end, short dice, short moved_piece);

void visualize_finalize();

#endif // GRAPHVIZ_H

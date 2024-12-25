#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <stddef.h>

void visualize_init(const char *filename);

void visualize_free();

void visualize_add_node(size_t id, float eval, short dice, short moved_piece, short player_current);

void visualize_add_edge(size_t id_start, size_t id_end);

void visualize_finalize();

#endif //VISUALIZE_H

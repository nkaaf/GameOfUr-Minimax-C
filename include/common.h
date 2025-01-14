#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FIELD_FINISH ((uint8_t)0)
#define FIELD_START ((uint8_t)1)
#define ROSETTE_SAFE (9)
#define MIN_DICE_THROW (3)
#define MAX_DICE_THROW (4)
#define DICE_RANGE_TRUE (MAX_DICE_THROW - MIN_DICE_THROW + 1)

#define MASK_PIECE_0 ((uint32_t)0xF)
#define MASK_PIECE(n) ((uint32_t) (MASK_PIECE_0 << ((n) * 4)))

#define GET_PIECE_MASK(var_piece_mask, piece_index) uint32_t (var_piece_mask) = MASK_PIECE((piece_index))

#define PIECE_FIELD_SET(piece_index, pos) (((uint32_t)pos) << (4 * (piece_index)))
#define PIECE_FIELD_GET(var_piece_field, pieces, piece_index)                                                          \
    uint8_t(var_piece_field);                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        GET_PIECE_MASK(_piece_mask, (piece_index));                                                                    \
        (var_piece_field) = ((pieces) & _piece_mask) >> (4 * (piece_index));                                           \
    }                                                                                                                  \
    while (0)

#define PIECE_CANNOT_FINISH(field_next) ((field_next) > 16)
#define PIECE_CAN_FINISH(field_next) ((field_next) == 16)

bool any_piece_on_field(uint32_t pieces, uint8_t pos, size_t* piece_index);

#endif // COMMON_H

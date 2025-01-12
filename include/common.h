#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FIELD_FINISH ((uint8_t)0)
#define FIELD_START ((uint8_t)1)
#define ROSETTE_SAFE (9)
#define MIN_DICE_THROW (0)
#define MAX_DICE_THROW (1)
#define DICE_RANGE_TRUE (MAX_DICE_THROW - MIN_DICE_THROW + 1)

#define MASK_PIECE_0 ((uint32_t)0xF)
#define MASK_PIECE_1 ((uint32_t)(MASK_PIECE_0 << 4))
#define MASK_PIECE_2 ((uint32_t)(MASK_PIECE_1 << 4))
#define MASK_PIECE_3 ((uint32_t)(MASK_PIECE_2 << 4))
#define MASK_PIECE_4 ((uint32_t)(MASK_PIECE_3 << 4))
#define MASK_PIECE_5 ((uint32_t)(MASK_PIECE_4 << 4))
#define MASK_PIECE_6 ((uint32_t)(MASK_PIECE_5 << 4))

#define GET_PIECE_MASK(var_piece_mask, piece_index)                                                                    \
    uint32_t(var_piece_mask);                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((piece_index) == 0)                                                                                        \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_0;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 1)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_1;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 2)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_2;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 3)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_3;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 4)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_4;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 5)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_5;                                                                           \
        }                                                                                                              \
        else if ((piece_index) == 6)                                                                                   \
        {                                                                                                              \
            (var_piece_mask) = MASK_PIECE_6;                                                                           \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            assert(false && "Invalid piece index");                                                                    \
        }                                                                                                              \
    }                                                                                                                  \
    while (0)

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

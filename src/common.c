#include "common.h"

#include <config.h>

bool any_piece_on_field(const uint32_t pieces, const uint8_t pos, size_t* piece_index)
{
    bool on_field = false;

    size_t tmp;
    if (!piece_index)
    {
        piece_index = &tmp;
    }

    for (short i = 0; i < NUM_OF_PIECES_PER_PLAYER; i++)
    {
        PIECE_FIELD_GET(piece_field, pieces, i);
        if (piece_field == pos)
        {
            on_field = true;
            *piece_index = i;
            break;
        }
    }

    return on_field;
}

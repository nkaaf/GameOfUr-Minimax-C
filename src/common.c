#include "common.h"

bool any_piece_on_field(const uint32_t pieces, const uint8_t pos, size_t* piece_index)
{
    bool on_field = false;

    size_t tmp;
    if (!piece_index)
    {
        piece_index = &tmp;
    }

    PIECE_FIELD_GET(piece_field, pieces, 0);
    if (piece_field == pos)
    {
        on_field = true;
        *piece_index = 0;
    }
    else
    {
        PIECE_FIELD_GET(piece_field, pieces, 1);
        if (piece_field == pos)
        {
            on_field = true;
            *piece_index = 1;
        }
        else
        {
            PIECE_FIELD_GET(piece_field, pieces, 2);
            if (piece_field == pos)
            {
                on_field = true;
                *piece_index = 2;
            }
            else
            {
                PIECE_FIELD_GET(piece_field, pieces, 3);
                if (piece_field == pos)
                {
                    on_field = true;
                    *piece_index = 3;
                }
                else
                {
                    PIECE_FIELD_GET(piece_field, pieces, 4);
                    if (piece_field == pos)
                    {
                        on_field = true;
                        *piece_index = 4;
                    }
                }
            }
        }
    }

    return on_field;
}
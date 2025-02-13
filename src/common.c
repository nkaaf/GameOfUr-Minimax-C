#include "common.h"

bool any_piece_on_field(const uint32_t pieces, const uint8_t pos,
                        size_t *piece_index, const minimax_config_t *config) {
  bool on_field = false;

  size_t tmp;
  if (!piece_index) {
    piece_index = &tmp;
  }

  for (size_t i = 0; i < config->num_of_pieces_per_player; i++) {
    PIECE_FIELD_GET(piece_field, pieces, i);
    if (piece_field == pos) {
      on_field = true;
      *piece_index = i;
      break;
    }
  }

  return on_field;
}

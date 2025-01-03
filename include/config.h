#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>

#include "common.h"

#define STEPS_IN_FUTURE (5)
#define PLAYER_1_MIN (true)
#define ROSETTE_9_IS_SAFE (true)

#define NUM_OF_PIECES_PER_PLAYER (5)
static_assert(NUM_OF_PIECES_PER_PLAYER <= 5 && "Higher numbers are not implemented.");

#define PIECES_1_START                                                                                                 \
    (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |             \
     PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START))
#define PIECES_2_START                                                                                                 \
    (PIECE_FIELD_SET(0, FIELD_START) | PIECE_FIELD_SET(1, FIELD_START) | PIECE_FIELD_SET(2, FIELD_START) |             \
     PIECE_FIELD_SET(3, FIELD_START) | PIECE_FIELD_SET(4, FIELD_START))
#define SCORE_1_START (0)
#define SCORE_2_START (0)
#define PLAYER_CURRENT_START (1)
#define PLAYER_OTHER_START (2)

#define EVAL_POINT_FINISH (100)
#define EVAL_POINT_START (-5)
#define EVAL_MULTIPLIER_ROSETTE (1.5)
#define EVAL_MULTIPLIER_KILLABLE (10)
#define EVAL_MULTIPLIER_ATTACKER (-1.5)
#define EVAL_ADDER_KILL_HAPPENS (100)

#define VISUALIZE (false)
#define VISUALIZE_THROWS_COUNT (5)
const static short visualize_throws[] = {1, 1, 1, 0, 0};

const static float evaluation_base_points[] = {
    EVAL_POINT_FINISH, EVAL_POINT_START, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
const static float evaluation_rosette_bonus[] = {
    0, 0, 1.0f / 16.0f, 1.0f / 4.0f, 3.0f / 8.0f, 1, 1.0f / 16.0f, 1.0f / 4.0f, 3.0f / 8.0f, 1,
    0, 0, 1.0f / 16.0f, 1.0f / 4.0f, 3.0f / 8.0f, 1};
const static float kill_distance_multipliers[] = {-1, 1.0f / 4.0f, 3.0f / 8.0f, 1.0f / 4.0f, 1.0f / 16.0f};

#endif // CONFIG_H

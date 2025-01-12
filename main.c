#include "config.h"
#include "sim.h"

int main(void)
{
    state_t* state_root = state_init(SCORE_1_START, SCORE_2_START, PIECES_1_START, PIECES_2_START, PLAYER_CURRENT_START,
                                     PLAYER_OTHER_START);

    get_best_move(state_root, -1);

    return 0;
}

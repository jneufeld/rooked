#include "proto.h"

int board[BOARD_SIZE];

/* Return the best move the AI can make. Use negascout!  */
void best_move (struct move *m)
{
    int curr_util  = -999;

    int legal_moves[BOARD_SIZE], i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] < chp_null) {
            gen_legal_moves (BPLAYER, i, legal_moves);
            // Now make the move (or subsequent moves) and evaluate
        }
    }
}

/* Return the utility of the board.  */
int board_utility ()
{
    return 0;
}

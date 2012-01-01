#include "proto.h"

int board[BOARD_SIZE];  // from board.c

/* Return the best move the AI can make.  */
void best_move (struct move *mv)
{
    int legal_moves[BOARD_SIZE];
    int curr_util = NEG_INF, i;

    /* For each black piece on the board, generate its legal moves and evaluate
     * its utility. Track the move with the greatest utility.  */
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] < chp_null) {
            init_moves_board (legal_moves);
            gen_legal_moves (BPLAYER, i, legal_moves);

            int j;
            for (j = 0; j < BOARD_SIZE; j++) {
                if (legal_moves[j] == TRUE) {
                    int attacked_piece = move_piece (i, j);
                    int move_util = -1 * abp_search (WPLAYER, SEARCH_DEP - 1, 
                        NEG_INF, POS_INF);
                    unmove_piece (i, j, attacked_piece);

                    if (move_util > curr_util) {
                        mv->start_pos = i;
                        mv->end_pos   = j;
                        curr_util     = move_util;
                    }
                }
            }
        }
    }
}

/* Alpha-beta pruning search.  */
int abp_search (int player, int depth, int alpha, int beta)
{
    int legal_moves[BOARD_SIZE];
    int curr_util = NEG_INF, i;
    int mod = (player == BPLAYER) ? -1 : 1;

    /* For each piece on the board, generate its legal moves and evaluate
     * its utility. Track the move with the greatest utility.  */
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] > chp_null * mod) {
            init_moves_board (legal_moves);
            gen_legal_moves (player, i, legal_moves);

            int j;
            for (j = 0; j < BOARD_SIZE; j++) {
                if (legal_moves[j] == TRUE) {
                    int attacked_piece = move_piece (i, j);
                    int move_util = 0;

                    if (depth == 0) {
                        move_util = (-1 * mod) * board_utility ();
                    } else {
                        move_util = -1 * abp_search (opponent_player (player),
                            depth - 1, -1 * beta, -1 * alpha);
                    }
                    
                    if (move_util > curr_util) {
                        curr_util = move_util;
                    }
                    if (curr_util > alpha) {
                        alpha = curr_util;
                    }
                    if (alpha >= beta) {
                        unmove_piece (i, j, attacked_piece);
                        return alpha;
                    }
                    
                    unmove_piece (i, j, attacked_piece);
                }
            }
        }
    }
    return curr_util;
}

/* Return the material (piece) score.  */
int material_score ()
{
    int wt_pawn, wt_rook, wt_knight, wt_bishop, wt_queen, wt_king;
    int bk_pawn, bk_rook, bk_knight, bk_bishop, bk_queen, bk_king;
    wt_pawn = wt_rook = wt_knight = wt_bishop = wt_queen = wt_king = 0;
    bk_pawn = bk_rook = bk_knight = bk_bishop = bk_queen = bk_king = 0;

    /* Get counts of each piece type for each player.  */
    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        switch (board[i]) {
            case chp_wpawn:
                wt_pawn++;
                break;
            case chp_wrook:
                wt_rook++;
                break;
            case chp_wbishop:
                wt_bishop++;
                break;
            case chp_wknight:
                wt_knight++;
                break;
            case chp_wqueen:
                wt_queen++;
                break;
            case chp_wking:
                wt_king++;
                break;
            case chp_bpawn:
                bk_pawn++;
                break;
            case chp_brook:
                bk_rook++;
                break;
            case chp_bbishop:
                bk_bishop++;
                break;
            case chp_bknight:
                bk_knight++;
                break;
            case chp_bqueen:
                bk_queen++;
                break;
            case chp_bking:
                bk_king++;
                break;
            default:
                break;
        }
    }

    /* Simple evaluation function: each piece type has a value, sum values and
     * return black's score minus white's score.  */
    int white_score = (wt_pawn * PAWN_VAL) + (wt_rook * ROOK_VAL) + (wt_bishop *
        BISHOP_VAL) + (wt_knight * KNIGHT_VAL) + (wt_queen * QUEEN_VAL) +
        (wt_king * KING_VAL);
    int black_score = (bk_pawn * PAWN_VAL) + (bk_rook * ROOK_VAL) + (bk_bishop *
        BISHOP_VAL) + (bk_knight * KNIGHT_VAL) + (bk_queen * QUEEN_VAL) +
        (bk_king * KING_VAL);

    return black_score - white_score;
}

/* Return the utility of the board.  */
int board_utility ()
{
    return material_score ();
}

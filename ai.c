#include "ai.h"
#include "board.h"

extern int board[BOARD_SIZE]; /* From board.c.  */

/* Search and evaluate AI's moves. MV->START_POS and MV->END_POS store AI's best
 * move.  */
void best_move (struct move *mv)
{
    int legal_moves[BOARD_SIZE];
    int curr_util = NEG_INF, i;

    /* Generate legal moves for each black piece.  */
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] < chp_null) {
            init_moves_board (legal_moves);
            gen_legal_moves (BPLAYER, i, legal_moves);

            /* For each legal move, evaluate subsequent moves. If this move
             * leads to current best score, save it.  */
            int j;
            for (j = 0; j < BOARD_SIZE; j++) {
                if (legal_moves[j] == TRUE) {
                    /* Make move and evaluate subsequent moves.  */
                    int attacked_piece = move_piece (i, j);

                    /* If the move wins the game, automatically make it.  */
                    if (game_over () == TRUE) {
                        mv->start_pos = i;
                        mv->end_pos   = j;
                        unmove_piece (i, j, attacked_piece);
                        return;
                    }

                    /* Evaluate subsequent moves and choose the best one.  */
                    int move_util = -1 * abp_search (WPLAYER, SEARCH_DEP - 1, 
                        NEG_INF, POS_INF);
                    unmove_piece (i, j, attacked_piece);

                    /* If move is best yet, save it.  */
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
    int mod = -1 * ((player == BPLAYER) ? -1 : 1);

    /* For each piece on the board, generate its legal moves and evaluate
     * its utility. Track the move with the greatest utility.  */
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] > chp_null * mod) {
            init_moves_board (legal_moves);
            gen_plegal_moves (player, i, legal_moves);

            int j;
            for (j = 0; j < BOARD_SIZE; j++) {
                if (legal_moves[j] == TRUE) {
                    int attacked_piece = move_piece (i, j);
                    int move_util = 0;

                    /* If move wins the game, automatically make that move.  */
                    if (game_over () == TRUE) {
                        unmove_piece (i, j, attacked_piece);
                        return (-1 * mod) * POS_INF;
                    }

                    /* If maximum depth reached evaluate the board. Else,
                     * continue search.  */
                    if (depth == 1) {
                        move_util = mod * board_utility ();
                    } else {
                        move_util = mod * abp_search (opponent_player (player),
                            depth - 1, -1 * beta, -1 * alpha);
                    }
                    
                    /* If this move's utility is a new maximum, save it. Alter
                     * alpha value and check against beta to potentially short
                     * circuit the search.  */
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

/* Return utility of BOARD as function of material and positional scores.  */
int board_utility ()
{
    return (MATERIAL_WT * material_score ())
        + (POSITION_WT * positional_score ());
}

/* Return the material (piece) score of BOARD.  */
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

    /* Material score is the number of each piece times it's value for each
     * player, then subtract white's score from black's score.  */
    return ((bk_pawn - wt_pawn) * PAWN_VAL) 
        + ((bk_knight - wt_knight) * KNIGHT_VAL)
        + ((bk_bishop - wt_bishop) * BISHOP_VAL)
        + ((bk_rook - wt_rook) * ROOK_VAL)
        + ((bk_queen - wt_queen) * QUEEN_VAL)
        + ((bk_king - wt_king) * KING_VAL);
}

/* Return the positional utility of BOARD.  */
int positional_score ()
{
    /* Sum of score of legal moves, attacks and defends by white pieces.  */
    int white_score = 0, i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] == chp_wknight || board[i] == chp_wbishop) {
            white_score += 2 * knight_pos_score (WPLAYER, i);
        } else if (board[i] == chp_wpawn) {
            white_score += knight_pos_score (WPLAYER, i);
        }
    }

    /* Sum of score of legal moves, attacks and defends by black pieces.  */
    int black_score = 0;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] == chp_bknight || board[i] == chp_bbishop) {
            black_score += 2 * knight_pos_score (BPLAYER, i);
        } else if (board[i] == chp_bpawn) {
            black_score += knight_pos_score (BPLAYER, i);
        }
    }

    /* Increase score for pawns, knights and bishops in center of board.  */ 
    for (i = 33; i < 82; i += 16) {
        int j;
        for (j = 0; j < 6; j++) {
            if (board[i + j] == chp_bbishop || board[i + j] == chp_bknight) {
                printf (":: EVAL: bk_bishop/knight in board center\n");
                black_score += 500;
            } else if (board[i + j] == chp_bpawn) {
                printf (":: EVAL: bk_pawn in board center\n");
                black_score += 200;
            } else if (board[i + j] == chp_wbishop 
                || board[i + j] == chp_wknight) {
                printf (":: EVAL: wt_bishop/knight in board center\n");
                white_score += 500;
            } else if (board[i + j] == chp_wpawn) {
                printf (":: EVAL: wt_pawn in board center\n");
                white_score += 200;
            }
        }
    }

    return black_score - white_score;
}

/* Return position score for knight at START_POS owned by PLAYER.  */
int knight_pos_score (int player, int start_pos)
{
    int score = 0, legal_moves[BOARD_SIZE];
    int mod   = (player == BPLAYER) ? -1 : 1;
    init_moves_board (legal_moves);
    gen_legal_moves (player, start_pos, legal_moves);

    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        /* Each legal move increases score.  */
        if (legal_moves[i] == TRUE) {
            score++;
            
            /* Each enemy piece attacked increases score.  */
            if (board[i] * mod < chp_null) {
                printf (":: EVAL: %c attacking piece\n", (player == WPLAYER) ?
                    'W' : 'B');
                score -= board[i] * mod;
            }
        }
    }

    return score;
}

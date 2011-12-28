#include "proto.h"
#include <stdio.h>

/* Use 0x88 representation for the chess board.  */
#define BOARD_SIZE 128
int board[BOARD_SIZE];
int wking_pos;
int bking_pos;
int checkmate;

/* Place all pieces in default start position and reset game state.  */
void reset_board () 
{
    /* Clear board completely, just fill it with null pieces.  */
    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        board[i] = chp_null;
    }

    /* Place black pawns in indices 96 - 103 and white pawns in 16 - 23.  */
    for (i = 16; i < 16 + 8; i++) {
        board[i] = chp_wpawn;
        board[i + 80] = chp_bpawn;
    }

    /* Place major pieces for white in indicies 0 - 7, first row.  */
    board[0] = board[7] = chp_wrook;
    board[1] = board[6] = chp_wknight;
    board[2] = board[5] = chp_wbishop;
    board[3] = chp_wqueen;
    board[4] = chp_wking;

    /* Place major pieces for black in indicies 112 - 119, last row.  */
    board[112] = board[119] = chp_brook;
    board[113] = board[118] = chp_bknight;
    board[114] = board[117] = chp_bbishop;
    board[115] = chp_bqueen;  
    board[116] = chp_bking;

    /* Set WKING_POS and BKING_POS to starting positions.  */
    wking_pos = 4;
    bking_pos = 116;
    checkmate = FALSE;
}

/* Print a crude command line version of the board. Just for debugging.  */
void print_board () 
{
    /* Represent each piece by a character. Capitalized pieces are white.  */
    char piece_codes[] = { 'k', 'q', 'b', 'n', 'r', 'p', ' ', 'P', 'R', 'N', 
    'B', 'Q', 'K' };

    printf ("\n  | a | b | c | d | e | f | g | h |\n");
    printf ("-----------------------------------\n");

    /* J moves along the 8 files of the board, I moves down the ranks. Since
     * the piece at I + J is printed, I begins at the top left and is
     * decremented by 16 to move down a rank (0x88 board has 16 files).  */
    int i, j, row_num = 8;
    for (i = 112; i >= 0; i -= 16) {
        printf ("%d | ", row_num--);
        for (j = 0; j < 8; j++) {
            printf ("%c | ", piece_codes[(chp_bking * -1) + board[i + j]]);
        }
        printf ("\n-----------------------------------\n");
    }
}

/* Return FALSE if POS is on the right side of the 0x88 board.  */ 
int valid_x88_move (int pos)
{
    if ((pos & 0x88) != 0) {
        return FALSE;
    }       
    return TRUE;
}

/* Return FALSE if POS is outside bounds of the board.  */
int square_on_board (int pos)
{
    if (pos >= BOARD_SIZE || pos < 0) {
        return FALSE;
    }
    return TRUE;
}

/* Return TRUE if the board contains any piece at index POS.  */
int square_is_occupied (int pos) 
{
    return (board[pos] == chp_null) ? FALSE : TRUE;
}

/* Return TRUE if board contains a piece owned by PLAYER at index POS.  */
int contains_players_piece (int player, int pos)
{
    if (player == WPLAYER && board[pos] > 0) {
        return TRUE;
    } else if (player == BPLAYER && board[pos] < 0) {
        return TRUE;
    }
    return FALSE;
}

/* Perform checks on a move's legality and return TRUE if the move is made.  */
int make_move (int player, int start_pos, int end_pos) 
{
    if (valid_start_pos (player, start_pos) == FALSE) {
        return FALSE;
    }
    if (valid_end_pos (end_pos) == FALSE) {
        return FALSE;
    }
    if (is_legal_move (player, start_pos, end_pos) == FALSE) {
        return FALSE;
    }

    move_piece (start_pos, end_pos);
    if (player_in_check (opponent_player (player)) == TRUE) {
        printf ("Move places opponent in check!\n");
    }

    if (player_has_moves (opponent_player (player)) == FALSE) {
        checkmate = TRUE;
    }

    return TRUE;
}

/* Move piece from START_POS to END_POS.  */
int move_piece (int start_pos, int end_pos)
{
    int moved        = board[start_pos];
    int attacked     = board[end_pos];
    board[start_pos] = chp_null;
    board[end_pos]   = moved;

    if (board[end_pos] == chp_wking) {
        wking_pos = end_pos;
    } else if (board[end_pos] == chp_bking) {
        bking_pos = end_pos;
    }

    return attacked;
}

/* Move piece from END_POS to START_POS and place OLD_PIECE in END_POS.  */
void unmove_piece (int start_pos, int end_pos, int old_piece)
{
    int moved        = board[end_pos];
    board[start_pos] = moved;
    board[end_pos]   = old_piece;

    if (board[start_pos] == chp_wking) {
        wking_pos = start_pos;
    } else if (board[start_pos] == chp_bking) {
        bking_pos = start_pos;
    }
}

/* Return TRUE if a move has valid START_POS and END_POS.  */
int valid_start_pos (int player, int pos)
{
    if (square_on_board (pos) == FALSE 
        || contains_players_piece (player, pos) == FALSE) {
        printf ("Error: invalid start_pos %d\n", pos);
        return FALSE;
    }
    return TRUE;
}

/* Return TRUE if a move has a valid END_POS.  */
int valid_end_pos (int pos)
{
    if (valid_x88_move (pos) == FALSE
        || square_on_board (pos) == FALSE) {
        printf ("Error: invalid end_pos %d\n", pos);
        return FALSE;
    }
    return TRUE;
}

/* Return WPLAYER if PLAYER is BPLAYER, else BPLAYER.  */
int opponent_player (int player)
{
    return (player == BPLAYER) ? WPLAYER : BPLAYER;
}

/* TRUE if move from START_POS to END_POS by PLAYER is legal.  */
int is_legal_move (int player, int start_pos, int end_pos) 
{
    int legal_moves[BOARD_SIZE];
    init_moves_board (legal_moves);

    gen_legal_moves (player, start_pos, legal_moves);

    if (legal_moves[end_pos] == FALSE) {
        printf ("Error: Illegal move %d - %d\n", start_pos, end_pos);
    }

    return legal_moves[end_pos];
}

/* Generate legal moves for piece at START_POS and store in MOVES_ARRAY.  */
void gen_legal_moves (int player, int start_pos, int *moves_array) 
{
    gen_plegal_moves (player, start_pos, moves_array);
    remove_check_moves (player, start_pos, moves_array);
}

/* Set moves leaving player in check to FALSE in MOVES_ARRAY.  */
void remove_check_moves (int player, int start_pos, int *moves_array)
{
    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (moves_array[i] == TRUE) {
            int attacked = move_piece (start_pos, i);
            if (player_in_check (player) == TRUE) {
                moves_array[i] = FALSE;
            }
            unmove_piece (start_pos, i, attacked);
        }
    }
}

/* Generate pseudo legal moves at START_POS and store in MOVES_ARRAY.  */
void gen_plegal_moves (int player, int start_pos, int *moves_array)
{
    switch (board[start_pos]) {
        case chp_wpawn:
            gen_wpawn_moves (start_pos, moves_array); 
            break;

        case chp_bpawn:
            gen_bpawn_moves (start_pos, moves_array);
            break;

        case chp_wknight:
        case chp_bknight:
            gen_knight_moves (player, start_pos, moves_array);
            break;

        case chp_wking:
        case chp_bking:
            gen_king_moves (player, start_pos, moves_array);
            break;

        case chp_wrook:
        case chp_brook:
            gen_rook_moves (player, start_pos, moves_array);
            break;

        case chp_wbishop:
        case chp_bbishop:
            gen_bishop_moves (player, start_pos, moves_array);
            break;

        case chp_wqueen:
        case chp_bqueen:
            gen_queen_moves (player, start_pos, moves_array);
            break;
    }
}

/* Set each index of a legal move for white pawn in MOVES_ARRAY to TRUE.  */
void gen_wpawn_moves (int start_pos, int *moves_array)
{
    /* Move up one if not blocked, two if first move.  */
    int up_one = MOVE_UP + start_pos;
    if ((up_one < BOARD_SIZE)
        && (square_is_occupied (up_one) == FALSE)) {
        moves_array[up_one] = TRUE;
    }

    int up_two = MOVE_UP + up_one;
    if ((up_two < BOARD_SIZE)
        && (start_pos > 15 && start_pos < 24)
        && (square_is_occupied (up_one) == FALSE)
        && (square_is_occupied (up_two) == FALSE) ) {
            moves_array[up_two] = TRUE;
    }

    /* Attack right or left.  */
    int up_right = MOVE_DU_RIGHT + start_pos;
    if ((up_right < BOARD_SIZE)
        && (valid_x88_move (up_right))
        && (board[up_right] <= chp_bpawn)) {
        moves_array[up_right] = TRUE;
    }

    int up_left  = MOVE_DU_LEFT + start_pos;
    if ((up_left < BOARD_SIZE)
        && (valid_x88_move (up_left))
        && (board[up_left] <= chp_bpawn)) {
        moves_array[up_left] = TRUE;
    } 
}

/* Set each index of a legal move for black pawn in MOVES_ARRAY to TRUE.  */
void gen_bpawn_moves (int start_pos, int *moves_array)
{
    /* Move down one if not blocked, two if first move.  */
    int down_one = MOVE_DOWN + start_pos;
    if ((down_one >= 0)
        && (square_is_occupied (down_one) == FALSE)) {
        moves_array[down_one] = TRUE;
    }

    int down_two = MOVE_DOWN + down_one;
    if ((down_two >= 0)
        && (start_pos > 95 && start_pos < 104)
        && (square_is_occupied (down_one) == FALSE)
        && (square_is_occupied (down_two) == FALSE) ) {
            moves_array[down_two] = TRUE;
    }

    /* Attack right or left.  */
    int down_right = MOVE_DD_RIGHT + start_pos;
    if ((down_right >= 0)
        && (valid_x88_move (down_right))
        && (board[down_right] >= chp_wpawn)) {
        moves_array[down_right] = TRUE;
    }

    int down_left  = MOVE_DD_LEFT + start_pos;
    if ((down_left >= 0)
        && (valid_x88_move (down_left))
        && (board[down_left] >= chp_wpawn)) {
        moves_array[down_left] = TRUE;
    } 
}

/* Set each index of a legal move for knight in MOVES_ARRAY to TRUE.  */
void gen_knight_moves (int player, int start_pos, int *moves_array)
{
    /* White pieces have positive values, so black knight must move into a space
     * with value >= 0. To make this more general, when PLAYER is white a legal
     * move is into a space with value <= 0, since black pieces have negative
     * value. Thus, multiply by -1 and we can use >= 0 for both.  */
    int mod = (player == BPLAYER) ? 1 : -1;

    if ((MOVE_K_URV + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_K_URV + start_pos))
        && (board[MOVE_K_URV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_URV + start_pos] = TRUE;
    }
    if ((MOVE_K_URH + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_K_URH + start_pos))
        && (board[MOVE_K_URH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_URH + start_pos] = TRUE;
    }
    if ((MOVE_K_ULH + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_K_ULH + start_pos))
        && (board[MOVE_K_ULH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_ULH + start_pos] = TRUE;
    }
    if ((MOVE_K_ULV + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_K_ULV + start_pos))
        && (board[MOVE_K_ULV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_ULV + start_pos] = TRUE;
    }
    if ((MOVE_K_DRH + start_pos >= 0)
        && (valid_x88_move (MOVE_K_DRH + start_pos))
        && (board[MOVE_K_DRH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DRH + start_pos] = TRUE;
    }
    if ((MOVE_K_DRV + start_pos >= 0)
        && (valid_x88_move (MOVE_K_DRV + start_pos))
        && (board[MOVE_K_DRV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DRV + start_pos] = TRUE;
    }
    if ((MOVE_K_DLV + start_pos >= 0)
        && (valid_x88_move (MOVE_K_DLV + start_pos))
        && (board[MOVE_K_DLV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DLV + start_pos] = TRUE;
    }
    if ((MOVE_K_DLH + start_pos >= 0) 
        && (valid_x88_move (MOVE_K_DLH + start_pos))
        && (board[MOVE_K_DLH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DLH + start_pos] = TRUE;
    }
}

/* Set each index of a legal move for king in MOVES_ARRAY to TRUE.  */
void gen_king_moves (int player, int start_pos, int *moves_array)
{
    /* White pieces have positive values, so black king must move into a space
     * with value >= 0. To make this more general, when PLAYER is white a legal
     * move is into a space with value <= 0, since black pieces have negative
     * value. Thus, multiply by -1 and we can use >= 0 for both.  */
    int mod = (player == BPLAYER) ? 1 : -1;
    
    if ((MOVE_UP + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_UP + start_pos) == TRUE)
        && (board[MOVE_UP + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_UP + start_pos] = TRUE;
    }
    if ((MOVE_RIGHT + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_RIGHT + start_pos) == TRUE)
        && (board[MOVE_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_RIGHT + start_pos] = TRUE;
    }
    if ((MOVE_DU_RIGHT + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_DU_RIGHT + start_pos) == TRUE)
        && (board[MOVE_DU_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DU_RIGHT + start_pos] = TRUE;
    }  
    if ((MOVE_DU_LEFT + start_pos < BOARD_SIZE)
        && (valid_x88_move (MOVE_DU_LEFT + start_pos) == TRUE)
        && (board[MOVE_DU_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DU_LEFT + start_pos] = TRUE;
    }
    if ((MOVE_DOWN + start_pos >= 0)
        && (valid_x88_move (MOVE_DOWN + start_pos) == TRUE)
        && (board[MOVE_DOWN + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DOWN + start_pos] = TRUE;
    }
    if ((MOVE_LEFT + start_pos >= 0)
        && (valid_x88_move (MOVE_LEFT + start_pos) == TRUE)
        && (board[MOVE_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_LEFT + start_pos] = TRUE;
    }
    if ((MOVE_DD_RIGHT + start_pos >= 0)
        && (valid_x88_move (MOVE_DD_RIGHT + start_pos) == TRUE)
        && (board[MOVE_DD_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DD_RIGHT + start_pos] = TRUE;
    }
    if ((MOVE_DD_LEFT + start_pos >= 0)
        && (valid_x88_move (MOVE_DD_LEFT + start_pos) == TRUE)
        && (board[MOVE_DD_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DD_LEFT + start_pos] = TRUE;
    }
}

/* Set index of each legal move for a sliding piece to TRUE in MOVES_ARRAY.  */
void gen_sliding_moves (int start_pos, int mod, int move_dir, int *moves_array)
{ 
    /* For each move direction, the move is legal if the space is empty. If the
     * space contains an opponents piece, the move is legal and the loop breaks,
     * since the piece is then blocked.  */
    int i;
    for (i = 1; i < 8; i++) {
        int move = (move_dir * i) + start_pos;
        if ((square_on_board (move)) && (valid_x88_move (move) == TRUE)) {
            if (board[move] == chp_null) {
                moves_array[move] = TRUE;
            } else if (board[move] * mod > chp_null) {
                moves_array[move] = TRUE;
                break;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

/* Generate legal moves in directions rook moves with GEN_SLIDING_MOVES.  */
void gen_rook_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? 1 : -1;
    gen_sliding_moves (start_pos, mod, MOVE_UP, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DOWN, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_LEFT, moves_array);
}

/* Generate legal moves in directions bishop moves using GEN_SLIDING_MOVES.  */
void gen_bishop_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? 1 : -1;
    gen_sliding_moves (start_pos, mod, MOVE_DU_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_LEFT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DU_LEFT, moves_array);
}

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
void gen_queen_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? 1 : -1;
    gen_sliding_moves (start_pos, mod, MOVE_UP, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DOWN, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_LEFT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DU_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_LEFT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DU_LEFT, moves_array);
}

/* Return TRUE if PLAYER's king is in check.  */
int player_in_check (int player)
{
    int king_pos = (player == WPLAYER) ? wking_pos : bking_pos;
    int mod = (player == BPLAYER) ? 1 : -1;
    int moves_array[BOARD_SIZE];

    if (player_check_by_pawn (king_pos, player) == TRUE) {
        return TRUE;
    }
    if (player_check_by_knight (king_pos, player, moves_array) == TRUE) {
        return TRUE;
    }
    if (player_check_by_rook (king_pos, mod, moves_array) == TRUE) {
        return TRUE;
    }
    if (player_check_by_bishop (king_pos, mod, moves_array) == TRUE) {
        return TRUE;
    }

    return FALSE;
}

/* Check if king at START_POS is in check by opponent's pawn.  */
int player_check_by_pawn (int start_pos, int player)
{
    if (player == WPLAYER) {
        if ((board[MOVE_DU_RIGHT + start_pos] == chp_bpawn)
            || (board[MOVE_DU_LEFT + start_pos] == chp_bpawn)) {
            return TRUE;
        }
    } else {
        if ((board[MOVE_DD_RIGHT + start_pos] == chp_wpawn)
            || (board[MOVE_DD_LEFT + start_pos] == chp_wpawn)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Generate legal moves for king as if it were a knight and return TRUE if king
 * is in check by these pieces.  */
int player_check_by_knight (int start_pos, int player, int *moves_array)
{
    /* Pretend king is a knight and generate moves. If king can then attack an
     * enemy's knight, king is in check.  */
    init_moves_board (moves_array);
    gen_knight_moves (player, start_pos, moves_array);
    int mod = (player == BPLAYER) ? 1 : -1;

    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if ((moves_array[i] == TRUE) && (board[i] == chp_wknight * mod)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Generate legal moves for king as if it were a rook or queen and return TRUE
 * if king is in check by these pieces.  */
int player_check_by_rook (int start_pos, int mod, int *moves_array)
{
    /* Pretend king is a rook/queen and generate moves. If king can then attack
     * an enemy's rook/queen, king is in check.  */
    init_moves_board (moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_UP, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DOWN, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_LEFT, moves_array);

    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if ((moves_array[i] == TRUE) 
            && (board[i] == chp_wrook * mod || board[i] == chp_wqueen * mod)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Generate legal moves for king as if it were a rook or queen and return TRUE
 * if king is in check by these pieces.  */
int player_check_by_bishop (int start_pos, int mod, int *moves_array)
{
    /* Pretend king is a bishop/queen and generate moves. If king can then 
     * attack an enemy's rook/queen, king is in check.  */
    init_moves_board (moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DU_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_RIGHT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DD_LEFT, moves_array);
    gen_sliding_moves (start_pos, mod, MOVE_DU_LEFT, moves_array);

    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        if ((moves_array[i] == TRUE) 
            && (board[i] == chp_wbishop * mod 
                || board[i] == chp_wqueen * mod)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Fill MOVES_ARRAY with FALSE values.  */
void init_moves_board (int *moves_array)
{
    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        moves_array[i] = FALSE;
    }
}

/* Return TRUE if PLAYER has a legal move.  */
int player_has_moves (int player)
{
    int i, mod = (player == BPLAYER) ? -1 : 1;

    /* If a board square contains a player's piece, generate all legal moves for
     * that piece, then check if there are any legal moves. If there are, simply
     * return TRUE. Else continue for all pieces.  */
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i] * mod > chp_null) {
            int moves_array[BOARD_SIZE];
            init_moves_board (moves_array);
            gen_legal_moves (player, i, moves_array);

            int j;
            for (j = 0; j < BOARD_SIZE; j++) {
                if (moves_array[j] == TRUE) {
                    printf (":: player_has_moves: legal %d - %d\n", i, j);
                    return TRUE;
                }
            }
        }
    }
    
    return FALSE;
}

/* Return TRUE if the game has been won. No special checking for 50 move
 * draw or other special termination conditions currently implemented.  */
int game_over () 
{
    return checkmate;
}

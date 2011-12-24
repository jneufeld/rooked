#include "proto.h"
#include <stdio.h>

/* Use 0x88 representation for the chess board.  */
#define BOARD_SIZE 128
int board[BOARD_SIZE];

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
    /* START_POS must be on board and contain a player's piece.  */
    if (square_on_board (start_pos) == FALSE 
        || contains_players_piece (player, start_pos) == FALSE) {
        printf ("Error: invalid start_pos %d\n", start_pos);
        return FALSE;
    }

    /* END_POS must be valid 0x88 board index and on board. */
    if (valid_x88_move (end_pos) == FALSE
        || square_on_board (end_pos) == FALSE) {
        printf ("Error: invalid end_pos %d\n", end_pos);
        return FALSE;
    }

    /* Check that the move is legal for the given piece.  */
    if (is_legal_move (player, start_pos, end_pos) == FALSE) {
        printf ("Error: Illegal move %d - %d\n", start_pos, end_pos);
        return FALSE;
    }

    int piece = board[start_pos];
    board[start_pos] = chp_null;
    board[end_pos]   = piece;
    return TRUE;
}

/* Generate legal moves for piece at START_POS and store in MOVES_ARRAY.  */
void gen_legal_moves (int player, int start_pos, int *moves_array) 
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

/* TRUE if move from START_POS to END_POS by PLAYER is legal.  */
int is_legal_move (int player, int start_pos, int end_pos) 
{
    int legal_moves[BOARD_SIZE], i;
    for (i = 0; i < BOARD_SIZE; i++) {
        legal_moves[i] = FALSE;
    }

    gen_legal_moves (player, start_pos, legal_moves);
    return legal_moves[end_pos];
}

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
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

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
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

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
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

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
void gen_king_moves (int player, int start_pos, int *moves_array)
{
    /* White pieces have positive values, so black king must move into a space
     * with value >= 0. To make this more general, when PLAYER is white a legal
     * move is into a space with value <= 0, since black pieces have negative
     * value. Thus, multiply by -1 and we can use >= 0 for both.  */
    int mod = (player == BPLAYER) ? -1 : 1;
    
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

/* Generate legal moves in directions rook moves with GEN_ROOK_LINE_MOVES.  */
void gen_rook_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? 1 : -1;
    gen_rook_line_moves (start_pos, mod, MOVE_UP, moves_array);
    gen_rook_line_moves (start_pos, mod, MOVE_RIGHT, moves_array);
    gen_rook_line_moves (start_pos, mod, MOVE_DOWN, moves_array);
    gen_rook_line_moves (start_pos, mod, MOVE_LEFT, moves_array);

printf ("Rook on %d can move to:\n", start_pos);
int i;
for (i = 0; i < BOARD_SIZE; i++) {
    if (moves_array[i] == TRUE) printf ("%d ", i);
}
printf ("\n\n");
}

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
void gen_rook_line_moves (int start_pos, int mod, int move_dir, 
                          int *moves_array)
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

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
void gen_bishop_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? -1 : 1;
}

/* Set each index of a legal move in MOVES_ARRAY to TRUE.  */
void gen_queen_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? -1 : 1;
}

/* Return TRUE if the game has been won. No special checking for 50 move
 * draw or other special termination conditions currently implemented.  */
int game_over () 
{
    return FALSE;
}

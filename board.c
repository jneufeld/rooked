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
    }
    return; // TODO

    /* Sliding pieces require a special loop to compare the argument
     * end_square with the piece's delta. During the search, if a vector
     * happens to be blocked at some point, the delta is re-written so that
     * future iterations of the search no longer attempt to check along that
     * vector. This shouldn't damage the search, as if a legal move along that
     * vector were to be found, it would be found before that point and thus
     * would have been returned.  */
    int delta[8];
    make_delta (delta, board[start_pos]);
    int i, j;
    for (i = 1; i < 7; i++) {
        for (j = 0; j < 8; j++) {
            int square = (delta[j] * i) + start_pos;

            /* If the square we're trying to check is outside the
             * bounds of the array, just skip it.  */
            if (square_on_board (square) == FALSE) {
                continue;
            }

            /* Check if the vector is blocked. If it is, remove this
             * direction from the delta.  */
            if (square_is_occupied (square) == TRUE) {
                delta[j] = MOVE_NULL;
            }

            /* The move is legal.  */
            if (square == 0 //end_pos
                && contains_players_piece (player, square) == FALSE) {
                //return TRUE;
            }
        }
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

void gen_wpawn_moves (int start_pos, int *moves_array)
{
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

    int up_right = MOVE_DU_RIGHT + start_pos;
    if ((up_right < BOARD_SIZE)
        && (board[up_right] <= chp_bpawn)) {
        moves_array[up_right] = TRUE;
    }

    int up_left  = MOVE_DU_LEFT + start_pos;
    if ((up_left < BOARD_SIZE)
        && (board[up_left] <= chp_bpawn)) {
        moves_array[up_left] = TRUE;
    } 
}

void gen_bpawn_moves (int start_pos, int *moves_array)
{
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

    int down_right = MOVE_DD_RIGHT + start_pos;
    if ((down_right >= 0)
        && (board[down_right] >= chp_wpawn)) {
        moves_array[down_right] = TRUE;
    }

    int down_left  = MOVE_DD_LEFT + start_pos;
    if ((down_left >= 0)
        && (board[down_left] >= chp_wpawn)) {
        moves_array[down_left] = TRUE;
    } 
}

void gen_knight_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? 1 : -1;

    if ((MOVE_K_URV + start_pos < BOARD_SIZE)
        && (board[MOVE_K_URV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_URV + start_pos] = TRUE;
    }
    if ((MOVE_K_URH + start_pos < BOARD_SIZE)
        && (board[MOVE_K_URH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_URH + start_pos] = TRUE;
    }
    if ((MOVE_K_ULH + start_pos < BOARD_SIZE)
        && (board[MOVE_K_ULH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_ULH + start_pos] = TRUE;
    }
    if ((MOVE_K_ULV + start_pos < BOARD_SIZE)
        && (board[MOVE_K_ULV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_ULV + start_pos] = TRUE;
    }
    if ((MOVE_K_DRH + start_pos >= 0)
        && (board[MOVE_K_DRH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DRH + start_pos] = TRUE;
    }
    if ((MOVE_K_DRV + start_pos >= 0)
        && (board[MOVE_K_DRV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DRV + start_pos] = TRUE;
    }
    if ((MOVE_K_DLV + start_pos >= 0)
        && (board[MOVE_K_DLV + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DLV + start_pos] = TRUE;
    }
    if ((MOVE_K_DLH + start_pos >= 0) 
        && (board[MOVE_K_DLH + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_K_DLH + start_pos] = TRUE;
    }
}

void gen_king_moves (int player, int start_pos, int *moves_array)
{
    int mod = (player == BPLAYER) ? -1 : 1;
    
    if ((MOVE_UP + start_pos < BOARD_SIZE)
        && (board[MOVE_UP + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_UP + start_pos] = TRUE;
    }
    if ((MOVE_RIGHT + start_pos < BOARD_SIZE)
        && (board[MOVE_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_RIGHT + start_pos] = TRUE;
    }
    if ((MOVE_DU_RIGHT + start_pos < BOARD_SIZE)
        && (board[MOVE_DU_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DU_RIGHT + start_pos] = TRUE;
    }  
    if ((MOVE_DU_LEFT + start_pos < BOARD_SIZE)
        && (board[MOVE_DU_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DU_LEFT + start_pos] = TRUE;
    }
    if ((MOVE_DOWN + start_pos >= 0)
        && (board[MOVE_DOWN + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DOWN + start_pos] = TRUE;
    }
    if ((MOVE_LEFT + start_pos >= 0)
        && (board[MOVE_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_LEFT + start_pos] = TRUE;
    }
    if ((MOVE_DD_RIGHT + start_pos >= 0)
        && (board[MOVE_DD_RIGHT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DD_RIGHT + start_pos] = TRUE;
    }
    if ((MOVE_DD_LEFT + start_pos >= 0)
        && (board[MOVE_DD_LEFT + start_pos] * mod >= chp_null)) {
        moves_array[MOVE_DD_LEFT + start_pos] = TRUE;
    }
}

/* Create delta for a sliding piece. A delta is a 8-element array. Each 
 * element represents a direction, the 0th element being up, 1st element
 * being up-right diagonally, etc.. Once the basic direction of the piece
 * is created in the delta it can be used to determine whether moving that
 * piece into a certain square is legal (this is done in the is_legal_move
 * function).  */
void make_delta (int delta[], int piece)
{
    /* Delta should be empty initially.  */
    int i;
    for (i = 0; i < 8; i++) {
        delta[i] = MOVE_NULL;
    }

    /* Fill the delta array based based on the type of piece.  */
    switch (piece) {
        /* White and black rook can both slide vertically and
         * horizontally.  */
        case chp_wrook:
        case chp_brook:
            delta[0] = MOVE_UP;
            delta[2] = MOVE_RIGHT;
            delta[4] = MOVE_DOWN;
            delta[6] = MOVE_LEFT;
            break;

        /* White and black bishop can both slide diagonally.  */
        case chp_wbishop:
        case chp_bbishop:
            delta[1] = MOVE_DU_RIGHT;
            delta[3] = MOVE_DD_RIGHT;
            delta[5] = MOVE_DD_LEFT;
            delta[7] = MOVE_DU_LEFT;
            break;

        /* White and black queen can move any direction.  */
        case chp_wqueen:
        case chp_bqueen:
            delta[0] = MOVE_UP;
            delta[1] = MOVE_DU_RIGHT;
            delta[2] = MOVE_RIGHT;
            delta[3] = MOVE_DD_RIGHT;
            delta[4] = MOVE_DOWN;
            delta[5] = MOVE_DD_LEFT;
            delta[6] = MOVE_LEFT;
            delta[7] = MOVE_DU_LEFT;
            break;
    }
}

/* Return TRUE if the game has been won. No special checking for 50 move
 * draw or other special termination conditions currently implemented.  */
int game_over () 
{
    return FALSE;
}

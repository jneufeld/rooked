/* Include the chess piece enumerations and function prototypes.  */
#include "proto.h"
#include <stdio.h>

/* Use 0x88 representation for the chess board.  */
#define BOARD_SIZE 128
int board[BOARD_SIZE];

/* Place all pieces in default start position. Also reset game state,
 * like en passent, move count, castling rights, etc.  */
void reset_board () 
{
    /* Clear board completely, just fill it with null pieces.  */
    int i;
    for (i = 0; i < BOARD_SIZE; i++) {
        board[i] = chp_null;
    }

    /* Place black pawns in indices 96 - 103 and white pawns in
     * 16 - 23.  */
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

/* Print the board to the console in crude manner. This is strictly for
 * debugging purposes, as the program will always be run through the XBoard
 * GUI.  */
void print_board () 
{
    char display[] = { 'k', 'q', 'b', 'n', 'r', 'p', ' ',
        'P', 'R', 'N', 'B', 'Q', 'K' };

    /* Print the numerical value of each piece, i.e. 0 is the null
     * piece and -6 is the black king, for each board square. Make
     * sure to skip the right side of the board since we're using
     * 0x88.  */
    printf ("\n  | a | b | c | d | e | f | g | h |\n");
    printf ("-----------------------------------\n");

    int i, j, m = 8;
    for (i = 112; i >= 0; i -= 16) {
        printf ("%d | ", m--);
        for (j = 0; j < 8; j++) {
            printf ("%c | ", display[(chp_bking * -1) + board[i + j]]);
        }
        printf ("\n-----------------------------------\n");
    }
}

/* Return FALSE if the argument SQUARE falls outside the bounds of the board
 * array.  */
int test_square (int square)
{
    if (square >= BOARD_SIZE || square < 0) {
        return FALSE;
    }
    return TRUE;
}

/* Return TRUE if board[square] has a piece in it, otherwise return FALSE.  */
int square_is_occupied (int square) 
{
    /* If the given square is outside the bounds of the array we'd
     * better just return false before something nasty happens.  */
    if (test_square (square) == FALSE) {
        return FALSE;
    }

    /* Return true if the square does not contain the null piece.  */
    return (board[square] == chp_null) ? FALSE : TRUE;
}

/* Return TRUE if board[square] contains a piece owned by player.  */
int is_players_piece (int player, int square)
{
    if (test_square (square) == FALSE || square_is_occupied (square) == FALSE) {
        return FALSE;
    } else if (player == WPLAYER && board[square] > 0) {
        return TRUE;
    } else if (player == BPLAYER && board[square] < 0) {
        return TRUE;
    }

    return FALSE;
}

/* Attempt to move piece at START_SQUARE into END_SQUARE. This will perform
 * lots of checks to make sure the move is legal. No outside checking of a 
 * move's legality is necessary from the outside. Simply attempting to make 
 * the move will result in either the move being made, in which case FALSE is
 * return, or returning TRUE, meaning the move is illegal.  */
int make_move (int player, int start_square, int end_square) 
{
    /* Check that START_SQUARE has a piece and that it belongs to
     * the player.  */
    if (square_is_occupied (start_square) == FALSE
        || is_players_piece (player, start_square) == FALSE) {
        printf ("Error: Start position %d - %d\n", start_square, end_square);
        return FALSE;
    }

    /* Check that END_SQUARE is on the actual board, not the right
     * side board from the 0x88 representation.  */
    if ((end_square & 0x88) != 0) {
        printf ("Error: End positon %d - %d/n", start_square, end_square);
        return FALSE;
    }

    /* Check that the move is legal for the given piece.  */
    if (is_legal_move (player, start_square, end_square) == FALSE) {
        printf ("Error: Illegal move %d - %d\n", start_square, end_square);
        return FALSE;
    }

     /* The is_legal_move function should ensure the move doesn't
     * capture the player's own piece.  */
    int piece = board[start_square];
    board[start_square] = chp_null;
    board[end_square]   = piece;
    return TRUE;
}

/* Check if the move is legal for the specified piece by comparing it's delta
 * with START_SQUARE and END_SQUARE. The delta array is created for each piece
 * to indicate which squares it can move into from it's current position. More
 * information on how it works is provided in the make_delta function. Return
 * TRUE if the move is legal, otherwise return FALSE. Any function that calls
 * this should check if START_SQUARE contains the players piece, or any piece
 * at all, so there is no need to check again.  */
int is_legal_move (int player, int start_square, int end_square) 
{
    /* For simple (minor) pieces, check if the argument end_square
     * is in the piece's delta. If it is, the move is legal.  */
    switch (board[start_square]) {

        /* White pawn can move up one square, one up diagonally to attack
         * or two up if it hasn't moved yet.  */
        case chp_wpawn:
            if (MOVE_UP + start_square == end_square) {
                return TRUE;
            }
            if ( (start_square > 15 && start_square < 24)
                && (MOVE_UP + start_square + MOVE_UP == end_square)
                && (square_is_occupied (MOVE_UP + start_square) == FALSE) ) {
                return TRUE;
            }
            if ( (MOVE_DU_RIGHT + start_square == end_square
                    && board[end_square] <= chp_bpawn) 
                || (MOVE_DU_LEFT + start_square == end_square
                    && board[end_square] <= chp_bpawn) ) {
                return TRUE;
            }
            /* None of the legal moves matched, so move is illegal.  */
            return FALSE;

        /* Black pawn can move down one square, one down diagonally to attack
         * or two down if it hasn't moved yet.  */
        case chp_bpawn:
            if (MOVE_DOWN + start_square == end_square) {
                return TRUE;
            }
            if ( (start_square > 95 && start_square < 104)
                && (MOVE_DOWN + start_square + MOVE_DOWN == end_square)
                && (square_is_occupied (MOVE_DOWN + start_square) == FALSE) ) {
                return TRUE;
            }
            if ( (MOVE_DD_RIGHT + start_square == end_square
                    && board[end_square] >= chp_wpawn)
                || (MOVE_DD_LEFT + start_square == end_square
                    && board[end_square] >= chp_wpawn) ) {
                return TRUE;
            }
            /* None of the legal moves matched, so move is illegal.  */
            return FALSE;

        /* White knight has funky moves, same as black knight so group them
         * together.  */
        case chp_wknight:
        case chp_bknight:
            if ( (MOVE_K_URV + start_square == end_square)
                || (MOVE_K_URH + start_square == end_square)
                || (MOVE_K_DRH + start_square == end_square)
                || (MOVE_K_DRV + start_square == end_square)
                || (MOVE_K_DLV + start_square == end_square)
                || (MOVE_K_DLH + start_square == end_square)
                || (MOVE_K_ULH + start_square == end_square)
                || (MOVE_K_ULV + start_square == end_square) ) {
                return TRUE;
            }
            /* None of the legal moves matched, so move is illegal.  */
            return FALSE;

        /* White king can move one space in any direction, same as black king
         * so group them together.  */
        case chp_wking:
        case chp_bking:
            if ( (MOVE_UP + start_square == end_square)
                || (MOVE_DU_RIGHT + start_square == end_square)
                || (MOVE_DD_RIGHT + start_square == end_square)
                || (MOVE_DOWN + start_square == end_square)
                || (MOVE_DD_LEFT + start_square == end_square)
                || (MOVE_LEFT + start_square == end_square) 
                || (MOVE_DU_LEFT + start_square == end_square) ) {
                return TRUE;
            }
            /* None of the legal moves matched, so move is illegal.  */
            return FALSE;
    }
    
    /* Sliding pieces require a special loop to compare the argument
     * end_square with the piece's delta. During the search, if a vector
     * happens to be blocked at some point, the delta is re-written so that
     * future iterations of the search no longer attempt to check along that
     * vector. This shouldn't damage the search, as if a legal move along that
     * vector were to be found, it would be found before that point and thus
     * would have been returned.  */
    int delta[8];
    make_delta (delta, board[start_square]);
    int i, j;
    for (i = 1; i < 7; i++) {
        for (j = 0; j < 8; j++) {
            int square = (delta[j] * i) + start_square;

            /* If the square we're trying to check is outside the
             * bounds of the array, just skip it.  */
            if (test_square (square) == FALSE) {
                continue;
            }

            /* Check if the vector is blocked. If it is, remove this
             * direction from the delta.  */
            if (square_is_occupied (square) == TRUE) {
                delta[j] = MOVE_NULL;
            }

            /* The move is legal.  */
            if (square == end_square 
                && is_players_piece (player, square) == FALSE) {
                return TRUE;
            }
        }
    }
    return FALSE;
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

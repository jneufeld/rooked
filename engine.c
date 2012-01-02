#include <stdio.h>
#include <string.h>

#include "ai.h"
#include "engine.h"
#include "board.h"

FILE *fp;
char  str_buff[BUF_SIZE];
int   curr_player;
extern int board[BOARD_SIZE];  /* From board.c, for debugging move evaluation.  */

/* XBoard starts engine from here.  */
int main (int argc, char *argv[]) 
{
    /* Open a logging file that records everything received from XBoard and some
     * output sent to XBoard.  */
    fp = fopen ("iolog.txt", "w");
    if (fp == NULL) {
        return -1;
    }

    /* XBoard suggests the following to fix buffering for I/O problems.  */
    setbuf (stdout, NULL);
    setbuf (stdin, NULL);

    /* -c for command-line test game, 2-player.  */
    if (argc >= 2 && strncmp (argv[1], "-c", 2) == 0) {
        play_test_game ();
        return 0;
    } 

    /* -a for command-line test game vs AI.  */
    else if (argc >= 2 && strncmp (argv[1], "-a", 2) == 0) {
        play_ai_game ();
        return 0;
    } 

    /* -t for a search test. Useful to check search time.  */
    else if (argc >= 2 && strncmp (argv[1], "-t", 2) == 0) {
        search_test ();
        return 0;
    } 

    /* -e for an evaluation test. Displays material and positional scores for a
     * variety of board positions.  */
    else if (argc >= 2 && strncmp (argv[1], "-e", 2) == 0) {
        eval_test ();
        return 0;
    } 

    /* If command-line arguments aren't nicely formatted, present usage.  */
    else if (argc >= 2) {
        printf ("Argument(s) not recognized.\n");
        printf ("\t-c play command line 2-player game\n");
        printf ("\t-a play command line 2-player game vs AI\n");
        printf ("\t-t run a test search\n");
        printf ("\tno arguments for regular XBoard game\n");
        return -1;
    } 

    /* No arguments usually means jgn-chess is being invoked by XBoard.  */
    else {
        while (strncmp ("quit", str_buff, 4) != 0) { 
            get_input ();

            /* XBoard requests new game.  */
            if (strncmp ("new", str_buff, 3) == 0) { 
                play_game ();
            } 

            /* Send features list to XBoard. Don't alter this, see XBoard
             * documentation if you want to send different features.  */ 
            else if (strncmp ("protover 2", str_buff, 10) == 0) { 
                printf ("feature myname=\"jgn-chess\" usermove=1 sigint=0 done=1\n");
            }
        }
    }

    /* Close logging file and exit cleanly.  */
    fclose (fp);
    return 0;
}

/* Set all elements in the string buffer to the null character.  */
void clean_buffer () 
{
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        str_buff[i] = '\0';
    }
}

/* Small function to read one character at a time and place it in the string
 * buffer, str_buff. It reads until the newline character (return key) is 
 * reached.  */
void get_input () 
{
    clean_buffer ();

    int ch, i = 0;
    while ((ch = getchar ()) != '\n') {
        /* If XBoard sends an unusually large string, record it and print an
         * error to the log file.  */
        if (i > BUF_SIZE) {
            fprintf (fp, "E: XBoard sent huge string:\n * %s", str_buff);
            break;
        }
        str_buff[i++] = ch;
    }
    fprintf (fp, "R: %s\n", str_buff);
}

/* Clear the board of pieces, reset move counts and all state associated with
 * the previous game. Basically prepare for a totally new game.  */
void init_game () 
{
    curr_player = WPLAYER;
    reset_board ();
}

/* Play a debugging/test game, controlling white and black. This is done via 
 * the command line, there is absolutely no GUI!  */
void play_test_game ()
{
    init_game ();

    while (game_over () == FALSE && strncmp ("quit", str_buff, 4) != 0) {
        print_board ();

        struct move mv;
        mv.start_pos = 0;
        mv.end_pos   = 0;

        /* Get user's move then parse it from coordinate notation into an array
        index. Loop until the move is valid.  */
        do {
            char pl = (curr_player == WPLAYER) ? 'W' : 'B';
            printf ("\nEnter %c move: ", pl);
            get_input ();

            if (strncmp ("quit", str_buff, 4) == 0) {
                break;
            }

            parse_move (&mv, FALSE);
        } while (make_move (curr_player, mv.start_pos, mv.end_pos) == FALSE);

        curr_player = opponent_player (curr_player);
    }

    if (game_over () == TRUE) {
        print_board ();
        printf ("Checkmate!\n");
    }
}

/* Play a test game controlling white vs the AI.  */
void play_ai_game ()
{
    fprintf (fp, "A: play_ai_game\n");
    init_game ();

    while (game_over () == FALSE && strncmp ("quit", str_buff, 4) != 0) { 
        print_board ();

        struct move mv;
        mv.start_pos = 0;
        mv.end_pos   = 0;

        /* Get user's move then parse it from coordinate notation into an array
        index. Loop until the move is valid.  */
        do {
            /* Get player's move or move from AI.  */
            if (curr_player == WPLAYER) {
                char pl = (curr_player == WPLAYER) ? 'W' : 'B';
                printf ("\nEnter %c move: ", pl);
                get_input ();

                if (strncmp ("quit", str_buff, 4) == 0) {
                    break;
                }
                parse_move (&mv, FALSE);
            } else {
                printf ("making AI's move\n");
                fprintf (fp, "A: best_move\n");
                best_move (&mv);
            }
        } while (make_move (curr_player, mv.start_pos, mv.end_pos) == FALSE);

        if (curr_player == BPLAYER) {
            unparse_move (&mv);
            printf ("move %s\n", str_buff);
        }

        curr_player = opponent_player (curr_player);
    }

    if (game_over () == TRUE) {
        print_board ();
        printf ("Checkmate!\n");
    }
}

/* Hook up with XBoard and play a game of chess :).  */
void play_game () 
{
    fprintf (fp, "A: play_game\n");
    extern int board[BOARD_SIZE];
    init_game ();

    while (game_over () == FALSE && strncmp ("quit", str_buff, 4) != 0) { 
        struct move mv;
        mv.start_pos = 0;
        mv.end_pos   = 0;

        /* Get user's move then parse it from coordinate notation into an array
         * index. Loop until the move is valid. AI moves when player is black,
         * parse move into coordinate notation and send to XBoard.  */
        do {
            /* Player's move. Take input from XBoard.  */
            if (curr_player == WPLAYER) {
                get_input ();

                if (strncmp ("quit", str_buff, 4) == 0) {
                    break;
                } else if (strncmp ("usermove ", str_buff, 9) == 0) { 
                    parse_move (&mv, TRUE);
                }
            }

            /* AI's move. Send info to AI and store his move in BEST_MOVE.  */
             else {
                fprintf (fp, "A: best_move\n");
                best_move (&mv);
            }
        } while (make_move (curr_player, mv.start_pos, mv.end_pos) == FALSE);

        /* Convert AI's move to coordinate notation and send move to XBoard.  */
        if (curr_player == BPLAYER) {
            unparse_move (&mv);
            printf ("move %s\n", str_buff);
            fprintf (fp, "A: sending \"move %s\"\n", str_buff);
        }

        /* To help with making a better evaluation function, print contents of
         * BOARD to log file.  */
        fprintf (fp, "\n\n * --- *\n");
        int i;
        for (i = 0; i < BOARD_SIZE; i++) {
            fprintf (fp, "%d", board[i]);
        }
        fprintf (fp, "\n * --- *\n\n");

        curr_player = opponent_player (curr_player);
    }
}

/* Run a dummy search. Nice to checking how long it takes to search to some
 * depth.  */
void search_test ()
{
    printf ("Beginning search test to depth %d...\n", SEARCH_DEP);
    init_game ();
    abp_search (WPLAYER, SEARCH_DEP, NEG_INF, POS_INF);
    printf ("End of search.\n");
}

/* Print material and position scores for a variety of test boards to learn more
 * about what the evaluation function is doing. Great for tuning.  */
void eval_test ()
{
    /* Board b2 has black's queen move into an insane position.  */
    char *b1 = "4235632400000000111100110000000000000100000000000000100-5000000000000-1000000000000000000000000000-1-1-1-10-1-1-100000000-4-2-30-6-3-2-400000000x";
    int c, i = 0, n = 0;
    while ((c = b1[i]) != 'x') {
        if (c == '-') {
            c = b1[++i];
            c = c - '0';
            c *= -1;
        } else {
            c = c - '0';
        }
        board[n++] = c;
        i++;
    }
    print_board ();
    printf ("material score: %d\npositional score: %d\n", material_score (),
        positional_score ());

    /* Board b2 has black considering losing it's knight to a pawn.  */
    char *b2 = "4235632400000000111101110000000000000000000000000000000000000000000000000000000000-20010000000000-1-1-1-1-1-1-1-100000000-40-3-5-6-30-400000000x";
    i = 0; n = 0;
    while ((c = b2[i]) != 'x') {
        if (c == '-') {
            c = b2[++i];
            c = c - '0';
            c *= -1;
        } else {
            c = c - '0';
        }
        board[n++] = c;
        i++;
    }
    print_board ();
    printf ("material score: %d\npositional score: %d\n",
        material_score () * MATERIAL_WT, positional_score () * POSITION_WT);

    /* Black moves to attack white's queen but loses bishop next move.  */
    char *b3 = "420060240000000010100111000000000101300000000000000000500000000003-2-100000000000000-20000000000000-1-1-10-1-1-1-100000000-400-5-6-30-400000000";
    i = 0; n = 0;
    while ((c = b3[i]) != 'x') {
        if (c == '-') {
            c = b3[++i];
            c = c - '0';
            c *= -1;
        } else {
            c = c - '0';
        }
        board[n++] = c;
        i++;
    }
    print_board ();
    printf ("material score: %d\npositional score: %d\n",
        material_score () * MATERIAL_WT, positional_score () * POSITION_WT);
}

/* Convert coordinate notation of a move from STR_BUF to array index for
 * MV->START_POS and MV->END_POS.  */
void parse_move (struct move *mv, int playing_xboard)
{
    if (playing_xboard == FALSE) {
        mv->start_pos = (str_buff[0] - 'a') + ((str_buff[1] - '1') * 16);
        mv->end_pos   = (str_buff[2] - 'a') + ((str_buff[3] - '1') * 16);
    } else {
        mv->start_pos = (str_buff[9] - 'a') + ((str_buff[10] - '1') * 16);
        mv->end_pos   = (str_buff[11] - 'a') + ((str_buff[12] - '1') * 16);
    }
    fprintf (fp, "A: parse_move to %d - %d\n", mv->start_pos, mv->end_pos);
}

/* Convert MV->START_POS and MV->END_POS to coordinate notation in STR_BUFF.  */
void unparse_move (struct move *mv)
{
    clean_buffer ();
    str_buff[0] = (mv->start_pos & 7) + 'a';
    str_buff[1] = (mv->start_pos >> 4) + '1';
    str_buff[2] = (mv->end_pos & 7) + 'a';
    str_buff[3] = (mv->end_pos >> 4) + '1';
    fprintf (fp, "A: unparse_move to %s\n", str_buff);
}

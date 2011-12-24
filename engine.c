#include <stdio.h>
#include <string.h>
#include "proto.h"

#define BUF_SIZE 128

FILE* fp;
char  str_buff[BUF_SIZE];
int   curr_player;

/* XBoard starts engine from here.  */
int main (int argc, char *argv[]) 
{
    /* XBoard suggests the following to fix buffering for I/O problems.  */
    setbuf (stdout, NULL);
    setbuf (stdin, NULL);

    /* The -c switch plays a command line game, good for debugging.  */
    if (argc >= 2 && strcmp (argv[1], "-c") == 0) {
        play_test_game ();
    } else if (argc >= 2) {
        printf ("Argument(s) not recognized.\n");
        printf ("\t-c play command line 2-player game\n");
        printf ("\tno arguments for regular XBoard game\n");
        return -1;
    }

    fp = fopen ("iolog.txt", "w");
    if (fp == NULL) {
        return -1;
    } else {
        while (strcmp ("quit", str_buff) != 0) { 
            get_input ();

            /* Catch XBoard's messages from stdin.  */
            if (strcmp ("new", str_buff) == 0) { 
                fprintf (fp, "R: new\nA: init_game()\nA: play_game()\n");
                init_game ();
                play_game ();
            } else if (strcmp ("protover 2", str_buff) == 0) { 
                fprintf (fp, "R: protover 2\nS: feature done=1\n");
                printf ("feature myname=jgnchess done=1\n");
            } else if (strcmp ("quit", str_buff) == 0) { 
                fprintf (fp, "R: quit\nA: quitting\n");
            } else
                fprintf (fp, "U: \"%s\"\n", str_buff);
        }
    }
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
        str_buff[i++] = ch;
    }
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
    int start_pos, end_pos;

    while (game_over () != TRUE && strcmp ("quit", str_buff) != 0) { 
        print_board ();

        /* Get user's move then parse it from coordinate notation into an array
        index. Loop until the move is valid.  */
        do {
            char pl = (curr_player == WPLAYER) ? 'W' : 'B';
            printf ("\nEnter %c move: ", pl);
            get_input ();
            parse_move (&start_pos, &end_pos);
        } while (make_move (curr_player, start_pos, end_pos) == FALSE
                 && strcmp ("quit", str_buff) != 0);
        curr_player = (curr_player == WPLAYER) ? BPLAYER : WPLAYER;
    }
}

/* Begin playing chess starting with white's (player's) move.
 * Either receive player's move from XBoard and check it's
 * legality or generate a move via the AI and send it to XBoard.
 * Continue playing until the game is over or one player quits.  */
void play_game () 
{
    init_game ();
    int start_pos, end_pos;

    while (game_over () != TRUE && strcmp ("quit", str_buff) != 0) {
        /* Get user's move then parse it from coordinate notation into an array
         * index. Loop until the move is valid.  */
        do {
            get_input ();
            parse_move (&start_pos, &end_pos);
            fprintf (fp, "R: %s\n", str_buff);
        } while (strcmp ("quit", str_buff) != 0
                 && make_move (curr_player, start_pos, end_pos) == FALSE);
    
        /* Record the player's move in the debugging file. */
        fprintf (fp, "M: P%d moves %d - %d\n", curr_player, start_pos, end_pos);
        curr_player = (curr_player == WPLAYER) ? BPLAYER : WPLAYER;
    }
}

/* Convert coordinate notation into an array index.  */
void parse_move (int *start, int *end)
{
    *start = (str_buff[0] - 'a') + ((str_buff[1] - '1') * 16);
    *end   = (str_buff[2] - 'a') + ((str_buff[3] - '1') * 16);
}

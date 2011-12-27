#define TRUE        1
#define FALSE       0

#define WPLAYER     0
#define BPLAYER     1

/* Using the 0x88 board representation, these values help compute the move
 * squares on the array. For example, moving up from square 51 should land you
 * in square 67, thus we add 16.  */
#define MOVE_NULL       -999
#define MOVE_UP         16
#define MOVE_RIGHT      1
#define MOVE_DOWN       -16
#define MOVE_LEFT       -1
#define MOVE_DU_RIGHT   17
#define MOVE_DD_RIGHT   -15
#define MOVE_DD_LEFT    -17
#define MOVE_DU_LEFT    15
#define MOVE_K_URV      33  
#define MOVE_K_URH      18
#define MOVE_K_DRH      -14 
#define MOVE_K_DRV      -31
#define MOVE_K_DLV      -33
#define MOVE_K_DLH      -18
#define MOVE_K_ULH      14
#define MOVE_K_ULV      31

/* Null piece is 0, white pieces range 1 to 6, black pieces from -1 to -6.  */
enum ch_piece { 
    chp_null = 0,

    chp_wpawn = 1,
    chp_wrook,
    chp_wknight,
    chp_wbishop,
    chp_wqueen,
    chp_wking,

    chp_bpawn   = -1,
    chp_brook   = -2,
    chp_bknight = -3,
    chp_bbishop = -4,
    chp_bqueen  = -5,
    chp_bking   = -6
};

/* Function prototypes for board.c.  */
void print_board ();
void reset_board ();
int  square_is_occupied (int);
int  valid_x88_move (int);
int  square_on_board (int);
int  contains_players_piece (int, int);
int  valid_start_pos (int, int);
int  valid_end_pos (int);
int  opponent_player (int);
void move_piece (int, int);
void unmove_piece (int, int);
void gen_legal_moves (int, int, int *); 
void gen_plegal_moves (int, int, int *);
void remove_check_moves (int, int, int *);
void gen_wpawn_moves (int, int *);
void gen_bpawn_moves (int, int *);
void gen_knight_moves (int, int, int *);
void gen_king_moves (int, int, int *);
void gen_rook_moves (int, int, int *);
void gen_bishop_moves (int, int, int *);
void gen_queen_moves (int, int, int *);
void gen_sliding_moves (int, int, int, int *);
int  make_move (int, int, int);
int  is_legal_move (int, int, int);
int  player_in_check (int);
int  player_check_by_rook (int, int, int *);
int  player_check_by_bishop (int, int, int *);
int  player_check_by_pawn (int, int);
int  player_check_by_knight (int, int, int *);
void init_moves_board (int *);
int  game_over ();

/* Function prototypes for engine.c.  */
void clean_buffer ();
void get_input ();
void init_game ();
void play_game ();
void play_test_game ();
void parse_move (int *, int *);

/* Function prototypes for ai.c.  */

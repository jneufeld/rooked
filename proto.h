#define TRUE        1
#define FALSE       0

#define WPLAYER     0
#define BPLAYER     1

#define NEG_INF     -30000
#define POS_INF     30000

#define SEARCH_DEP  3

#define BOARD_SIZE  128

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

/* Values of each piece during move evaluation.  */
#define PAWN_VAL    100
#define KNIGHT_VAL  300
#define BISHOP_VAL  300
#define ROOK_VAL    500
#define QUEEN_VAL   900
#define KING_VAL    10000

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

struct move {
    int start_pos;
    int end_pos;
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
int  move_piece (int, int);
void unmove_piece (int, int, int);
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
int  player_has_moves (int);
int  game_over ();

/* Function prototypes for engine.c.  */
void clean_buffer ();
void get_input ();
void init_game ();
void play_game ();
void play_test_game ();
void play_ai_game ();
void search_test ();
void parse_move (struct move *, int);
void unparse_move (struct move *);

/* Function prototypes for ai.c.  */
void best_move (struct move *);
int  abp_search (int, int, int, int);
int  material_score ();
int  positional_score ();
int  board_utility ();

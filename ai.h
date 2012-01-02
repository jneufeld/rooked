/* Values of each piece during move evaluation.  */
#define PAWN_VAL    100
#define KNIGHT_VAL  300
#define BISHOP_VAL  300
#define ROOK_VAL    500
#define QUEEN_VAL   900
#define KING_VAL    10000

/* Weights for material and positional evaluation.  */
#define MATERIAL_WT 3
#define POSITION_WT 2

#define NEG_INF     -30000
#define POS_INF     30000

#define SEARCH_DEP  2

struct move {
    int start_pos;
    int end_pos;
};

void best_move (struct move *);
int  abp_search (int, int, int, int);
int  board_utility ();
int  material_score ();
int  positional_score ();
int  knight_pos_score (int, int);

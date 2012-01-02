#define BUF_SIZE 128

void clean_buffer ();
void get_input ();
void init_game ();
void play_game ();
void play_test_game ();
void play_ai_game ();
void search_test ();
void eval_test ();
void parse_move (struct move *, int);
void unparse_move (struct move *);

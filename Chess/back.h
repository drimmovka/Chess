#ifndef BACK_H_INCLUDED
#define BACK_H_INCLUDED

#define mapH 8
#define mapW 8
#define N 100
#define EMPTY 0
#define PAWN 1
#define ROOK 2
#define HORSE 3
#define BISHOP 4
#define QUEEN 5
#define KING 6
/* 0 - W; 1 - B */

typedef struct {
    int x, y, col, type;
} Pos;


typedef struct {
    char rec;
    int x, y, col, type;
    Pos pos[N]; /* interaction positions */
} Fg;


extern Fg map[mapH][mapW];
extern int w_att[mapH][mapW];
extern int b_att[mapH][mapW];

extern int turn, move_number;

extern int white_king_under_check, black_king_under_check;

extern int pawn_turning, turning_i, turning_j;

extern int figure_taken;

void show(int i1, int j1, int i2, int j2);
void game_start();
void generate_positions();
int move_to(int i1, int j1, int i2, int j2);
void change_condition(int i, int j);
int init_pawn_turning(int x, int y);
int check_move(Fg fg, int px, int py);
int check_check(int col, Fg (*used_map)[mapW], int (*used_w_att)[mapW], int (*used_b_att)[mapW]);
int check_check_mate();
int check_stalemate();
int check_move_result();
void move_figure(Fg (*used_map)[mapW], int i1, int j1, int i2, int j2);
#endif // BACK_H_INCLUDED

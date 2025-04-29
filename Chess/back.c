#include <stdio.h>
#include <stdlib.h>

#include "back.h"

Fg map[mapH][mapW];
int w_att[mapH][mapW];
int b_att[mapH][mapW];

Fg map_copy[mapH][mapW];
int w_att_copy[mapH][mapW];
int b_att_copy[mapH][mapW];

int turn = 0, move_number = 0;

int white_king_under_check = 0, black_king_under_check = 0;

int lw_rook_moved = 0, rw_rook_moved = 0, w_king_moved = 0;
int lb_rook_moved = 0, rb_rook_moved = 0, b_king_moved = 0;

int pawn_turning = 0, turning_i = -1, turning_j = -1;

int figure_taken = 0;

int w_taking_on_pass_j = -1; b_taking_on_pass_j = -1;

char sym[] = "abcdefgh";

void show(int i1, int j1, int i2, int j2) {
    printf("Move Number: %d | Player: %s | Move done: ", move_number, move_number == 0 ? "-" : turn == 0 ? "W" : "B");
    if (i1 != -1 && j1 != -1 && i2 != -1 && j2 != -1) {
        printf("%c%d%c%d\n", sym[j1], 8-i1, sym[j2], 8-i2);
    } else {
        printf("----\n");
    }
    for (int i=0; i<mapH; ++i) {
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("  %d | ", mapH-i);
        for (int j=0; j<mapW; ++j) {
            printf("%c | ", map[i][j].rec);
        }
        printf("\n");
    }
    printf("    +---+---+---+---+---+---+---+---+\n");
    printf("      A   B   C   D   E   F   G   H\n\n");
}

int check_in(int axis) {
    return (axis >= 0 && axis <= 7);
}

void pawn_move(Fg *fg, Fg (*used_map)[mapW]) {
    int k = fg->col * 2 - 1; /*k of movement for white and black pieces (-1) for W and (+1) for B*/

    if (fg->x == 6 || fg->x == 1) { /*move on two cells*/
        if (check_in(fg->x+2*k)) {
            if (used_map[fg->x+k][fg->y].col == 2 && used_map[fg->x+2*k][fg->y].col == 2) {
                fg->pos[0].x = fg->x+2*k;
                fg->pos[0].y = fg->y;
                fg->pos[0].col = used_map[fg->x+2*k][fg->y].col;
            }
        }
    }

    if (check_in(fg->x+k)) { /*move on one cell*/
        if (used_map[fg->x+k][fg->y].col == 2) {
            fg->pos[1].x = fg->x+k;
            fg->pos[1].y = fg->y;
            fg->pos[1].col = used_map[fg->x+k][fg->y].col;
        }
        /*enemy capture (left or right)*/
        if (check_in(fg->y-1)) {
            fg->pos[2].x = fg->x+k;
            fg->pos[2].y = fg->y-1;
            fg->pos[2].col = used_map[fg->x+k][fg->y-1].col;
        }
        if (check_in(fg->y+1)) {
            fg->pos[3].x = fg->x+k;
            fg->pos[3].y = fg->y+1;
            fg->pos[3].col = used_map[fg->x+k][fg->y+1].col;
        }
    }

    /* taking on the pass */
    if (fg->col == 0 && fg->x == 3 && b_taking_on_pass_j != -1 && abs(fg->y - b_taking_on_pass_j) == 1) {
        fg->pos[4].x = 2;
        fg->pos[4].y = b_taking_on_pass_j;
        fg->pos[4].col = 1;
    }
    if (fg->col == 1 && fg->x == 4 && w_taking_on_pass_j != -1 && abs(fg->y - w_taking_on_pass_j) == 1) {
        fg->pos[4].x = 5;
        fg->pos[4].y = w_taking_on_pass_j;
        fg->pos[4].col = 0;
    }
}

void rook_move(Fg *fg, Fg (*used_map)[mapW]) {
    int q = 0, k;

    /* up */
    k = 1;
    while (1) {
        if (!check_in(fg->x - k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y;
        fg->pos[q].col = used_map[fg->x-k][fg->y].col;
        ++q;
        if (used_map[fg->x - k][fg->y].col != 2) break;
        ++k;
    }
    /* down */
    k = 1;
    while (1) {
        if (!check_in(fg->x + k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y;
        fg->pos[q].col = used_map[fg->x+k][fg->y].col;
        ++q;
        if (used_map[fg->x + k][fg->y].col != 2) break;
        ++k;
    }
    /* left */
    k = 1;
    while (1) {
        if (!check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x][fg->y-k].col;
        ++q;
        if (used_map[fg->x][fg->y - k].col != 2) break;
        ++k;
    }
    /* right */
    k = 1;
    while (1) {
        if (!check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x][fg->y+k].col;
        ++q;
        if (used_map[fg->x][fg->y + k].col != 2) break;
        ++k;
    }
}

void horse_move(Fg *fg, Fg (*used_map)[mapW]) {
    int q = 0;
    for (int k=-2; k<3; ++k) {
        for (int p=-2; p<3; ++p) {
            if (k != 0 && p != 0 && (abs(k) != abs(p))) {
                if (check_in(fg->x+k) && check_in(fg->y+p)) {
                    fg->pos[q].x = fg->x+k;
                    fg->pos[q].y = fg->y+p;
                    fg->pos[q].col = used_map[fg->x+k][fg->y+p].col;
                }
                ++q;
            }
        }
    }
}

void bishop_move(Fg *fg, Fg (*used_map)[mapW]) {
    int q = 0, k;

    /*left up*/
    k = 1;
    while (1) {
        if (!check_in(fg->x - k) || !check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x-k][fg->y-k].col;
        ++q;
        if (used_map[fg->x - k][fg->y - k].col != 2) break;
        ++k;
    }
    /*right up*/
    k = 1;
    while (1) {
        if (!check_in(fg->x - k) || !check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x-k][fg->y+k].col;
        ++q;
        if (used_map[fg->x - k][fg->y + k].col != 2) break;
        ++k;
    }
    /*left down*/
    k = 1;
    while (1) {
        if (!check_in(fg->x + k) || !check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x+k][fg->y-k].col;
        ++q;
        if (used_map[fg->x + k][fg->y - k].col != 2) break;
        ++k;
    }
    /*right down*/
    k = 1;
    while (1) {
        if (!check_in(fg->x + k) || !check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x+k][fg->y+k].col;
        ++q;
        if (used_map[fg->x + k][fg->y + k].col != 2) break;
        ++k;
    }
}

void queen_move(Fg *fg, Fg (*used_map)[mapW]) {
    int q = 0, k;

    /* up */
    k = 1;
    while (1) {
        if (!check_in(fg->x - k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y;
        fg->pos[q].col = used_map[fg->x-k][fg->y].col;
        ++q;
        if (used_map[fg->x - k][fg->y].col != 2) break;
        ++k;
    }
    /* down */
    k = 1;
    while (1) {
        if (!check_in(fg->x + k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y;
        fg->pos[q].col = used_map[fg->x+k][fg->y].col;
        ++q;
        if (used_map[fg->x + k][fg->y].col != 2) break;
        ++k;
    }
    /* left */
    k = 1;
    while (1) {
        if (!check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x][fg->y-k].col;
        ++q;
        if (used_map[fg->x][fg->y - k].col != 2) break;
        ++k;
    }
    /* right */
    k = 1;
    while (1) {
        if (!check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x][fg->y+k].col;
        ++q;
        if (used_map[fg->x][fg->y + k].col != 2) break;
        ++k;
    }
    /*left up*/
    k = 1;
    while (1) {
        if (!check_in(fg->x - k) || !check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x-k][fg->y-k].col;
        ++q;
        if (used_map[fg->x - k][fg->y - k].col != 2) break;
        ++k;
    }
    /*right up*/
    k = 1;
    while (1) {
        if (!check_in(fg->x - k) || !check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x - k;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x-k][fg->y+k].col;
        ++q;
        if (used_map[fg->x - k][fg->y + k].col != 2) break;
        ++k;
    }
    /*left down*/
    k = 1;
    while (1) {
        if (!check_in(fg->x + k) || !check_in(fg->y - k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y - k;
        fg->pos[q].col = used_map[fg->x+k][fg->y-k].col;
        ++q;
        if (used_map[fg->x + k][fg->y - k].col != 2) break;
        ++k;
    }
    /*right down*/
    k = 1;
    while (1) {
        if (!check_in(fg->x + k) || !check_in(fg->y + k)) break;
        fg->pos[q].x = fg->x + k;
        fg->pos[q].y = fg->y + k;
        fg->pos[q].col = used_map[fg->x+k][fg->y+k].col;
        ++q;
        if (used_map[fg->x + k][fg->y + k].col != 2) break;
        ++k;
    }
}

void king_move(Fg *fg, Fg (*used_map)[mapW]) {
    int q = 0;
    for (int k=-1; k<2; ++k) {
        for (int p=-1; p<2; ++p) {
            if (k != 0 || p != 0) {
                if (check_in(fg->x+k) && check_in(fg->y+p)) {
                    fg->pos[q].x = fg->x+k; fg->pos[q].y = fg->y+p; fg->pos[q].col = used_map[fg->x+k][fg->y+p].col;
                }
                ++q;
            }
        }
    }
}

void get_positions(Fg *fg, Fg (*used_map)[mapW]) {
    if (fg->type == PAWN)
        pawn_move(fg, used_map);
    if (fg->type == ROOK)
        rook_move(fg, used_map);
    if (fg->type == HORSE)
        horse_move(fg, used_map);
    if (fg->type == BISHOP)
        bishop_move(fg, used_map);
    if (fg->type == QUEEN)
        queen_move(fg, used_map);
    if (fg->type == KING)
        king_move(fg, used_map);
}

void clear_moves(Fg (*used_map)[mapW]) {
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            for (int q=0; q<N; ++q) {
                used_map[i][j].pos[q].x = -1;
                used_map[i][j].pos[q].y = -1;
                used_map[i][j].pos[q].col = -1;
                used_map[i][j].pos[q].type = -1;
            }
        }
    }
}

void get_attacked_positions(Fg (*used_map)[mapW], int (*used_w_att)[mapW], int (*used_b_att)[mapW]) {
    /* clear attack_map */
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            used_w_att[i][j] = 0;
            used_b_att[i][j] = 0;
        }
    }
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            if (used_map[i][j].type == PAWN) {
                for (int q=0; q<N; ++q) {
                    if (used_map[i][j].pos[q].col != -1 && used_map[i][j].pos[q].y != used_map[i][j].y) {
                        if (used_map[i][j].col == 0)
                            used_w_att[used_map[i][j].pos[q].x][used_map[i][j].pos[q].y] = 1; /* attacked by W */
                        if (used_map[i][j].col == 1)
                            used_b_att[used_map[i][j].pos[q].x][used_map[i][j].pos[q].y] = 1; /* attacked by B */
                    }
                }
            } else {
                for (int q=0; q<N; ++q) {
                    if (used_map[i][j].pos[q].col != -1) {
                        if (used_map[i][j].col == 0)
                            used_w_att[used_map[i][j].pos[q].x][used_map[i][j].pos[q].y] = 1; /* attacked by W */
                        if (used_map[i][j].col == 1)
                            used_b_att[used_map[i][j].pos[q].x][used_map[i][j].pos[q].y] = 1; /* attacked by B */
                    }
                }
            }
        }
    }
}

int check_check(int col, Fg (*used_map)[mapW], int (*used_w_att)[mapW], int (*used_b_att)[mapW]) {
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            if (used_map[i][j].type == KING && used_map[i][j].col == col) {
                if (col == 0 && used_b_att[i][j] == 1) return 1;
                if (col == 1 && used_w_att[i][j] == 1) return 1;
            }
        }
    }
    return 0;
}

int check_check_mate() {
    int flag = 0;
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            if (map[i][j].col == turn) {
                for (int q=0; q<N; ++q) {
                    if (map[i][j].pos[q].col != -1) flag = 1;
                }
            }
        }
    }
    return check_check(turn, &map, &w_att, &b_att) && !flag;
}

int check_stalemate() {
    int flag = 0;
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            if (map[i][j].col == turn) {
                for (int q=0; q<N; ++q) {
                    if (map[i][j].pos[q].col != -1) flag = 1;
                }
            }
        }
    }
    return !check_check(turn, &map, &w_att, &b_att) && !flag;
}

int check_move_result() {
    if (check_check(turn, &map, &w_att, &b_att)) {
        if (turn == 0) white_king_under_check = 1;
        if (turn == 1) black_king_under_check = 1;
    } else {
        white_king_under_check = 0;
        black_king_under_check = 0;
    }
    if (check_check_mate()) {
        printf("Winner is %s\n", 1-turn == 0 ? "W" : "B");
        return 1;
    }
    if (check_stalemate()) {
        printf("Draw\n");
        return 1;
    }
    return 0;
}

int check_move(Fg fg, int px, int py) {
    if (fg.col == map[px][py].col || map[px][py].type == KING) return 0;

    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            map_copy[i][j] = map[i][j];
            w_att_copy[i][j] = 0;
            b_att_copy[i][j] = 0;
        }
    }

    move_figure(&map_copy, fg.x, fg.y, px, py);

    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            get_positions(&map_copy[i][j], &map_copy);
        }
    }
    get_attacked_positions(&map_copy, &w_att_copy, &b_att_copy);

    return !check_check(fg.col, &map_copy, &w_att_copy, &b_att_copy);
}

void check_castling() {

    if (w_king_moved == 0 && b_att[7][4] == 0) {
        if (lw_rook_moved == 0 && b_att[7][2] == 0 && b_att[7][3] == 0
            && map[7][1].type == EMPTY && map[7][2].type == EMPTY && map[7][3].type == EMPTY) {
                map[7][4].pos[8].x = 7;
                map[7][4].pos[8].y = 2;
                map[7][4].pos[8].col = map[7][2].col;
                map[7][4].pos[8].type = map[7][2].type;
            }
        if (rw_rook_moved == 0 && b_att[7][5] == 0 && b_att[7][6] == 0
            && map[7][5].type == EMPTY && map[7][6].type == EMPTY) {
                map[7][4].pos[9].x = 7;
                map[7][4].pos[9].y = 6;
                map[7][4].pos[9].col = map[7][6].col;
                map[7][4].pos[9].type = map[7][6].type;
            }
    }

    if (b_king_moved == 0 && w_att[0][4] == 0) {
        if (lb_rook_moved == 0 && w_att[0][2] == 0 && w_att[0][3] == 0
            && map[0][1].type == EMPTY && map[0][2].type == EMPTY && map[0][3].type == EMPTY) {
                map[0][4].pos[8].x = 0;
                map[0][4].pos[8].y = 2;
                map[0][4].pos[8].col = map[0][2].col;
                map[0][4].pos[8].type = map[0][2].type;
            }
        if (rb_rook_moved == 0 && w_att[0][5] == 0 && w_att[0][6] == 0
            && map[0][5].type == EMPTY && map[0][6].type == EMPTY) {
                map[0][4].pos[9].x = 0;
                map[0][4].pos[9].y = 6;
                map[0][4].pos[9].col = map[0][6].col;
                map[0][4].pos[9].type = map[0][6].type;
            }
    }
}

void generate_positions() {
    clear_moves(&map);
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            get_positions(&map[i][j], &map);
        }
    }
    get_attacked_positions(&map, &w_att, &b_att);

    check_castling();

    /* deleting impossible positions */
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            for (int q=0; q<N; ++q) {
                if (map[i][j].type == PAWN) {
                    if (map[i][j].pos[q].col != -1 && map[i][j].pos[q].y != map[i][j].y) {
                        if (map[map[i][j].pos[q].x][map[i][j].pos[q].y].col != 1 - map[i][j].col && q != 4) {
                            map[i][j].pos[q].x = -1;
                            map[i][j].pos[q].y = -1;
                            map[i][j].pos[q].col = -1;
                            map[i][j].pos[q].type = -1;
                        }
                    }
                }
                if (map[i][j].pos[q].col != -1) {
                    if (!(map[i][j].type == KING && (q == 8 || q == 9))) {
                        if (!check_move(map[i][j], map[i][j].pos[q].x, map[i][j].pos[q].y)) {
                            map[i][j].pos[q].x = -1;
                            map[i][j].pos[q].y = -1;
                            map[i][j].pos[q].col = -1;
                            map[i][j].pos[q].type = -1;
                        }
                    }
                }
            }
        }
    }
}

int check_possibility(Pos (*pos), int i2, int j2) {
    int flag = 0;
    for (int q=0; q<N; ++q) {
        if (i2 == pos[q].x && j2 == pos[q].y) {
            flag = 1;
        }
    }
    return flag;
}

int move_to(int i1, int j1, int i2, int j2) {
    if (!(check_in(i1) && check_in(j1) && check_in(i2) && check_in(j2))) return 0;
    if (map[i1][j1].col == turn) {
        if (check_possibility(&map[i1][j1].pos, i2, j2)) {
            change_condition(i1, j1); change_condition(i2, j2);

            if (map[i2][j2].col == 1 - map[i1][j1].col)
                figure_taken = 1;

            move_figure(&map, i1, j1, i2, j2);

            w_taking_on_pass_j = -1; b_taking_on_pass_j = -1;
            if (map[i2][j2].type == PAWN && i1 == 6 && i2 == 4) { /* */
                w_taking_on_pass_j = j2;
            }
            if (map[i2][j2].type == PAWN && i1 == 1 && i2 == 3) { /* */
                b_taking_on_pass_j = j2;
            }


            if (map[i2][j2].type == PAWN && (i2 == 0 || i2 == 7)) { /* turning */
                pawn_turning = 1;
                turning_i = i2;
                turning_j = j2;
            }
            if (map[i2][j2].type == KING) { /* castling */
                if (j2 == 2 && abs(j1 - j2) == 2) {
                    change_condition(i1, 0);
                    move_figure(&map, i1, 0, i1, 3);
                }
                if (j2 == 6 && abs(j1 - j2) == 2) {
                    change_condition(i1, 7);
                    move_figure(&map, i1, 7, i1, 5);
                }
            }

            return 1;
        }
    }
    return 0;
}

void change_condition(int i, int j) {
    if (i == 7 && j == 4) w_king_moved = 1;
    if (i == 0 && j == 4) b_king_moved = 1;

    if (i == 7 && j == 0) lw_rook_moved = 1;
    if (i == 7 && j == 7) rw_rook_moved = 1;

    if (i == 0 && j == 0) lb_rook_moved = 1;
    if (i == 0 && j == 7) rb_rook_moved = 1;
}

int init_pawn_turning(int x, int y) {
    if (y == turning_j) {
        if (1 - turn == 0 && x == 0) {
            map[0][y].type = QUEEN;
            map[0][y].rec = 'Q';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 0 && x == 1) {
            map[0][y].type = HORSE;
            map[0][y].rec = 'H';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 0 && x == 2) {
            map[0][y].type = ROOK;
            map[0][y].rec = 'R';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 0 && x == 3) {
            map[0][y].type = BISHOP;
            map[0][y].rec = 'B';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }


        if (1 - turn == 1 && x == 7) {
            map[7][y].type = QUEEN;
            map[7][y].rec = 'q';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 1 && x == 6) {
            map[7][y].type = HORSE;
            map[7][y].rec = 'h';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 1 && x == 5) {
            map[7][y].type = ROOK;
            map[7][y].rec = 'r';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
        if (1 - turn == 1 && x == 4) {
            map[7][y].type = BISHOP;
            map[7][y].rec = 'b';
            pawn_turning = 0, turning_i = -1, turning_j = -1;
            return 1;
        }
    }
    return 0;
}

void move_figure(Fg (*used_map)[mapW], int i1, int j1, int i2, int j2) {
    if (used_map[i1][j1].type == PAWN && j2 != j1 && used_map[i2][j2].type == EMPTY) { /*taking on the pass*/
        used_map[i2][j2].type = used_map[i1][j1].type;
        used_map[i2][j2].col = used_map[i1][j1].col;
        used_map[i2][j2].rec = used_map[i1][j1].rec;
        used_map[i1][j1].type = EMPTY;
        used_map[i1][j1].col = 2;
        used_map[i1][j1].rec = ' ';

        used_map[i1][j2].type = EMPTY;
        used_map[i1][j2].col = 2;
        used_map[i1][j2].rec = ' ';
    } else {
                used_map[i2][j2].type = used_map[i1][j1].type;
        used_map[i2][j2].col = used_map[i1][j1].col;
        used_map[i2][j2].rec = used_map[i1][j1].rec;
        used_map[i1][j1].type = EMPTY;
        used_map[i1][j1].col = 2;
        used_map[i1][j1].rec = ' ';
    }
}

void game_start() {
    int default_type_placement[64] = {2,3,4,5,6,4,3,2,1,1,1,1,1,1,1,1,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                1,1,1,1,1,1,1,1,2,3,4,5,6,4,3,2};
    int default_col[64] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
                            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    char default_rec_placement[65] = "rhbqkbhrpppppppp                                PPPPPPPPRHBQKBHR";
    int k = 0;
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].col = default_col[k];
            map[i][j].type = default_type_placement[k];
            map[i][j].rec = default_rec_placement[k];


            for (int q=0; q<N; ++q) {
                map[i][j].pos[q].x = -1;
                map[i][j].pos[q].y = -1;
                map[i][j].pos[q].col = -1;
                map[i][j].pos[q].type = -1;
            }
            ++k;
        }
    }

    generate_positions();
    show(-1, -1, -1, -1);
    ++move_number;
}

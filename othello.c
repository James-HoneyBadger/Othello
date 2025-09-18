/* othello.c – core game engine */
#include "othello.h"
#include <stdlib.h>

int board[BOARD_SZ][BOARD_SZ];
int cursor_x = 3, cursor_y = 3;
int current_player = BLACK;          /* human is BLACK, computer is WHITE */

/* direction vectors (8 neighbours) */
static const int8_t dir[8][2] = {
    { -1, -1 }, {  0, -1 }, {  1, -1 },
    { -1,  0 },           {  1,  0 },
    { -1,  1 }, {  0,  1 }, {  1,  1 }
};

/* -----------------------------------------------------------------
   Initialise the board (standard Othello start position)
   ----------------------------------------------------------------- */
void othello_init(void)
{
    for (int y = 0; y < BOARD_SZ; ++y)
        for (int x = 0; x < BOARD_SZ; ++x)
            board[y][x] = EMPTY;

    board[3][3] = WHITE;
    board[4][4] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;

    cursor_x = 3; cursor_y = 3;
    current_player = BLACK;
}

/* -----------------------------------------------------------------
   Test whether a move at (bx,by) by *player* would flip at least
   one opponent disc.  Returns the number of flipped discs (0 = illegal).
   ----------------------------------------------------------------- */
static int count_flips(int bx, int by, int player)
{
    if (board[by][bx] != EMPTY) return 0;
    int opp = -player;
    int total = 0;

    for (int d = 0; d < 8; ++d) {
        int x = bx + dir[d][0];
        int y = by + dir[d][1];
        int cnt = 0;
        while (x >= 0 && x < BOARD_SZ && y >= 0 && y < BOARD_SZ && board[y][x] == opp) {
            ++cnt;
            x += dir[d][0];
            y += dir[d][1];
        }
        if (cnt && x >= 0 && x < BOARD_SZ && y >= 0 && y < BOARD_SZ && board[y][x] == player)
            total += cnt;
    }
    return total;
}

/* -----------------------------------------------------------------
   Perform the flip for a legal move.  Assumes the move is legal.
   ----------------------------------------------------------------- */
static void do_flip(int bx, int by, int player)
{
    int opp = -player;
    board[by][bx] = player;

    for (int d = 0; d < 8; ++d) {
        int x = bx + dir[d][0];
        int y = by + dir[d][1];
        int cnt = 0;
        while (x >= 0 && x < BOARD_SZ && y >= 0 && y < BOARD_SZ && board[y][x] == opp) {
            ++cnt;
            x += dir[d][0];
            y += dir[d][1];
        }
        if (cnt && x >= 0 && x < BOARD_SZ && y >= 0 && y < BOARD_SZ && board[y][x] == player) {
            /* walk back and flip */
            int fx = bx + dir[d][0];
            int fy = by + dir[d][1];
            while (fx != x || fy != y) {
                board[fy][fx] = player;
                fx += dir[d][0];
                fy += dir[d][1];
            }
        }
    }
}

/* -----------------------------------------------------------------
   Public wrapper: attempt a move for *player* at board coords (bx,by).
   Returns true if the move was legal and performed.
   ----------------------------------------------------------------- */
bool othello_move(int bx, int by, int player)
{
    if (count_flips(bx, by, player) == 0) return false;
    do_flip(bx, by, player);
    return true;
}

/* -----------------------------------------------------------------
   Does *player* have at least one legal move?
   ----------------------------------------------------------------- */
bool othello_has_move(int player)
{
    for (int y = 0; y < BOARD_SZ; ++y)
        for (int x = 0; x < BOARD_SZ; ++x)
            if (count_flips(x, y, player) > 0)
                return true;
    return false;
}

/* -----------------------------------------------------------------
   Is the whole game over? (no legal moves for either side)
   ----------------------------------------------------------------- */
bool othello_game_over(void)
{
    return !othello_has_move(BLACK) && !othello_has_move(WHITE);
}

/* -----------------------------------------------------------------
   Count discs for each colour.
   ----------------------------------------------------------------- */
void othello_score(int *black, int *white)
{
    int b = 0, w = 0;
    for (int y = 0; y < BOARD_SZ; ++y)
        for (int x = 0; x < BOARD_SZ; ++x) {
            if (board[y][x] == BLACK) ++b;
            else if (board[y][x] == WHITE) ++w;
        }
    *black = b; *white = w;
}

/* -----------------------------------------------------------------
   Very simple AI – choose a random legal move.
   ----------------------------------------------------------------- */
void othello_computer_turn(void)
{
    int legal[64][2];
    int n = 0;
    for (int y = 0; y < BOARD_SZ; ++y)
        for (int x = 0; x < BOARD_SZ; ++x)
            if (count_flips(x, y, WHITE) > 0) {
                legal[n][0] = x; legal[n][1] = y; ++n;
            }

    if (n == 0) return;           /* should never happen – caller checks */
    int idx = rand() % n;
    othello_move(legal[idx][0], legal[idx][1], WHITE);
}

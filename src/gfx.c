/* gfx.c – very small 2‑bit graphics library for the Picocomputer‑6502 */
#include "othello.h"
#include <string.h>

/* --------------------------------------------------------------
   Set a single pixel (2‑bit colour 0‑3) at (x,y)
   -------------------------------------------------------------- */
void set_pixel2(int x, int y, uint8_t colour)
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) return;

    int byte = (y * FB_STRIDE) + (x >> 2);          /* 4 pixels per byte   */
    int shift = (3 - (x & 3)) * 2;                /* bit position 6‑7 …  */
    uint8_t mask = 0x03 << shift;

    volatile uint8_t *p0 = FB_PLANE0 + byte;
    volatile uint8_t *p1 = FB_PLANE1 + byte;

    uint8_t b0 = *p0 & ~mask;
    uint8_t b1 = *p1 & ~mask;

    /* colour bits → plane0 = LSB, plane1 = MSB */
    if (colour & 0x01) b0 |= (0x01 << shift);
    if (colour & 0x02) b1 |= (0x02 << shift);

    *p0 = b0;
    *p1 = b1;
}

/* --------------------------------------------------------------
   Fill a solid rectangle with a colour (used for board squares).
   -------------------------------------------------------------- */
void gfx_fill_rect(int x, int y, int w, int h, uint8_t colour)
{
    for (int yy = y; yy < y + h; ++yy)
        for (int xx = x; xx < x + w; ++xx)
            set_pixel2(xx, yy, colour);
}

/* --------------------------------------------------------------
   Initialise the video RAM (clear to colour 0 = black).
   -------------------------------------------------------------- */
void gfx_init(void)
{
    memset((void*)FB_PLANE0, 0, FB_STRIDE * FB_HEIGHT);
    memset((void*)FB_PLANE1, 0, FB_STRIDE * FB_HEIGHT);
}

/* --------------------------------------------------------------
   Draw the whole 8×8 board.
   Each square is 24×24 pixels → board occupies 192×192 pixels,
   centred in the 256×240 screen.
   -------------------------------------------------------------- */
void gfx_draw_board(void)
{
    const int sq = 24;                     /* square size in pixels */
    const int offx = (FB_WIDTH  - BOARD_SZ * sq) / 2;
    const int offy = (FB_HEIGHT - BOARD_SZ * sq) / 2 + 12; /* leave room for status bar */

    for (int by = 0; by < BOARD_SZ; ++by) {
        for (int bx = 0; bx < BOARD_SZ; ++bx) {
            int colour = ((bx + by) & 1) ? 2 : 1;   /* alternating green shades */
            int x0 = offx + bx * sq;
            int y0 = offy + by * sq;
            gfx_fill_rect(x0, y0, sq, sq, colour);
        }
    }

    /* draw the discs that are already on the board */
    extern int board[BOARD_SZ][BOARD_SZ];
    for (int by = 0; by < BOARD_SZ; ++by) {
        for (int bx = 0; bx < BOARD_SZ; ++bx) {
            if (board[by][bx] != EMPTY) {
                int colour = 3;                 /* white pixel for both players */
                int x0 = offx + bx * sq;
                int y0 = offy + by * sq;

                /* a very simple “disc” – a filled 16×16 square centred */
                int pad = 4;
                gfx_fill_rect(x0 + pad, y0 + pad, sq - 2*pad, sq - 2*pad, colour);
            }
        }
    }
}

/* --------------------------------------------------------------
   Draw the cursor (a thin white rectangle around the selected square)
   -------------------------------------------------------------- */
void gfx_draw_cursor(int bx, int by)
{
    const int sq = 24;
    const int offx = (FB_WIDTH  - BOARD_SZ * sq) / 2;
    const int offy = (FB_HEIGHT - BOARD_SZ * sq) / 2 + 12;

    int x0 = offx + bx * sq;
    int y0 = offy + by * sq;

    /* top and bottom lines */
    for (int i = 0; i < sq; ++i) {
        set_pixel2(x0 + i, y0, 3);
        set_pixel2(x0 + i, y0 + sq - 1, 3);
    }
    /* left and right lines */
    for (int i = 0; i < sq; ++i) {
        set_pixel2(x0, y0 + i, 3);
        set_pixel2(x0 + sq - 1, y0 + i, 3);
    }
}

/* --------------------------------------------------------------
   Draw the status bar (player turn + scores) at the top of the screen.
   For simplicity we just write text using a very small 4×6 pixel font
   that is hard‑coded in this file.
   -------------------------------------------------------------- */
static const uint8_t font4x6[96][6] = {
    /*   0x20 (space) … 0x7F (~) – each character is 4×6 bits */
    /* ... a tiny subset is filled in; you can expand it if you wish ... */
    /* '0'..'9' */
    [16] = {0x0E,0x11,0x13,0x15,0x19,0x0E}, /* 0 */
    [17] = {0x04,0x0C,0x04,0x04,0x04,0x0E}, /* 1 */
    [18] = {0x0E,0x11,0x01,0x0E,0x10,0x1F}, /* 2 */
    [19] = {0x1F,0x02,0x04,0x02,0x01,0x1E}, /* 3 */
    [20] = {0x02,0x06,0x0A,0x12,0x1F,0x02}, /* 4 */
    [21] = {0x1F,0x10,0x1E,0x01,0x01,0x1E}, /* 5 */
    [22] = {0x07,0x08,0x1E,0x11,0x11,0x0E}, /* 6 */
    [23] = {0x1F,0x01,0x02,0x04,0x08,0x08}, /* 7 */
    [24] = {0x0E,0x11,0x0E,0x11,0x11,0x0E}, /* 8 */
    [25] = {0x0E,0x11,0x11,0x0F,0x01,0x0E}, /* 9 */
    /* ':' and space */
    [58-32] = {0x00,0x06,0x06,0x00,0x06,0x06},
    [32-32] = {0x00,0x00,0x00,0x00,0x00,0x00}
};

/* helper: draw a single 4×6 char at (x,y) in colour 3 (white) */
static void draw_char4x6(int x, int y, char c)
{
    if (c < 0x20) return;
    const uint8_t *g = font4x6[c - 0x20];
    for (int row = 0; row < 6; ++row) {
        uint8_t bits = g[row];
        for (int col = 0; col < 4; ++col) {
            if (bits & (0x08 >> col))
                set_pixel2(x + col, y + row, 3);
        }
    }
}

/* helper: draw a null‑terminated string */
static void draw_string(int x, int y, const char *s)
{
    while (*s) {
        draw_char4x6(x, y, *s);
        x += 5;      /* 4 pixels + 1 pixel gap */
        ++s;
    }
}

/* draw the status line (player turn + scores) */
void gfx_draw_status(void)
{
    int black, white;
    othello_score(&black, &white);
    char buf[32];
    const char *turn = (current_player == BLACK) ? "YOU" : "CPU";
    snprintf(buf, sizeof(buf), "TURN:%s  B:%d  W:%d", turn, black, white);
    draw_string(4, 2, buf);
}

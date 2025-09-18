/* othello.h – shared structures for the Picocomputer‑6502 Othello demo */
#ifndef OTHELLO_H
#define OTHELLO_H

#include <stdint.h>
#include <stdbool.h>

/* --------------------------------------------------------------
   2‑bit video RAM (Picocomputer‑6502)
   -------------------------------------------------------------- */
#define FB_PLANE0   ((volatile uint8_t*)0xF000)   /* bit‑plane 0 */
#define FB_PLANE1   ((volatile uint8_t*)0xF200)   /* bit‑plane 1 */
#define FB_WIDTH    256
#define FB_HEIGHT   240
#define FB_STRIDE   (FB_WIDTH / 4)               /* 4 pixels per byte */

/* --------------------------------------------------------------
   Input port – same as in the Pac‑Man demo
   -------------------------------------------------------------- */
#define INPUT_REG   (*(volatile uint8_t*)0xD010)
#define IN_LEFT   0x01
#define IN_RIGHT  0x02
#define IN_UP     0x04
#define IN_DOWN   0x08
#define IN_FIRE   0x10      /* SPACE bar */

/* --------------------------------------------------------------
   Board representation
   -------------------------------------------------------------- */
#define BOARD_SZ 8

/* board cell values */
#define EMPTY   0
#define BLACK   1      /* player   – colour 3 (white pixel) */
#define WHITE  -1      /* computer – colour 3 (white pixel) */

/* --------------------------------------------------------------
   Public API (used by main.c)
   -------------------------------------------------------------- */
void gfx_init(void);
void gfx_draw_board(void);
void gfx_draw_cursor(int bx, int by);
void gfx_draw_status(void);
void gfx_fill_rect(int x, int y, int w, int h, uint8_t colour);
void set_pixel2(int x, int y, uint8_t colour);

uint8_t input_poll(void);

/* Othello core */
void othello_init(void);
bool othello_move(int bx, int by, int player);   /* returns true if move was legal */
bool othello_has_move(int player);
bool othello_game_over(void);
void othello_score(int *black, int *white);
void othello_computer_turn(void);
extern int cursor_x, cursor_y;   /* board coordinates (0‑7) */
extern int current_player;       /* BLACK for human, WHITE for computer */

#endif /* OTHELLO_H */

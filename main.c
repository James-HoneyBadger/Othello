/* main.c – Othello for the Picocomputer‑6502 */
#include "othello.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static void wait_vblank(void)
{
    volatile uint8_t *vblank = (volatile uint8_t*)0x00;  /* incremented by IRQ */
    uint8_t target = *vblank + 2;      /* 2 ticks = ~30 fps (IRQ runs at 60 Hz) */
    while (*vblank < target) { __asm__("nop"); }
}

/* --------------------------------------------------------------
   Show a tiny title screen and wait for a key press.
   -------------------------------------------------------------- */
static void title_screen(void)
{
    gfx_fill_rect(0,0,FB_WIDTH,FB_HEIGHT,0);
    const char *msg = "OTHELLO – PUSH SPACE TO START";
    draw_string(8, FB_HEIGHT/2 - 4, msg);
    while (!(input_poll() & IN_FIRE)) { __asm__("nop"); }
    while (input_poll() & IN_FIRE)     { __asm__("nop"); }
}

/* --------------------------------------------------------------
   Main loop
   -------------------------------------------------------------- */
int main(void)
{
    gfx_init();
    othello_init();

    title_screen();

    while (!othello_game_over()) {
        /* ---- draw everything ------------------------------------ */
        gfx_draw_board();
        gfx_draw_cursor(cursor_x, cursor_y);
        gfx_draw_status();

        /* ---- handle player input -------------------------------- */
        uint8_t in = input_poll();
        if (in & IN_LEFT)  cursor_x = (cursor_x - 1) & 7;
        if (in & IN_RIGHT) cursor_x = (cursor_x + 1) & 7;
        if (in & IN_UP)    cursor_y = (cursor_y - 1) & 7;
        if (in & IN_DOWN)  cursor_y = (cursor_y + 1) & 7;

        if (in & IN_FIRE) {
            if (current_player == BLACK) {
                if (othello_move(cursor_x, cursor_y, BLACK)) {
                    current_player = WHITE;   /* give turn to computer */
                }
            }
        }

        /* ---- computer turn (if it’s the computer’s move) -------- */
        if (current_player == WHITE && othello_has_move(WHITE)) {
            othello_computer_turn();
            current_player = BLACK;
        } else if (current_player == BLACK && !othello_has_move(BLACK)) {
            /* human has no move – skip */
            current_player = WHITE;
        }

        wait_vblank();   /* throttle to ~30 fps */
    }

    /* --------------------------------------------------------------
       Game over – display final score
       -------------------------------------------------------------- */
    int b, w;
    othello_score(&b, &w);
    gfx_fill_rect(0,0,FB_WIDTH,FB_HEIGHT,0);
    char buf[32];
    snprintf(buf, sizeof(buf), "GAME OVER  B:%d  W:%d", b, w);
    draw_string(48, FB_HEIGHT/2 - 4, buf);
    while (1) { __asm__("nop"); }   /* freeze */
    return 0;
}

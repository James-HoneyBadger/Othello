/* input.c – simple wrapper around $D010 */
#include "othello.h"

uint8_t input_poll(void)
{
    return INPUT_REG;
}

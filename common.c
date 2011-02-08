#include <stdio.h>
#include "common.h"

// Define the lucky charms...
struct charm LUCKY_CHARMS[CHARMS] = {
    {PINK, "HEART"},
    {ORANGE, "STAR"},
    {YELLOW, "MOON"},
    {GREEN, "CLOVER"},
    {BLUE, "DIAMOND"},
    {PURPLE, "HORSESHOE"},
    {RED, "BALLOON"}
};

// It's not important to understand
// anything that's happening here;
// just prints things in delicious
// colors and flavors...

enum ANSI_SETTING {
    DIM = 0,
    BRIGHT
};

enum ANSI_COLOR {
    ANSI_BLACK = 0,
    ANSI_RED,
    ANSI_GREEN,
    ANSI_YELLOW,
    ANSI_BLUE,
    ANSI_MAGENTA,
    ANSI_CYAN,
    ANSI_WHITE
};

void set_printf_color(
        int setting,
        int color) {
    printf("%c[%d;%dm", 0x1B, setting, color + 30);
}

void print_charm(struct charm *c) {
    char *str;
    switch(c->color) {
        case PINK:
            str = "PINK";
            set_printf_color(BRIGHT, ANSI_MAGENTA);
            break;
        case YELLOW:
            str = "YELLOW";
            set_printf_color(BRIGHT, ANSI_YELLOW);
            break;
        case ORANGE:
            str = "ORANGE";
            set_printf_color(DIM, ANSI_YELLOW);
            break;
        case GREEN:
            str = "GREEN";
            set_printf_color(DIM, ANSI_GREEN);
            break;
        case BLUE:
            str = "BLUE";
            set_printf_color(BRIGHT, ANSI_BLUE);
            break;
        case PURPLE:
            str = "PURPLE";
            set_printf_color(DIM, ANSI_MAGENTA);
            break;
        case RED:
            str = "RED";
            set_printf_color(DIM, ANSI_RED);
            break;
    }
    printf("%s %s\033[0m", str, c->shape);
}

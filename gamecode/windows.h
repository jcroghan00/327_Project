#ifndef GAMECODE_WINDOWS_H
#define GAMECODE_WINDOWS_H

#include <ncurses.h>

typedef struct windows {
    WINDOW *monster_list_win;
} windows_t;

void create_monster_list_win(dungeon_t *d);
#endif

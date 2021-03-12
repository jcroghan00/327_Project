#ifndef GAMECODE_WINDOWS_H
#define GAMECODE_WINDOWS_H

#include <ncurses.h>

typedef struct windows {
    WINDOW *monster_list_win;
    WINDOW *terrain_map_win;
} windows_t;

void create_windows(dungeon_t *d);
#endif

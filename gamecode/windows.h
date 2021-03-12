#ifndef GAMECODE_WINDOWS_H
#define GAMECODE_WINDOWS_H

#include <ncurses.h>

typedef struct windows {
    WINDOW *monster_list_win;
    WINDOW *terrain_map_win;
    WINDOW *hardness_map_win;
    WINDOW *dist_map_win;
    WINDOW *tun_dist_map_win;

} windows_t;

void create_windows(dungeon_t *d);
#endif

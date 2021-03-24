#ifndef GAMECODE_WINDOWS_H
#define GAMECODE_WINDOWS_H

#include <ncurses.h>

class Windows {
public:
    WINDOW *monster_list_win;
    WINDOW *terrain_map_win;
    WINDOW *hardness_map_win;
    WINDOW *dist_map_win;
    WINDOW *tun_dist_map_win;
    WINDOW *game_ending_win;
};

void create_windows(Dungeon *d);
#endif

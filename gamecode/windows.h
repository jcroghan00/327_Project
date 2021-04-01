#ifndef GAMECODE_WINDOWS_H
#define GAMECODE_WINDOWS_H

#include <ncurses.h>
class Dungeon;
typedef struct heap heap_t;

class Windows {
public:
    WINDOW *monster_list_win;
    WINDOW *terrain_map_win;
    WINDOW *hardness_map_win;
    WINDOW *dist_map_win;
    WINDOW *tun_dist_map_win;
    WINDOW *game_ending_win;
    WINDOW *teleport_win;
    WINDOW *character_info_win;
    WINDOW *inventory_win;
};

void render_terrain_map(Dungeon *d);
void render_hardness_map(Dungeon *d);
void render_dist_map(Dungeon *d);
void render_tun_dist_map(Dungeon *d);
void render_end_game(Dungeon *d, heap_t *h);
void render_character_info(Dungeon *d);
void render_inventory(Dungeon *d);
void create_windows(Dungeon *d);

#endif

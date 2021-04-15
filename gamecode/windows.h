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
    WINDOW *monster_info_win;
    WINDOW *monster_info;
};

void render_terrain_map(Dungeon *d);
void render_hardness_map(Dungeon *d);
void render_dist_map(Dungeon *d);
void render_tun_dist_map(Dungeon *d);
void render_end_game(Dungeon *d, heap_t *h);
void render_character_info(Dungeon *d);
void render_inventory(Dungeon *d);
void render_equipment(Dungeon *d);
void render_drop(Dungeon *d);
void render_equip(Dungeon *d);
void render_expunge(Dungeon *d);
void render_inspect(Dungeon *d);
void render_inspect_equipment(Dungeon *d);
void render_monster_info_win(Dungeon *d);
void render_unequip(Dungeon *d);
void create_windows(Dungeon *d);

#endif

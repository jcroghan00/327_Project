#include <ncurses.h>

#include "dungeon.h"
#include "windows.h"

WINDOW* create_window(){
    WINDOW *new_win = newwin(0, 0, 0, 0);
    return new_win;
}

void create_monster_list_win(Dungeon *d){
    d->windows->monster_list_win = create_window();
    //scrollok(d->windows->monster_list_win, TRUE);
    wresize(d->windows->monster_list_win, LINES-4, 30);
    mvwin(d->windows->monster_list_win, 2, COLS/2 - 15);
    init_pair(1,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_list_win, COLOR_PAIR(1));
    keypad(d->windows->monster_list_win, TRUE);
}

void create_terrain_map_win(Dungeon *d){
    d->windows->terrain_map_win = create_window();
}
void create_hardness_map_win(Dungeon *d){
    d->windows->hardness_map_win = create_window();
}
void create_dist_map_win(Dungeon *d){
    d->windows->dist_map_win = create_window();
}
void create_tun_dist_map_win(Dungeon *d){
    d->windows->tun_dist_map_win = create_window();
}
void create_game_ending_win(Dungeon *d){
    d->windows->game_ending_win = create_window();
}
void create_teleport_win(Dungeon *d){
    d->windows->teleport_win = create_window();
}

void create_windows(Dungeon *d){
    create_monster_list_win(d);
    create_terrain_map_win(d);
    create_hardness_map_win(d);
    create_dist_map_win(d);
    create_tun_dist_map_win(d);
    create_game_ending_win(d);
    create_teleport_win(d);
}
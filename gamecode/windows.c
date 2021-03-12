#include <ncurses.h>

#include "dungeon.h"
#include "windows.h"


WINDOW* create_window(){
    WINDOW *new_win = newwin(0, 0, 0, 0);
    return new_win;
}



void create_monster_list_win(dungeon_t *d){
    d->windows->monster_list_win = create_window();
    wresize(d->windows->monster_list_win, LINES-4, COLS-41);
    mvwin(d->windows->monster_list_win, 2, COLS/2 - (COLS-41)/2);
    init_pair(1,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_list_win, COLOR_PAIR(1));

}

void create_terrain_map_win(dungeon_t *d){
    d->windows->terrain_map_win = create_window();
    //init_pair(2,COLOR_WHITE, COLOR_BLACK);
   // wbkgd(d->windows->monster_list_win, COLOR_PAIR(2));
}

void create_windows(dungeon_t *d){
    create_monster_list_win(d);
    create_terrain_map_win(d);
}
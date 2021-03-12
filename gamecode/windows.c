#include <ncurses.h>

#include "dungeon.h"
#include "windows.h"


WINDOW* create_window(){
    WINDOW *new_win = newwin(LINES-4, COLS-20, 2, 10);;
    return new_win;
}

void create_monster_list_win(dungeon_t *d){
    d->windows->monster_list_win = create_window();
    init_pair(1,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_list_win, COLOR_PAIR(1));

}

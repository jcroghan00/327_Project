#include <ncurses.h>

#include "dungeon.h"
#include "windows.h"

WINDOW* create_window(){
    WINDOW *new_win = newwin(0, 0, 0, 0);
    keypad(new_win, TRUE);
    return new_win;
}

void create_monster_list_win(Dungeon *d){
    d->windows->monster_list_win = create_window();
    wresize(d->windows->monster_list_win, LINES-4, 30);
    mvwin(d->windows->monster_list_win, 2, COLS/2 - 15);
    init_pair(1,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_list_win, COLOR_PAIR(1));
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
void create_character_info_win(Dungeon *d){
    d->windows->character_info_win = create_window();
}
void render_character_info(Dungeon *d){
    WINDOW *info_win = d->windows->character_info_win;
    const char *msg = "Press \'Q\' to close character info";
    mvwprintw(info_win,0, (COLS/2 - strlen(msg)/2), msg);

    touchwin(info_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(info_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                touchwin(stdscr);
                break;
            default:
                break;
        }
    }
}
void create_inventory_win(Dungeon *d){
    d->windows->inventory_win = create_window();
}
void render_inventory(Dungeon *d){
    WINDOW *inventory_win = d->windows->inventory_win;
    const char *msg = "Press \'Q\' to close inventory";
    mvwprintw(inventory_win,0, (COLS/2 - strlen(msg)/2), msg);
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(inventory_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                touchwin(stdscr);
                break;
            default:
                break;
        }
    }
}

void create_windows(Dungeon *d){
    create_monster_list_win(d);
    create_terrain_map_win(d);
    create_hardness_map_win(d);
    create_dist_map_win(d);
    create_tun_dist_map_win(d);
    create_game_ending_win(d);
    create_teleport_win(d);
    create_character_info_win(d);
    create_inventory_win(d);
}
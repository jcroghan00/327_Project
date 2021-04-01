#include <ncurses.h>

#include "dungeon.h"
#include "windows.h"
#include "path.h"
#include "pc.h"
#include "rlg327.h"

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
void render_terrain_map(Dungeon *d)
{
    WINDOW *map_window = d->windows->terrain_map_win;
    pair_t p;
    const char *msg = "Press \'Q\' to close terrain map";
    mvwprintw(map_window,0, (COLS/2 - strlen(msg)/2), msg);
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            switch (mappair(p)) {
                case ter_wall:
                case ter_wall_immutable:
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],' ');
                    break;
                case ter_floor:
                case ter_floor_room:
                    wattron(map_window,COLOR_PAIR(FLOOR_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'.');
                    wattroff(map_window,COLOR_PAIR(FLOOR_PAIR));
                    break;
                case ter_floor_hall:
                    wattron(map_window,COLOR_PAIR(FLOOR_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'#');
                    wattroff(map_window,COLOR_PAIR(FLOOR_PAIR));
                    break;
                case ter_debug:
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'*');
                    //fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
                    break;
                case ter_stairs_up:
                    wattron(map_window,COLOR_PAIR(STAIR_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'<');
                    wattroff(map_window,COLOR_PAIR(STAIR_PAIR));
                    break;
                case ter_stairs_down:
                    wattron(map_window,COLOR_PAIR(STAIR_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'>');
                    wattroff(map_window,COLOR_PAIR(STAIR_PAIR));
                    break;
                default:
                    break;
            }
        }
    }
    touchwin(map_window);
    int visible = 1;
    while (visible) {
        int val = wgetch(map_window);
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

void create_hardness_map_win(Dungeon *d){
    d->windows->hardness_map_win = create_window();
}
void render_hardness_map(Dungeon *d){
    //TODO add cursor functionality to view targeted squares hardness
    init_pair(21, COLOR_RED, COLOR_BLACK);
    init_pair(22, COLOR_YELLOW, COLOR_BLACK);
    init_pair(23, COLOR_GREEN, COLOR_BLACK);
    WINDOW *map_window = d->windows->hardness_map_win;
    pair_t p;
    const char *msg = "Press \'Q\' to close hardness map";
    mvwprintw(map_window,0, (COLS/2 - strlen(msg)/2), msg);

    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            uint8_t hardness = d->hardness[p[dim_y]][p[dim_x]];
            if (hardness && hardness != 255){
                char display = '0' + 1 + hardness/85;
                int color = (1+ hardness/85) + 20;
                wattron(map_window,COLOR_PAIR(color));
                mvwaddch(map_window,p[dim_y] + 1, p[dim_x],display);
                wattroff(map_window,COLOR_PAIR(color));
            }
        }
    }


    touchwin(map_window);
    int visible = 1;
    while (visible) {
        int val = wgetch(map_window);
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

void create_dist_map_win(Dungeon *d){
    d->windows->dist_map_win = create_window();
}
void render_dist_map(Dungeon *d){
    init_pair(20, COLOR_WHITE, COLOR_BLACK);
    init_pair(21, COLOR_RED, COLOR_BLACK);
    init_pair(22, COLOR_YELLOW, COLOR_BLACK);
    init_pair(23, COLOR_GREEN, COLOR_BLACK);
    init_pair(24, COLOR_CYAN, COLOR_BLACK);
    init_pair(25, COLOR_BLUE, COLOR_BLACK);
    init_pair(26, COLOR_MAGENTA, COLOR_BLACK);
    dijkstra_non_tunneling(d);
    WINDOW *map_window = d->windows->terrain_map_win;
    //pair_t p;
    clear();
    const char *msg = "Press \'Q\' to close distance map";
    mvwprintw(map_window,0, (COLS/2 - strlen(msg)/2), msg);
    pair_t p;
    Monster_Path path;
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            path = d->non_tun_path[p[dim_y]][p[dim_x]];
            if (path.cost != INT_MAX){
                if (!path.cost){
                    wattron(map_window,COLOR_PAIR(PLAYER_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'@');
                    wattroff(map_window,COLOR_PAIR(PLAYER_PAIR));
                } else {
                    char display = '0' + path.cost % 10;
                    int color = ((path.cost / 10) % 7) + 20;
                    wattron(map_window,COLOR_PAIR(color));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],display);
                    wattroff(map_window,COLOR_PAIR(color));
                }
            }
        }
    }
    touchwin(map_window);
    int visible = 1;
    while (visible) {
        int val = wgetch(map_window);
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

void create_tun_dist_map_win(Dungeon *d){
    d->windows->tun_dist_map_win = create_window();
}
void render_tun_dist_map(Dungeon *d){
    init_pair(20, COLOR_WHITE, COLOR_BLACK);
    init_pair(21, COLOR_RED, COLOR_BLACK);
    init_pair(22, COLOR_YELLOW, COLOR_BLACK);
    init_pair(23, COLOR_GREEN, COLOR_BLACK);
    init_pair(24, COLOR_CYAN, COLOR_BLACK);
    init_pair(25, COLOR_BLUE, COLOR_BLACK);
    init_pair(26, COLOR_MAGENTA, COLOR_BLACK);
    dijkstra_tunneling(d);
    WINDOW *map_window = d->windows->terrain_map_win;
    const char *msg = "Press \'Q\' to close tunneling distance map";
    mvwprintw(map_window,0, (COLS/2 - strlen(msg)/2), msg);
    pair_t p;
    Monster_Path path;
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            path = d->tun_path[p[dim_y]][p[dim_x]];
            if (path.cost != INT_MAX){
                if (!path.cost){
                    wattron(map_window,COLOR_PAIR(PLAYER_PAIR));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],'@');
                    wattroff(map_window,COLOR_PAIR(PLAYER_PAIR));
                } else {
                    char display = '0' + path.cost % 10;
                    int color = ((path.cost / 10) % 7) + 20;
                    wattron(map_window,COLOR_PAIR(color));
                    mvwaddch(map_window,p[dim_y] + 1, p[dim_x],display);
                    wattroff(map_window,COLOR_PAIR(color));
                }
            }
        }
    }




    touchwin(map_window);
    int visible = 1;
    while (visible) {
        int val = wgetch(map_window);
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

void create_game_ending_win(Dungeon *d){
    d->windows->game_ending_win = create_window();
}
void render_end_game(Dungeon *d, heap_t *h){
    const char *victory =
            "\n                                       o\n"
            "                                      $\"\"$o\n"
            "                                     $\"  $$\n"
            "                                      $$$$\n"
            "                                      o \"$o\n"
            "                                     o\"  \"$\n"
            "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
            "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
            "\"oo   o o o o\n"
            "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
            "   \"o$$\"    o$$\n"
            "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
            "     o\"\"\n"
            "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
            "   o\"\n"
            "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
            "\"$$$  $\n"
            "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
            "\"\"      \"\"\" \"\n"
            "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
            "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
            "\"$$$$\n"
            "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
            "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
            "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
            "              $\"                                                 \"$\n"
            "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
            "$\"$\"$\"$\"$\"$\"$\"$\n"
            "                                   You win!";

    const char *tombstone =
            "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
            "               /     /         \\             __\n"
            "              /     /           \\            ||\n"
            "             /____ /   Rest in   \\           ||\n"
            "            |     |    Pieces     |          ||\n"
            "            |     |               |          ||\n"
            "            |     |   A. Luser    |          ||\n"
            "            |     |               |          ||\n"
            "            |     |     * *   * * |         _||_\n"
            "            |     |     *\\/* *\\/* |        | TT |\n"
            "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
            "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
            "            |     |         \\/..\"\"\"\"\"...\"\"\""
            "\\ || /.\"\"\".......\"\"\"\"...\n"
            "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
            "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
            "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
            "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
            "            You're dead.  Better luck in the next life.";
    const char* msg = "Game Over! Press \'Q\' to Quit or \'R\' to restart!";
    WINDOW *game_win = d->windows->game_ending_win;
    if (d->pc->isLiving()){
        wprintw(game_win,victory);
    } else {
        wprintw(game_win, tombstone);
    }
    mvwprintw(game_win,LINES-2,COLS/2 - strlen(msg)/2, msg);
    touchwin(game_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(game_win);
        switch (val) {
            // Quit the window
            case 'Q':
                endwin();
                delete_dungeon(d, h);
                exit(0);
            case 'R':
                visible = 0;
                new_dungeon(d,h);
                play_game(d, h);
                touchwin(stdscr);
            default:
                break;
        }
    }
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
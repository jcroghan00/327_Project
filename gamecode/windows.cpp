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
    init_pair(33,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_list_win, COLOR_PAIR(33));
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
    init_pair(21, COLOR_RED, COLOR_BLACK);
    init_pair(22, COLOR_YELLOW, COLOR_BLACK);
    init_pair(23, COLOR_GREEN, COLOR_BLACK);
    pair_t cursor;
    cursor[dim_x] = d->pc->pos[dim_x];
    cursor[dim_y] = d->pc->pos[dim_y];
    WINDOW *map_window = d->windows->hardness_map_win;
    int visible = 1;
    const char *msg = "Press \'Q\' to close hardness map";
    while (visible) {
        mvwprintw(map_window,0, (COLS/2 - strlen(msg)/2), msg);
        cursor[dim_x] = 0 < cursor[dim_x] ? cursor[dim_x] : 1;
        cursor[dim_y] = 0 < cursor[dim_y] ? cursor[dim_y] : 1;
        cursor[dim_x] = cursor[dim_x] < DUNGEON_X-1 ? cursor[dim_x] : DUNGEON_X - 2;
        cursor[dim_y] = cursor[dim_y] < DUNGEON_Y-1 ? cursor[dim_y] : DUNGEON_Y - 2;
        pair_t p;
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
        mvwaddch(map_window, cursor[dim_y] + 1, cursor[dim_x], '*');

        mvwprintw(map_window, DUNGEON_Y +1, 0, ("Cell Hardness: "));
        int hard = d->hardness[cursor[dim_y]][cursor[dim_x]];
        int color = (1+ hard/85) + 20;
        wattron(map_window,COLOR_PAIR(color));
        hard ? wprintw(map_window, ("%3i"),hard) : wprintw(map_window, ("   "));
        wattroff(map_window,COLOR_PAIR(color));
        wprintw(map_window, (" Pos: %2i %2i"),cursor[dim_x], cursor[dim_y]);
        int val = wgetch(map_window);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                werase(map_window);
                touchwin(stdscr);
                break;
                // Move up-left
            case KEY_HOME:
            case '7':
            case 'y':
                cursor[dim_y]--;
                cursor[dim_x]--;
                werase(map_window);
                break;
                // Move up
            case KEY_UP:
            case '8':
            case 'k':
                cursor[dim_y]--;
                werase(map_window);
                break;
                // Move up-right
            case KEY_PPAGE:
            case '9':
            case 'u':
                cursor[dim_y]--;
                cursor[dim_x]++;
                werase(map_window);
                break;
                // Move right
            case KEY_RIGHT:
            case '6':
            case 'l':
                cursor[dim_x]++;
                werase(map_window);
                break;

                // Move down-right
            case KEY_NPAGE:
            case '3':
            case 'n':
                cursor[dim_y]++;
                cursor[dim_x]++;
                werase(map_window);
                break;

                // Move down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor[dim_y]++;
                werase(map_window);
                break;
                // Move down-left
            case KEY_END:
            case '1':
            case 'b':
                cursor[dim_y]++;
                cursor[dim_x]--;
                werase(map_window);
                break;
                // Move left
            case KEY_LEFT:
            case '4':
            case 'h':
                cursor[dim_x]--;
                werase(map_window);
                break;
            default:
                break;
        }
    }
    //TODO add cursor functionality to view targeted squares hardness
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
                d->new_dungeon(h);
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
    wresize(d->windows->inventory_win, LINES-10, 50);
    mvwin(d->windows->inventory_win, 5, COLS/2 - 25);
    init_pair(33,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->inventory_win, COLOR_PAIR(33));
}
void render_inventory(Dungeon *d){
    WINDOW *inventory_win = d->windows->inventory_win;
    int row=0,col=0;
    getmaxyx(inventory_win,row,col);
    const char *msg = "Press \'Q\' to close inventory";
    mvwprintw(inventory_win,row-row, (col/2 - strlen(msg)/2), msg);
    wmove(inventory_win,2,0);
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    for (int i = 0; i < size; i++){
        wprintw(inventory_win, " Carry Slot %2i: ", i);
        if (d->pc->carrySlots[i]){
            wprintw(inventory_win,d->pc->carrySlots[i]->name.c_str());
        } else {
            wprintw(inventory_win, "Empty!");
        }
        wprintw(inventory_win,"\n");
    }
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(inventory_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;
            default:
                break;
        }
    }
}
void render_equipment(Dungeon *d){
    const char *titles[] = {"WEAPON ","OFFHAND ","RANGED ","ARMOR ","HELMET ","CLOAK ","GLOVES ","BOOTS ","RING ","RING ","AMULET ","LIGHT "};
    WINDOW *inventory_win = d->windows->inventory_win;
    int row=0,col=0;
    getmaxyx(inventory_win,row,col);
    const char *msg = "Press \'Q\' to close inventory";
    mvwprintw(inventory_win,row-row, (col/2 - strlen(msg)/2), msg);
    wmove(inventory_win,1,0);
    int size = sizeof d->pc->equipSlots / sizeof d->pc->equipSlots[0];
    for (int i = 0; i < size; i++){
        wprintw(inventory_win, " %s%2i","Equip. slot:", i);
        wprintw(inventory_win, "%8s",titles[i]);
        if (d->pc->equipSlots[i]){
            wprintw(inventory_win,d->pc->equipSlots[i]->name.c_str());
        } else {
            wprintw(inventory_win, "Empty!");
        }
        wprintw(inventory_win,"\n");
    }
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(inventory_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;
            default:
                break;
        }
    }
}
void render_drop(Dungeon *d) {
    WINDOW *inventory_win = d->windows->inventory_win;
    int row = 0, col = 0, cursor = 0;
    const char *l = "Press \'ENTER\' to drop item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win, row, col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        mvwprintw(inventory_win, row - row, (col / 2 - strlen(msg) / 2), msg);
        wmove(inventory_win, 2, 0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++) {
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " Carry Slot %2i: ", i);
            if (d->pc->carrySlots[i]) {
                wprintw(inventory_win, d->pc->carrySlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win, "\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            //select a item to drop
            case ENTER_KEY:
            case KEY_ENTER:
                if (d->pc->carrySlots[cursor]){
                    d->objMap[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = d->pc->carrySlots[cursor];
                    d->pc->carrySlots[cursor] = NULL;

                    visible = 0;
                    werase(inventory_win);
                    touchwin(stdscr);
                } else {
                    const char *mes = "        No Item In This Slot!        ";
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(mes) / 2), mes);
                }
                break;

                // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;

                // scroll up
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}
void render_equip(Dungeon *d) {
    WINDOW *inventory_win = d->windows->inventory_win;
    int row = 0, col = 0, cursor = 0;
    const char *l = "Press \'ENTER\' to equip item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win, row, col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        mvwprintw(inventory_win, row - row, (col / 2 - strlen(msg) / 2), msg);
        wmove(inventory_win, 2, 0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++) {
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " Carry Slot %2i: ", i);
            if (d->pc->carrySlots[i]) {
                wprintw(inventory_win, d->pc->carrySlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win, "\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            //select a item to drop
            case ENTER_KEY:
            case KEY_ENTER:
                if (d->pc->carrySlots[cursor]){
                    int type;

                    if(d->pc->carrySlots[cursor]->type.WEAPON){
                        type = 0;
                    }
                    else if(d->pc->carrySlots[cursor]->type.OFFHAND){
                        type = 1;
                    }
                    else if(d->pc->carrySlots[cursor]->type.RANGED){
                        type = 2;
                    }
                    else if(d->pc->carrySlots[cursor]->type.ARMOR){
                        type = 3;
                    }
                    else if(d->pc->carrySlots[cursor]->type.HELMET){
                        type = 4;
                    }
                    else if(d->pc->carrySlots[cursor]->type.CLOAK){
                        type = 5;
                    }
                    else if(d->pc->carrySlots[cursor]->type.GLOVES){
                        type = 6;
                    }
                    else if(d->pc->carrySlots[cursor]->type.BOOTS){
                        type = 7;
                    }
                    else if(d->pc->carrySlots[cursor]->type.RING && !d->pc->equipSlots[8]){
                        type = 8;
                    }
                    else if(d->pc->carrySlots[cursor]->type.RING){
                        type = 9;
                    }
                    else if(d->pc->carrySlots[cursor]->type.AMULET){
                        type = 10;
                    }
                    else if(d->pc->carrySlots[cursor]->type.LIGHT){
                        type = 11;
                    }

                    if(d->pc->equipSlots[type]){
                        swap(d->pc->carrySlots[cursor], d->pc->equipSlots[type]);
                    }
                    else {
                        d->pc->equipSlots[type] = d->pc->carrySlots[cursor];
                        d->pc->carrySlots[cursor] = NULL;
                    }

                    d->pc->speed = d->pc->getSpeed();

                    visible = 0;
                    werase(inventory_win);
                    touchwin(stdscr);
                } else {
                    const char *mes = "        No Item In This Slot!        ";
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(mes) / 2), mes);
                }
                break;

                // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;

                // scroll up
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}
void render_expunge(Dungeon *d){
    WINDOW *inventory_win = d->windows->inventory_win;
    int row = 0, col = 0, cursor = 0;
    const char *l = "Press \'ENTER\' to expunge(delete) item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win, row, col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        mvwprintw(inventory_win, row - row, (col / 2 - strlen(msg) / 2), msg);
        wmove(inventory_win, 2, 0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++) {
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " Carry Slot %2i: ", i);
            if (d->pc->carrySlots[i]) {
                wprintw(inventory_win, d->pc->carrySlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win, "\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            //select a item to expunge
            case ENTER_KEY:
            case KEY_ENTER:
                if (d->pc->carrySlots[cursor]){
                    d->pc->carrySlots[cursor] = NULL;

                    visible = 0;
                    werase(inventory_win);
                    touchwin(stdscr);
                } else {
                    const char *mes = "        No Item In This Slot!        ";
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(mes) / 2), mes);
                }
                break;

                // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;

                // scroll up
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}
void render_inspect(Dungeon *d){
    WINDOW *inventory_win = d->windows->inventory_win;
    int row = 0, col = 0, cursor = 0;
    const char *l = "Press \'ENTER\' to inspect item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win, row, col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while (visible) {
        mvwprintw(inventory_win, row - row, (col / 2 - strlen(msg) / 2), msg);
        wmove(inventory_win, 2, 0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++) {
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " Carry Slot %2i: ", i);
            if (d->pc->carrySlots[i]) {
                wprintw(inventory_win, d->pc->carrySlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win, "\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            //select a item to inspect
            case ENTER_KEY:
            case KEY_ENTER:
                if (d->pc->carrySlots[cursor]){
                    // inspect item
                    werase(inventory_win);
                    int info = 1;
                    const char *name = d->pc->carrySlots[cursor]->name.c_str();
                    mvwprintw(inventory_win, 1, (col / 2 - strlen(name) / 2), name);
                    wprintw(inventory_win,"\n");
                    wprintw(inventory_win, d->pc->carrySlots[cursor]->desc.c_str());
                    while (info){
                        int val = wgetch(inventory_win);
                        switch(val){
                            case 'Q':
                                info = 0;
                                break;
                        }
                    }
                    werase(inventory_win);
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                } else {
                    const char *mes = "        No Item In This Slot!        ";
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(mes) / 2), mes);
                }
                break;

                // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;

                // scroll up
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}
void render_unequip(Dungeon *d){
    const char *titles[] = {"WEAPON ","OFFHAND ","RANGED ","ARMOR ","HELMET ","CLOAK ","GLOVES ","BOOTS ","RING ","RING ","AMULET ","LIGHT "};
    WINDOW *inventory_win = d->windows->inventory_win;
    int row=0,col=0,cursor=0;
    const char *l = "Press \'ENTER\' to take off item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win,row,col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->equipSlots / sizeof d->pc->equipSlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while(visible)
    {
        mvwprintw(inventory_win,row-row, (col/2 - strlen(msg)/2), msg);
        wmove(inventory_win,1,0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++){
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " %s%2i","Equip. slot:", i);
            wprintw(inventory_win, "%8s",titles[i]);
            if (d->pc->equipSlots[i]){
                wprintw(inventory_win,d->pc->equipSlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win,"\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            case ENTER_KEY:
            case KEY_ENTER:
                {
                    int moved = 0;
                    //haha
                    int size2 = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
                    for(int i = 0; i < size2; ++i){
                        if(!d->pc->carrySlots[i]){
                            d->pc->carrySlots[i] = d->pc->equipSlots[cursor];
                            d->pc->equipSlots[cursor] = NULL;
                            moved = 1;
                            break;
                        }
                    }

                    if(!moved){
                        d->objMap[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = d->pc->equipSlots[cursor];
                        d->pc->equipSlots[cursor] = NULL;
                    }

                    d->pc->speed = d->pc->getSpeed();

                    visible = 0;
                    werase(inventory_win);
                    touchwin(stdscr);
                    break;
                }
            // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}
void render_inspect_equipment(Dungeon *d){
    const char *titles[] = {"WEAPON ","OFFHAND ","RANGED ","ARMOR ","HELMET ","CLOAK ","GLOVES ","BOOTS ","RING ","RING ","AMULET ","LIGHT "};
    WINDOW *inventory_win = d->windows->inventory_win;
    int row=0,col=0,cursor=0;
    const char *l = "Press \'ENTER\' to take off item";
    const int ENTER_KEY = 10;
    getmaxyx(inventory_win,row,col);
    const char *msg = "Press \'Q\' to close inventory";
    int size = sizeof d->pc->equipSlots / sizeof d->pc->equipSlots[0];
    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
    touchwin(inventory_win);
    int visible = 1;
    while(visible)
    {
        mvwprintw(inventory_win,row-row, (col/2 - strlen(msg)/2), msg);
        wmove(inventory_win,1,0);
        cursor = cursor >= 0 ? cursor : 0;
        cursor = cursor < size ? cursor : size-1;
        for (int i = 0; i < size; i++){
            if (cursor == i) {
                wattron(inventory_win, A_STANDOUT);
            }
            wprintw(inventory_win, " %s%2i","Equip. slot:", i);
            wprintw(inventory_win, "%8s",titles[i]);
            if (d->pc->equipSlots[i]){
                wprintw(inventory_win,d->pc->equipSlots[i]->name.c_str());
            } else {
                wprintw(inventory_win, "Empty!");
            }
            wprintw(inventory_win,"\n");
            wattroff(inventory_win, A_STANDOUT);
        }
        int val = wgetch(inventory_win);
        switch (val) {
            case ENTER_KEY:
            case KEY_ENTER:
            {
                if (d->pc->equipSlots[cursor]){
                    // inspect item
                    werase(inventory_win);
                    int info = 1;
                    const char *name = d->pc->equipSlots[cursor]->name.c_str();
                    mvwprintw(inventory_win, 1, (col / 2 - strlen(name) / 2), name);
                    wprintw(inventory_win,"\n");
                    wprintw(inventory_win, d->pc->equipSlots[cursor]->desc.c_str());
                    while (info){
                        int val = wgetch(inventory_win);
                        switch(val){
                            case 'Q':
                                info = 0;
                                break;
                        }
                    }
                    werase(inventory_win);
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                } else {
                    const char *mes = "        No Item In This Slot!        ";
                    mvwprintw(inventory_win, row - 1, (col / 2 - strlen(mes) / 2), mes);
                }
                break;
            }
                // Quit the window
            case 'Q':
                visible = 0;
                werase(inventory_win);
                touchwin(stdscr);
                break;
            case KEY_UP:
            case '8':
            case 'k':
                cursor--;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;

                // scroll down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor++;
                werase(inventory_win);
                mvwprintw(inventory_win, row - 1, (col / 2 - strlen(l) / 2), l);
                break;
            default:
                break;
        }
    }
}

void create_monster_info_win(Dungeon *d){
    d->windows->monster_info_win = create_window();
}
void create_monster_info(Dungeon *d){
    d->windows->monster_info = create_window();
    wresize(d->windows->monster_info, LINES, 80);
    mvwin(d->windows->monster_info, 0, COLS/2 - 40);
    init_pair(33,COLOR_WHITE, COLOR_BLUE);
    wbkgd(d->windows->monster_info, COLOR_PAIR(33));
}
void render_monster_info(Dungeon *d, Monster *m){
    WINDOW *info_win = d->windows->monster_info;
    wclear(info_win);
    const char *msg = "Press \'Q\' to close window";
    mvwprintw(info_win, 0, (COLS / 2 - strlen(msg) / 2), msg);
    wmove(info_win,2,0);

    string monster = "Name: " + m->name;
    monster.push_back('\n');
    monster += "Description: " + m->desc;
    wprintw(info_win,monster.c_str());
    touchwin(info_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(info_win);
        switch (val) {
            case 'Q':
                visible = 0;
                break;
            default:
                break;
        }
    }
}
void render_monster_info_win(Dungeon *d) {
    pair_t cursor;
    cursor[dim_x] = d->pc->pos[dim_x];
    cursor[dim_y] = d->pc->pos[dim_y];
    WINDOW *monster_win = d->windows->monster_info_win;
    const char *msg = "Press \'t\' to select a monster or \'Q\' to close window";
    touchwin(monster_win);
    int visible = 1;
    while (visible) {
        mvwprintw(monster_win, 0, (COLS / 2 - strlen(msg) / 2), msg);
        cursor[dim_x] = 0 < cursor[dim_x] ? cursor[dim_x] : 1;
        cursor[dim_y] = 0 < cursor[dim_y] ? cursor[dim_y] : 1;
        cursor[dim_x] = cursor[dim_x] < DUNGEON_X ? cursor[dim_x] : DUNGEON_X - 1;
        cursor[dim_y] = cursor[dim_y] < DUNGEON_Y ? cursor[dim_y] : DUNGEON_Y - 1;
        mvwprintw(monster_win, 0, (COLS / 2 - strlen(msg) / 2), msg);
        render_ncurses(d, monster_win, 0);
        mvwaddch(monster_win, cursor[dim_y] + 1, cursor[dim_x], '*');
        if (character_mappair(cursor)){
            Character *Char = character_mappair(cursor);
            wattron(monster_win,COLOR_PAIR(Char->dispColor));
            mvwprintw(monster_win, DUNGEON_Y +1, 0, Char->name.c_str());
            wattroff(monster_win,COLOR_PAIR(Char->dispColor));
            wprintw(monster_win, " HP: %i", Char->hitpoints);
            wprintw(monster_win, " Damage: %i-%i", Char->damage.base, Char->damage.base + (Char->damage.numDice * Char->damage.numSides));
            wprintw(monster_win, " Speed: %i", Char->speed);
        }
        int val = wgetch(monster_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                werase(monster_win);
                touchwin(stdscr);
                break;
            //select a monster
            case 't':
                if (character_mappair(cursor)){
                    if (character_mappair(cursor)->getDisplayChar() != '@'){
                        render_monster_info(d, (Monster *)character_mappair(cursor));
                        werase(monster_win);
                    } else {
                        mvwprintw(monster_win, DUNGEON_Y +1, 0, "This is you!");
                    }

                } else {
                    mvwprintw(monster_win, DUNGEON_Y +1, 0, "Not a monster!");
                }
                break;
                // Move up-left
            case KEY_HOME:
            case '7':
            case 'y':
                cursor[dim_y]--;
                cursor[dim_x]--;
                werase(monster_win);
                break;
                // Move up
            case KEY_UP:
            case '8':
            case 'k':
                cursor[dim_y]--;
                werase(monster_win);
                break;
                // Move up-right
            case KEY_PPAGE:
            case '9':
            case 'u':
                cursor[dim_y]--;
                cursor[dim_x]++;
                werase(monster_win);
                break;
                // Move right
            case KEY_RIGHT:
            case '6':
            case 'l':
                cursor[dim_x]++;
                werase(monster_win);
                break;

                // Move down-right
            case KEY_NPAGE:
            case '3':
            case 'n':
                cursor[dim_y]++;
                cursor[dim_x]++;
                werase(monster_win);
                break;

                // Move down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor[dim_y]++;
                werase(monster_win);
                break;
                // Move down-left
            case KEY_END:
            case '1':
            case 'b':
                cursor[dim_y]++;
                cursor[dim_x]--;
                werase(monster_win);
                break;
                // Move left
            case KEY_LEFT:
            case '4':
            case 'h':
                cursor[dim_x]--;
                werase(monster_win);
                break;
            default:
                break;
        }
    }
}

void create_windows(Dungeon *d) {
    create_monster_list_win(d);
    create_terrain_map_win(d);
    create_hardness_map_win(d);
    create_dist_map_win(d);
    create_tun_dist_map_win(d);
    create_game_ending_win(d);
    create_teleport_win(d);
    create_character_info_win(d);
    create_inventory_win(d);
    create_monster_info_win(d);
    create_monster_info(d);
}

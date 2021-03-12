#include <ncurses.h>

#include "dungeon.h"
#include "monster.h"
#include "pc.h"
#include "heap.h"
#include "character.h"
#include "windows.h"
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

int play_game(dungeon_t *d, heap_t *h);

void end_game(dungeon_t *d, heap_t *h){
    WINDOW *game_win = d->windows->game_ending_win;
    if (d->pc->living){
        wprintw(game_win,victory);
    } else {
        wprintw(game_win,tombstone);
    }
    char *msg = "Game Over! Press \'Q\' to Quit or \'R\' to restart!";
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

int play_game(dungeon_t *d, heap_t *h)
{
    for(int i = 0; i < d->num_monsters+1; i++)
    {
        d->characters[i]->turn = 0;
        d->characters[i]->sd = i;
        heap_insert(h,d->characters[i]);
    }
    character_t *c;
    while(pc_is_alive(d))
    {
        if (!d->num_monsters){break;}
        c = heap_remove_min(h);
        if (c->living){
            if (c->sd == 0) {
                move_pc_ncurses(d, h);
                // pc_next_pos(d);
                render_ncurses(d);
                refresh(); /* Print it on to the real screen */
                usleep(250000);
            }
            else{
                move_monster(c,d);
            }
            c->turn = c->turn + (1000/c->speed);
            heap_insert(h, c);

            usleep(2500); // you cant see the monsters' steps otherwise
            render_ncurses(d);
            refresh();
        }
    }
    end_game(d, h);
    return 0;
}

int main(int argc, char *argv[])
{
    dungeon_t d = { .num_monsters = -1};
    struct timeval tv;
    uint32_t seed = 0;

    int save = 0;
    int load = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"--save"))         {save = 1;}
        else if (!strcmp(argv[i],"--load"))    {load = 1;}
        else if (!strcmp(argv[i],"--nummon"))  {d.num_monsters = atoi(argv[++i]);}
        else {seed=atoi(argv[i]);}
    }
    if (!seed)
    {
        gettimeofday(&tv, NULL);
        seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
    }

    srand(seed);
    initscr();
    start_color();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    init_dungeon(&d);
    if (load){
        load_dungeon(&d);
    } else {
        gen_dungeon(&d);
    }
    if (save) {
        save_dungeon(&d);
    }

    render_ncurses(&d);

    heap_t h;
    heap_init(&h,character_cmp,NULL);
    int won = play_game(&d, &h);

    endwin();

    delete_dungeon(&d, &h);

    if (won){
        printf("%s", victory);
    } else {
        printf("%s", tombstone);
    }
    return 0;

}

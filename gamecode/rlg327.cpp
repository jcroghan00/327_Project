#include <ncurses.h>
#include <string>

#include "dungeon.h"
#include "Monster.h"
#include "pc.h"
#include "heap.h"
#include "Character.h"
#include "windows.h"
#include "parser.h"

vector<Monstertype> monster_types;

void play_game(Dungeon *d, heap_t *h)
{
    d->pc->setSd(0);
    heap_insert(h,d->pc);
    for(int i = 0; i < d->num_monsters; i++)
    {
        d->monsters[i]->setSd(i+1);
        heap_insert(h,d->monsters[i]);
    }
    Character *c;
    while(d->pc->isLiving())
    {
        if (!d->num_monsters){break;}
        c = (Character*)heap_remove_min(h);
        if (c->isLiving()){
            if (c->getSd() == 0) {
                move_pc_ncurses(d, h);
                render(d);
                refresh(); /* Print it on to the real screen */
                usleep(250000);
            }
            else{
                ((Monster*)c)->move_monster(d);
                update_last_seen(d);
            }
            c->setNextTurn();
            heap_insert(h, c);

            usleep(2500); // you cant see the monsters' steps otherwise
            render(d);
            refresh();
        }
    }
    render_end_game(d, h);
}

int main(int argc, char *argv[])
{
    Dungeon d = { .num_monsters = -1};
    struct timeval tv;
    uint32_t seed = 0;

    int save = 0;
    int load = 0;
    int parse = 1;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"--save"))         {save = 1;}
        else if (!strcmp(argv[i],"--load"))    {load = 1;}
        else if (!strcmp(argv[i],"--parseon"))    {parse = 1;}
        else if (!strcmp(argv[i],"--nummon"))  {d.num_monsters = atoi(argv[++i]);}
        else {seed=atoi(argv[i]);}
    }
    if(parse){
        //monster_parser();
        object_parser();
        return 0;
    }
    if (!seed)
    {
        gettimeofday(&tv, NULL);
        seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
    }

    srand(seed);
    initscr();
    resizeterm(24,81);
    start_color();

    //TODO remove at some point
    init_pair(HARD_WALL_PAIR, COLOR_YELLOW, COLOR_BLACK); //??
    init_pair(SOFT_WALL_PAIR, COLOR_GREEN, COLOR_BLACK);   //??
    init_pair(IMMUTABLE_WALL_PAIR, COLOR_BLUE, COLOR_BLACK);
    init_pair(FLOOR_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(STAIR_PAIR, COLOR_CYAN, COLOR_BLACK);
    init_pair(PLAYER_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(MONSTER_PAIR, COLOR_RED, COLOR_BLACK);

    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
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

    render(&d);

    heap_t h;
    heap_init(&h,character_cmp,NULL);
    play_game(&d, &h);

    endwin();
    delete_dungeon(&d, &h);
    return 0;

}

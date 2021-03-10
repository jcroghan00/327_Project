#include <ncurses.h>

#include "dungeon.h"
#include "monster.h"
#include "pc.h"


void move_pc_ncurses(dungeon_t *d){
    int x = 0; 
    int y = 0;
    mvaddch(x, y, 'i');
    if(getch() == '\033'){
        getch();
        switch(getch())
        {
        case 'A':
            y = -1;
            x = 0;
            break;
        case 'B':
            y = 1;
            x = 0;
            break;
        case 'C':
            y = 0;
            x = -1;
            break;
        case 'D':
            y = 0;
            x = 1;
            break;
        default:
            x = 0;
            y = 0;
            break;
        }

        if(d->map[d->pc.pos[dim_y] + y][d->pc.pos[dim_x] + x] == ter_wall || d->map[d->pc.pos[dim_y] + y][d->pc.pos[dim_x] + x] == ter_wall_immutable){y = 0, x = 0;}


        // if (d->character_map[d->pc.pos[dim_y] + y][d->pc.pos[dim_x] + x] != NULL &&
        //     d->character_map[d->pc.pos[dim_y] + y][d->pc.pos[dim_x] + x] != &d->pc)
        // {
        //     d->character_map[d->pc.pos[dim_y] + y][d->pc.pos[dim_x] + x]->living = 0;
        //     d->num_monsters--;
        // }
      d->character_map[d->pc.pos[dim_y]][d->pc.pos[dim_x]] = NULL;
        d->pc.pos[dim_y] += y;
        d->pc.pos[dim_x] += x;
        d->character_map[d->pc.pos[dim_y]][d->pc.pos[dim_x]] = &d->pc;
    }

}

int play_game(dungeon_t *d)
{
    heap_t h;
    heap_init(&h,character_cmp,NULL);
    for(int i = 0; i < d->num_monsters+1; i++)
    {
        d->characters[i]->turn = 0;
        d->characters[i]->sd = i;
        heap_insert(&h,d->characters[i]);
    }
    int won = 0;
    character_t *c;
    while(d->pc.living)
    {
        if (!d->num_monsters){won = 1;break;}
        c = heap_remove_min(&h);
        if (c->living){
            if (c->sd == 0) {
                move_pc_ncurses(d);
                // pc_next_pos(d);
                render_ncurses(d);
                refresh(); /* Print it on to the real screen */
                getch(); /* Wait for user input */
                usleep(250000);
            }
            else{
                move_monster(c,d);
            }
            c->turn = c->turn + (1000/c->speed);
            heap_insert(&h, c);
        }
    }
    if (won){
        return 1;
    } else {
        return 0;
    }
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

    init_dungeon(&d);
    if (load){
        load_dungeon(&d);
    }
    else {
        gen_dungeon(&d);
    }
    if (save) {
        save_dungeon(&d);
    }

    initscr();

    // initscr();

    // while(1){
    //     move_pc_ncurses();
    // }
    // // while(1){
    // //     int a = getch();
    // //     printf("%d",a);

        
    // // }
    // endwin();
    render_ncurses(&d);

    int won = play_game(&d);

	endwin();
	render_dungeon(&d);
    delete_dungeon(&d);

    if (won){
        printf("\nGAME OVER\nYOU WON\n");
    } else {
        printf("\nGAME OVER\nYOU LOST\n");
    }
    return 0;
}

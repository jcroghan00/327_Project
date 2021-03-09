#include <ncurses.h>
#include "dungeon.h"

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
    render_dungeon(&d);
    if (save) {
        save_dungeon(&d);
    }

    initscr();

    int won = play_game(&d);

	endwin();
    delete_dungeon(&d);

    if (won){
        printf("\nGAME OVER\nYOU WON\n");
    } else {
        printf("\nGAME OVER\nYOU LOST\n");
    }
    return 0;
}

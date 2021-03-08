/*

#include "dungeon.h"

//TODO MOVE DUNGEON FUNCTION NEEDED INTO HEADER
//#include "dungeon.c"

int main(int argc, char *argv[])
{
    dungeon_t d = { .num_monsters = -1};
    // change switches to local definition
    file_info_t f = { .load = 0, .save = 0, .ren_non_tun_dist_map = 0, .ren_tun_dist_map = 0};
    struct timeval tv;
    uint32_t seed = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"--save"))         {f.save=1;}
        else if (!strcmp(argv[i],"--load"))    {f.load=1;}
        else if (!strcmp(argv[i],"--distmap")) {f.ren_non_tun_dist_map = 1; f.ren_tun_dist_map = 1;}
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
    if (f.load){
        load_dungeon(&d, &f);
    }
    else {
        gen_dungeon(&d);
    }
    render_dungeon(&d,&f);
    if (f.save) {
        save_dungeon(&d,&f);
    }
    play_game(&d,&f);
    delete_dungeon(&d);

    return 0;
}
 */
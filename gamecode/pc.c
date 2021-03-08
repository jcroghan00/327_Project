#include "dungeon.h"







int is_pc_alive(dungeon_t *d){
    return d->pc.living;
}

void place_pc(dungeon_t *d){
    //if pc position isn't already defined, place it in a room
    if (!d->pc.pos[dim_x] && !d->pc.pos[dim_y])
    {
        int randRoom = rand() % d->num_rooms;
        int x = rand() % d->rooms[randRoom].size[dim_x];
        int y = rand() % d->rooms[randRoom].size[dim_y];
        d->pc.pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;
        d->pc.pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
    }
    //add pc to character map
    d->character_map[d->pc.pos[dim_y]][d->pc.pos[dim_x]] = &d->pc;
}
void config_pc(dungeon_t *d){
    memset(&d->pc, 0, sizeof (d->pc));
    d->pc.display_char = '@';
    d->pc.living = 1;
    d->pc.speed = PC_SPEED;
    place_pc(d);
    d->characters[0] = &d->pc;


}

int pc_next_pos(dungeon_t *d){
    //code for calculating the pc's next position should go here

    return 0;
}
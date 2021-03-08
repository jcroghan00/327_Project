#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "monster.h"
#include "dungeon.h"
#include "path.h"





int gen_monsters(dungeon_t *d)
{
    for(int i = 1; i <= d->num_monsters; i++)
    {
        d->characters[i]->living = 1;
        d->characters[i]->monster->intelligent = rand() % 2;
        //d->characters[i]->intelligent = 0;
        d->characters[i]->monster->telepath = rand() % 2;
        //d->characters[i]->telepath = 1;
        d->characters[i]->monster->tunneling = rand() % 2;
        //d->characters[i]->tunneling = 1;
        d->characters[i]->monster->erratic = rand() % 2;
        //d->characters[i]->erratic = 0;
        d->characters[i]->speed = rand() % 16 + 5;
        d->characters[i]->pc = 0;
        //outsource to function i monster.c to calc displaychar
        //d->characters[i]->display_char = get_display_char(&d->characters[i]);
    }

    int pcRoomNum;
    int totalArea = 0;
    for(int i = 0; i < d->num_rooms; ++i)
    {
        if(d->pc.pos[dim_x] >= d->rooms[i].position[dim_x] && d->pc.pos[dim_x] < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x]
           && d->pc.pos[dim_y] >= d->rooms[i].position[dim_y] && d->pc.pos[dim_y] < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y])
        {
            pcRoomNum = i;
        }
        else{
            totalArea += d->rooms[i].size[dim_x] * d->rooms[i].size[dim_y];
        }
    }

    int totalMonsters = 1;
    while(totalMonsters <= d->num_monsters)
    {
        if(totalMonsters == totalArea){break;}

        int randRoom = rand() % d->num_rooms;

        if(randRoom == pcRoomNum){continue;}

        int x = rand() % d->rooms[randRoom].size[dim_x];
        int y = rand() % d->rooms[randRoom].size[dim_y];

        if(d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] != NULL){continue;}

        d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] = d->characters[totalMonsters];

        d->characters[totalMonsters]->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
        d->characters[totalMonsters]->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;

        ++totalMonsters;
    }
    return 0;
}

/* ensure that character assignment is done on monster creation
char get_display_char(monster_t *m)
{
    if(c->pc == 1){return (char)'@';}

    char binary_char[4] = {'0'+(char)c->erratic,
                           '0'+(char)c->tunneling,
                           '0'+(char)c->telepath,
                           '0'+(char)c->monster->};

    int binary = atoi(binary_char),decimal = 0,base = 1,rem;
    while (binary > 0){
        rem = binary % 10;
        decimal = decimal + rem * base;
        binary = binary/10;
        base = base * 2;
    }
    if (decimal <= 9){
        return (char)('0' + decimal);}
    else {
        decimal -= 10;
        return (char)('A' + decimal);
    }
}
 */
void move_line(dungeon_t *d, character_t *c, dif_t *dif)
{
    if(c->pos[dim_x] < c->monster->pc_last_loc[dim_x]){
        dif->x = 1;
    }
    else if(c->pos[dim_x] > c->monster->pc_last_loc[dim_x]){
        dif->x = -1;
    }
    else{
        dif->x = 0;
    }

    if(c->pos[dim_y] < c->monster->pc_last_loc[dim_y]){
        dif->y = 1;
    }
    else if(c->pos[dim_y] > c->monster->pc_last_loc[dim_y]){
        dif->y = -1;
    }
    else{
        dif->y = 0;
    }

    if(d->map[c->pos[dim_y] + dif->y][c->pos[dim_x] + dif->x] == ter_wall || d->map[c->pos[dim_y] + dif->y][c->pos[dim_x] + dif->x] == ter_wall_immutable)
    {
        if(d->map[c->pos[dim_y]][c->pos[dim_x] + dif->x] == ter_wall || d->map[c->pos[dim_y]][c->pos[dim_x] + dif->x] == ter_wall_immutable){
            dif->x = 0;
        }
        else if(d->map[c->pos[dim_y] + dif->y][c->pos[dim_x] ] == ter_wall || d->map[c->pos[dim_y] + dif->y][c->pos[dim_x]] == ter_wall_immutable){
            dif->y = 0;
        }
    }
}

void final_move(character_t *c, dungeon_t *d,int dx,int dy)
{
    d->character_map[c->pos[dim_y]][c->pos[dim_x]] = NULL;
    if (d->character_map[c->pos[dim_y]+dy][c->pos[dim_x]+dx])
    {
        d->character_map[c->pos[dim_y]+dy][c->pos[dim_x]+dx]->living = 0;
    }
        // if collision is with the PC
    else if (c->pos[dim_y]+dy == d->pc.pos[dim_y] && c->pos[dim_x]+dx == d->pc.pos[dim_x])
    {
        d->pc.living = 0;
    }
    d->character_map[c->pos[dim_y]+dy][c->pos[dim_x]+dx] = c;
    c->pos[dim_y] = c->pos[dim_y]+dy;
    c->pos[dim_x] = c->pos[dim_x]+dx;
}

void move_monster(character_t *c, dungeon_t *d)
{
    int sees_player = 0,dx,dy;

    dif_t dif;
    dif.x = 0;
    dif.y = 0;

    if (c->monster->erratic) {
        if (rand() % 2) {
            int moved = 0,counter = 0;
            while(!moved) {
                if (counter > 500) {return;} //if it cant find a move, give up
                counter++;
                dx = (rand() % 3) - 1;
                dy = (rand() % 3) - 1;
                if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) == ter_wall) {
                    if(c->monster->tunneling) {
                        int hardness = hardnessxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy);
                        hardness = hardness-85;
                        if (hardness <= 0)
                        {
                            hardnessxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) = 0;
                            mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) = ter_floor_hall;
                        }
                        else {
                            hardnessxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) = hardness;
                            return;
                        } //tunneling monsters hit wall but didnt break it
                    }
                    else {continue;} //non-tunnel monster or immutable wall hit
                }
                if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) != ter_wall_immutable) {
                    final_move(c, d, dx, dy);
                    moved = 1;
                }
            }
            return;
        }
    }
    if (c->monster->telepath || bresenham_LOS(d,c,&dif))
    {
        sees_player = 1;
        c->monster->pc_last_loc[dim_x] = d->pc.pos[dim_x];
        c->monster->pc_last_loc[dim_y] = d->pc.pos[dim_y];
    }
    if (sees_player || c->monster->intelligent) {
        if (c->monster->intelligent) {
            if (c->monster->pc_last_loc[dim_x]) {
                if (c->monster->tunneling) {
                    //dx dy based on tunnel dist map
                    uint32_t cost = INT_MAX;
                    for (int i = -1; i <= 1; i++){
                        for (int j = -1; j <= 1; j++){
                            if (d->tun_path[c->pos[dim_y]+i][c->pos[dim_x]+j].cost < cost){
                                dy = i;
                                dx = j;
                                cost = d->tun_path[c->pos[dim_y]+i][c->pos[dim_x]+j].cost;
                            }
                        }
                    }
                } else // monster-> non tunneling
                {
                    //dx dy based on non tunnel dist map
                    uint32_t cost = INT_MAX;
                    for (int i = -1; i <= 1; i++){
                        for (int j = -1; j <= 1; j++){
                            if (d->non_tun_path[c->pos[dim_y]+i][c->pos[dim_x]+j].cost < cost){
                                dy = i;
                                dx = j;
                                cost = d->non_tun_path[c->pos[dim_y]+i][c->pos[dim_x]+j].cost;
                            }
                        }
                    }
                }
            }
            else {
                dx = 0;
                dy = 0;
            }
        }
        else // non monster->
        {
            if(c->monster->tunneling){
                bresenham_move(d,c,&dif);
                dx = dif.x;
                //printf("dx: %d\n",dif.x);
                dy = dif.y;
            }
            else{
                move_line(d,c,&dif);
                dx = dif.x;
                //printf("dx: %d\n",dif.x);
                dy = dif.y;
            }
        }
        if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) == ter_wall)
        {
            if (c->monster->tunneling)
            {
                int hardness = hardnessxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy);
                hardness = hardness-85;
                if (hardness <= 0)
                {
                    hardness = 0;
                    mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) = ter_floor_hall;
                }
                else {
                    hardnessxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) = hardness;
                    return;
                } //tunneling monsters hit wall but didnt break it
            }
            else{return;} // non-tunneling monster hit a wall
        }
        if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) != ter_wall_immutable) {
            final_move(c, d, dx, dy);
        }
    }
}

int bresenham_LOS(dungeon_t *d,character_t *c, dif_t *dif)
{
    int x0 = c->pos[dim_x];
    int y0 = c->pos[dim_y];
    int x1 = d->pc.pos[dim_x];
    int y1 = d->pc.pos[dim_y];

    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    while(1)
    {
        if(mapxy(x0, y0) == ter_wall || mapxy(x0, y0) == ter_wall_immutable)
        {
            return 0;
        }
        if(x0 == x1 && y0 == y1) {return 1;}
        e2 = err * 2;
        if(e2 >= dy){
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx){
            err += dx;
            y0 += sy;
        }
    }
}

void bresenham_move(dungeon_t *d,character_t *c, dif_t *dif)
{
    int x0 = c->pos[dim_x];
    int y0 = c->pos[dim_y];
    int x1 = d->pc.pos[dim_x];
    int y1 = d->pc.pos[dim_y];

    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    e2 = err * 2;
    if(e2 >= dy){
        dif->x = sx;
    }
    if(e2 <= dx){
        dif->y = sy;
    }
}



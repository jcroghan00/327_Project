#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "monster.h"
#include "dungeon.h"

void get_monster_path(character_t *c, dungeon_t *d){
    int x,y;
    if (c->monster->tunneling) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                c->monster->path_to_pc[y][x] = d->tun_path[y][x];
            }
        }
    } else {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                c->monster->path_to_pc[y][x] = d->non_tun_path[y][x];
            }
        }
    }
}

char get_monster_char(character_t *c)
{
    char binary_char[4] = {'0'+(char)c->monster->erratic,
                           '0'+(char)c->monster->tunneling,
                           '0'+(char)c->monster->telepath,
                           '0'+(char)c->monster->intelligent};

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

int gen_monsters(dungeon_t *d)
{
    for(int i = 1; i <= d->num_monsters; i++)
    {
        d->characters[i] = malloc(sizeof(character_t));
        d->characters[i]->living = 1;
        d->characters[i]->monster = malloc (sizeof(monster_t));

        d->characters[i]->monster->intelligent = rand() % 2;
        d->characters[i]->monster->telepath = rand() % 2;
        d->characters[i]->monster->tunneling = rand() % 2;
        d->characters[i]->monster->erratic = rand() % 2;

        //d->characters[i]->monster->intelligent = 0;
        //d->characters[i]->monster->telepath = 1;
        //d->characters[i]->monster->tunneling = 0;
        //d->characters[i]->monster->erratic = 1;

        d->characters[i]->speed = rand() % 16 + 5;
        d->characters[i]->display_char = get_monster_char(d->characters[i]);
    }
    int pcRoomNum;
    int totalArea = 0;
    for(int i = 0; i < d->num_rooms; ++i)
    {
        if (in_room(d->rooms[i],d->pc)){
            pcRoomNum = i;
        } else {
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
    //set current space to null
    d->character_map[c->pos[dim_y]][c->pos[dim_x]] = NULL;
    // if there's another character in destination, kill it
    if (d->character_map[c->pos[dim_y]+dy][c->pos[dim_x]+dx]){
        d->character_map[c->pos[dim_y]+dy][c->pos[dim_x]+dx]->living = 0;
    }
    //set future position and update the map
    c->pos[dim_y] = c->pos[dim_y]+dy;
    c->pos[dim_x] = c->pos[dim_x]+dx;
    d->character_map[c->pos[dim_y]][c->pos[dim_x]] = c;
}

void tun_rock_check(dungeon_t *d, character_t *c, int *dx, int *dy){
    if (d->map[c->pos[dim_y]+*dy][c->pos[dim_x]+*dx] == ter_wall){
        if (c->monster->tunneling) {
            int hardness = (hardnessxy(c->pos[dim_x] + *dx, c->pos[dim_y] + *dy)) - 85;
            hardnessxy(c->pos[dim_x] + *dx, c->pos[dim_y] + *dy) = MAX(0, hardness);
            if (is_open_space(d, c->pos[dim_y] + *dy, c->pos[dim_x] + *dx)) {
                mapxy(c->pos[dim_x] + *dx, c->pos[dim_y] + *dy) = ter_floor_hall;
            } else {
                *dx = *dy = 0;
            }
        } else {
            *dx = *dy = 0;
        }
    }
}
//needs to be cleaned up
void erratic_move(character_t *c,dungeon_t *d){
    int moved = 0,counter = 0,dx,dy;
    while(!moved) {
        if (counter > 500) {return;} //if it cant find a move, give up
        counter++;
        dx = (rand() % 3) - 1;
        dy = (rand() % 3) - 1;
        //wall and tunneling check dont need to happen anymore but continue still needs to be present
        if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) == ter_wall) {
            if(c->monster->tunneling) {
                tun_rock_check(d,c,&dx,&dy);
            } else {
                continue;
            }
        }
        if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) != ter_wall_immutable) {
            final_move(c, d, dx, dy);
            moved = 1;
        }
    }
}

void move_monster(character_t *c, dungeon_t *d)
{
    int sees_player = 0,dx = 0,dy = 0;

    dif_t dif;
    dif.x = 0;
    dif.y = 0;
    // 50% that if a monster is erratic it make an erratic move
    if (c->monster->erratic && rand() % 2) {
        erratic_move(c,d);
    }
    //if the player is seen
    if (c->monster->telepath || bresenham_LOS(d,c))
    {
        sees_player = 1;
        // can probably rewrite to use current pc location
        c->monster->pc_last_loc[dim_x] = d->pc.pos[dim_x];
        c->monster->pc_last_loc[dim_y] = d->pc.pos[dim_y];
        get_monster_path(c,d);

    }
    if (c->monster->intelligent) {
        //test if path_to_pc has been filled
        if (c->monster->path_to_pc[0][0].cost == INT_MAX) {
            uint32_t cost = INT_MAX;
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    if (c->monster->path_to_pc[c->pos[dim_y] + y][c->pos[dim_x] + x].cost < cost) {
                        dy = y;
                        dx = x;
                        cost = c->monster->path_to_pc[c->pos[dim_y] + y][c->pos[dim_x] + x].cost;
                    }
                }
            }
            tun_rock_check(d,c,&dx,&dy);
        }
    }
    else if (sees_player) {
         // sees player but non monster->intelligent
            if(c->monster->tunneling){
                bresenham_move(d,c,&dif);
                dx = dif.x;
                dy = dif.y;
            }
            else {
                move_line(d, c, &dif);
                dx = dif.x;
                dy = dif.y;
            }
        tun_rock_check(d,c,&dx,&dy);
    }
    if (mapxy(c->pos[dim_x]+dx,c->pos[dim_y]+dy) != ter_wall_immutable) {
        final_move(c, d, dx, dy);
    }
}

int bresenham_LOS(dungeon_t *d,character_t *c)
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "monster.h"
#include "dungeon.h"
#include "heap.h"

static int32_t monster_path_cmp(const void *key, const void *with) {
    return ((monster_path_t *) key)->cost - ((monster_path_t *) with)->cost;
}
char get_display_char(character_t *c)
{
    if(c->pc){return (char)'@';}

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

void dijkstra_non_tunneling(dungeon_t *d)
{
  static monster_path_t *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
      for (y = 0; y < DUNGEON_Y; y++)
      {
          for (x = 0; x < DUNGEON_X; x++)
          {
              d->non_tun_path[y][x].pos[dim_y] = y;
              d->non_tun_path[y][x].pos[dim_x] = x;
          }
      }
      initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
          d->non_tun_path[y][x].cost = INT_MAX;
      }
  }

  d->non_tun_path[d->characters[0].y][d->characters[0].x].cost = 0;

  heap_init(&h, monster_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
          if (mapxy(x, y) > ter_floor) {
              d->non_tun_path[y][x].hn = heap_insert(&h, &d->non_tun_path[y][x]);
          }
          else {
              d->non_tun_path[y][x].hn = NULL;
          }
      }
  }

  while ((p = heap_remove_min(&h))) {
      p->hn = NULL;

      for(int i = -1; i <= 1; ++i){
          for(int j = -1; j <= 1; ++j){
              if ((d->non_tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].hn) &&
              (d->non_tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].cost >
              p->cost + 1) && (p->cost != INT_MAX)) {
                  d->non_tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].cost =
		            p->cost + 1;
                  heap_decrease_key_no_replace(&h, d->non_tun_path[p->pos[dim_y] + j]
                    [p->pos[dim_x] + i].hn);
              }
          }
      }
  }
}

void dijkstra_tunneling(dungeon_t *d)
{
    static monster_path_t *p;
    static uint32_t initialized = 0;
    heap_t h;
    uint32_t x, y;
    // initialize array of paths
    if (!initialized) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                d->tun_path[y][x].pos[dim_y] = y;
                d->tun_path[y][x].pos[dim_x] = x;
            }
        }
        initialized = 1;
    }
    
    //set each path cost to infinity
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            d->tun_path[y][x].cost = INT_MAX;
        }
    }
    //set PC location cost to 0
    d->tun_path[d->characters[0].y][d->characters[0].x].cost = 0;

    heap_init(&h, monster_path_cmp, NULL);

    //if it is not a border wall, add it to the heap
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            if (mapxy(x, y) != ter_wall_immutable) {
                d->tun_path[y][x].hn = heap_insert(&h, &d->tun_path[y][x]);
            } else {
                d->tun_path[y][x].hn = NULL;
            }
        }
    }

    while ((p = heap_remove_min(&h))) {
        p->hn = NULL;
	

	//iterate through neighbors are if cost is more than cost from node, update
	for(int i = -1; i <= 1; ++i){
            for(int j = -1; j <= 1; ++j){
                if ((d->tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].hn) &&
                    (d->tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].cost >
                     p->cost + 1 + (hardnesspair(p->pos)/85))) {
                    d->tun_path[p->pos[dim_y] + j][p->pos[dim_x] + i].cost =
		      p->cost + 1 + (hardnesspair(p->pos)/85);
                    heap_decrease_key_no_replace(&h, d->tun_path[p->pos[dim_y] + j]
                    [p->pos[dim_x] + i].hn);
                }
            }
        }
    }
}

void move_monster(character_t *c, dungeon_t *d)
{
    int sees_player = 0,dx,dy;

    dif_t dif;
    dif.x = 0;
    dif.y = 0;

    void final_move(character_t *c, dungeon_t *d)
    {
        d->character_map[c->y][c->x] = NULL;
        if (d->character_map[c->y+dy][c->x+dx])
        {
            d->character_map[c->y+dy][c->x+dx]->living = 0;
        }
        // if collision is with the PC
        else if (c->y+dy == d->characters[0].y && c->x+dx == d->characters[0].x)
        {
            d->characters[0].living = 0;
        }
        d->character_map[c->y+dy][c->x+dx] = c;
        c->y = c->y+dy;
        c->x = c->x+dx;
    }
    if (c->monster->erratic) {

        if (rand() % 2) {
            int moved = 0,counter = 0;
            while(!moved) {
                if (counter > 500) {return;} //if it cant find a move, give up
                counter++;
                dx = (rand() % 3) - 1;
                dy = (rand() % 3) - 1;
                if (mapxy(c->x+dx,c->y+dy) == ter_wall) {
                    if(c->monster->tunneling) {
                        hardnessxy(c->x+dx,c->y+dy) = hardnessxy(c->x+dx,c->y+dy)-85;
                        if (hardnessxy(c->x+dx,c->y+dy) <= 0) {
                            hardnessxy(c->x+dx,c->y+dy) = 0;
                            mapxy(c->x+dx,c->y+dy) = ter_floor_hall;
                        }
                        else{return;} //hit wall but didn't break
                    }
                    else {continue;} //non-tunnel monster or immutable wall hit
                }
                if (mapxy(c->x+dx,c->y+dy) != ter_wall_immutable) {
                    final_move(c, d);
                    moved = 1;
                }
            }
            return;
        }
    }
    if (c->monster->telepath || bresenham_LOS(d,c->monster,&dif))
    {
        sees_player = 1;
        c->monster->pc_last_loc[dim_x] = d->characters[0].x;
        c->monster->pc_last_loc[dim_y] = d->characters[0].y;
    }
    if (sees_player || c->monster->intelligent) {
        if (c->monster->intelligent) {
            if (c->monster->pc_last_loc[dim_x]) {
                if (c->monster->tunneling) {
                    //dx dy based on tunnel dist map
                } else // intelligent non tunneling
                {
                    //dx dy based on non tunnel dist map
                }
            }
            else {dx = 0; dy = 0;}
            dx=0;
            dy=0;
        }
        else // non intelligent
        {
            dx = dif.x;
            dy = dif.y;
        }
        if (mapxy(c->x+dx,c->y+dy) == ter_wall)
        {
            if (c->monster->tunneling)
            {
                hardnessxy(c->x+dx,c->y+dy) = hardnessxy(c->x+dx,c->y+dy)-85;
                if (hardnessxy(c->x+dx,c->y+dy) <= 0)
                {
                    hardnessxy(c->x+dx,c->y+dy) = 0;
                    mapxy(c->x+dx,c->y+dy) = ter_floor_hall;
                }
                else {
                    hardnessxy(m->x+dx,m->y+dy) = hardness;
                    return;
                } //tunneling monsters hit wall but didnt break it
            }
            else{return;} // non-tunneling monster hit a wall
        }
        if (mapxy(c->x+dx,c->y+dy) != ter_wall_immutable) {
            final_move(c, d);
        }
    }


}


int bresenham_LOS(dungeon_t *d,monster_t *m, dif_t *dif)
{
    int x0 = m->x;
    int y0 = m->y;
    int x1 = d->characters[0].x;
    int y1 = d->characters[0].y;

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

void bresenham_move(dungeon_t *d,monster_t *m, dif_t *dif)
{
    int x0 = m->x;
    int y0 = m->y;
    int x1 = d->pc.x;
    int y1 = d->pc.y;

    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    e2 = err * 2;
    if(e2 >= dy){
        err += dy;
        x0 += sx;
        dif->x = sx;
    }
    if(e2 <= dx){
        err += dx;
        y0 += sy;
        dif->y = sy;
    }
}



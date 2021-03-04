#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "monster.h"
#include "dungeon.h"
#include "heap.h"

static int32_t monster_path_cmp(const void *key, const void *with) {
    return ((monster_path_t *) key)->cost - ((monster_path_t *) with)->cost;
}
char get_display_char(monster_t *m)
{
    char binary_char[4] = {'0'+(char)m->erratic,
                           '0'+(char)m->tunneling,
                           '0'+(char)m->telepath,
                           '0'+(char)m->intelligent};

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

  d->non_tun_path[d->pc.y][d->pc.x].cost = 0;

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
    d->tun_path[d->pc.y][d->pc.x].cost = 0;

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

int bresenham_LOS(dungeon_t *d, int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    mapxy(x0, y0) = ter_debug;
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


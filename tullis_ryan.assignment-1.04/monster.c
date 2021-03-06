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
    if(m->pc == 1){return (char)'@';}

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


    void move_line(dungeon_t *d, monster_t *m, dif_t *dif)
    {
        if(m->x < m->pc_last_loc[dim_x]){
            dif->x = 1;
        }
        else if(m->x > m->pc_last_loc[dim_x]){
            dif->x = -1;
        }
        else{
            dif->x = 0;
        }

        if(m->y < m->pc_last_loc[dim_y]){
            dif->y = 1;
        }
        else if(m->y > m->pc_last_loc[dim_y]){
            dif->y = -1;
        }
        else{
            dif->y = 0;
        }

        if(d->map[m->y + dif->y][m->x + dif->x] == ter_wall || d->map[m->y + dif->y][m->x + dif->x] == ter_wall_immutable)
        {
            if(d->map[m->y][m->x + dif->x] == ter_wall || d->map[m->y][m->x + dif->x] == ter_wall_immutable){
                dif->x = 0;
            }
            else if(d->map[m->y + dif->y][m->x ] == ter_wall || d->map[m->y + dif->y][m->x] == ter_wall_immutable){
                dif->y = 0;
            }
        }
    }


void final_move(monster_t *m, dungeon_t *d,int dx,int dy)
{
    d->monster_map[m->y][m->x] = NULL;
    if (d->monster_map[m->y+dy][m->x+dx])
    {
        d->monster_map[m->y+dy][m->x+dx]->living = 0;
    }
        // if collision is with the PC
    else if (m->y+dy == d->monsters[0].y && m->x+dx == d->monsters[0].x)
    {
        d->monsters[0].living = 0;
    }
    d->monster_map[m->y+dy][m->x+dx] = m;
    m->y = m->y+dy;
    m->x = m->x+dx;
}

void move_monster(monster_t *m, dungeon_t *d)
{
    int sees_player = 0,dx,dy;

    dif_t dif;
    dif.x = 0;
    dif.y = 0;

    if (m->erratic) {
        if (rand() % 2) {
            int moved = 0,counter = 0;
            while(!moved) {
                if (counter > 500) {return;} //if it cant find a move, give up
                counter++;
                dx = (rand() % 3) - 1;
                dy = (rand() % 3) - 1;
                if (mapxy(m->x+dx,m->y+dy) == ter_wall) {
                    if(m->tunneling) {
                        int hardness = hardnessxy(m->x+dx,m->y+dy);
                        hardness = hardness-85;
                        if (hardness <= 0)
                        {
                            hardnessxy(m->x+dx,m->y+dy) = 0;
                            mapxy(m->x+dx,m->y+dy) = ter_floor_hall;
                        }
                        else {
                            hardnessxy(m->x+dx,m->y+dy) = hardness;
                            return;
                        } //tunneling monsters hit wall but didnt break it
                    }
                    else {continue;} //non-tunnel monster or immutable wall hit
                }
                if (mapxy(m->x+dx,m->y+dy) != ter_wall_immutable) {
                    final_move(m, d, dx, dy);
                    moved = 1;
                }
            }
            return;
        }
    }
    if (m->telepath || bresenham_LOS(d,m,&dif))
    {
        sees_player = 1;
        m->pc_last_loc[dim_x] = d->pc.x;
        m->pc_last_loc[dim_y] = d->pc.y;
    }
    if (sees_player || m->intelligent) {
        if (m->intelligent) {
            if (m->pc_last_loc[dim_x]) {
                if (m->tunneling) {
                    //dx dy based on tunnel dist map
                    uint32_t cost = INT_MAX;
                    for (int i = -1; i <= 1; i++){
                        for (int j = -1; j <= 1; j++){
                            if (d->tun_path[m->y+i][m->x+j].cost < cost){
                                dy = i;
                                dx = j;
                                cost = d->tun_path[m->y+i][m->x+j].cost;
                            }
                        }
                    }
                } else // intelligent non tunneling
                {
                    //dx dy based on non tunnel dist map
                    uint32_t cost = INT_MAX;
                    for (int i = -1; i <= 1; i++){
                        for (int j = -1; j <= 1; j++){
                            if (d->non_tun_path[m->y+i][m->x+j].cost < cost){
                                dy = i;
                                dx = j;
                                cost = d->non_tun_path[m->y+i][m->x+j].cost;
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
        else // non intelligent
        {
            if(m->tunneling){
                bresenham_move(d,m,&dif);
                dx = dif.x;
                //printf("dx: %d\n",dif.x);
                dy = dif.y;
            }
            else{
                move_line(d,m,&dif);
                dx = dif.x;
                //printf("dx: %d\n",dif.x);
                dy = dif.y;
            }
        }
        if (mapxy(m->x+dx,m->y+dy) == ter_wall)
        {
            if (m->tunneling)
            {
                int hardness = hardnessxy(m->x+dx,m->y+dy);
                hardness = hardness-85;
                if (hardness <= 0)
                {
                    hardness = 0;
                    mapxy(m->x+dx,m->y+dy) = ter_floor_hall;
                }
                else {
                    hardnessxy(m->x+dx,m->y+dy) = hardness;
                    return;
                } //tunneling monsters hit wall but didnt break it
            }
            else{return;} // non-tunneling monster hit a wall
        }
        if (mapxy(m->x+dx,m->y+dy) != ter_wall_immutable) {
            final_move(m, d, dx, dy);
        }
    }
}


int bresenham_LOS(dungeon_t *d,monster_t *m, dif_t *dif)
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



#ifndef MONSTER_H
# define MONSTER_H

#include "dungeon.h"

void dijkstra_non_tunneling(dungeon_t *d);
void dijkstra_tunneling(dungeon_t *d);
void bresenham_monsters(dungeon_t *d, int x0, int y0, int x1, int y1);

#endif

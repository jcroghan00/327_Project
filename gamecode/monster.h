#ifndef MONSTER_H
# define MONSTER_H

#include "dungeon.h"

void dijkstra_non_tunneling(dungeon_t *d);
void dijkstra_tunneling(dungeon_t *d);

#endif

#ifndef PATH_H
# define PATH_H

#include "dungeon.h"

/*I don't know why this doesn't work
typedef struct dungeon{
}dungeon_t;
 */

void dijkstra_non_tunneling(dungeon_t *d);
void dijkstra_tunneling(dungeon_t *d);

#endif

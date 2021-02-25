#include <stdint.h>

struct dungeon{
    struct dungeon_t *d;
};

void dijkstra_normal(dungeon *d);
void dijkstra_tunneling(dungeon *d);
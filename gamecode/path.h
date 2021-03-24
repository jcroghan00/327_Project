#ifndef PATH_H
# define PATH_H


#include <stdint.h>

typedef struct heap_node heap_node_t;
class Dungeon;

class Corridor_Path {
public:
    heap_node_t *hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
};

int32_t corridor_path_cmp(const void *key, const void *with);
void dijkstra_non_tunneling(Dungeon *d);
void dijkstra_tunneling(Dungeon *d);

#endif

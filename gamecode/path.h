#ifndef PATH_H
# define PATH_H


#include <stdint.h>

typedef struct heap_node heap_node_t;
typedef struct dungeon dungeon_t;

typedef struct corridor_path {
    heap_node_t *hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
} corridor_path_t;

int32_t corridor_path_cmp(const void *key, const void *with);
void dijkstra_non_tunneling(dungeon_t *d);
void dijkstra_tunneling(dungeon_t *d);

#endif

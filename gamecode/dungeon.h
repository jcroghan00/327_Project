#ifndef DUNGEON_H
# define DUNGEON_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <vector>
#include "dim.h"
#include "config.h"
#include <ncurses.h>


 class Character;
 class Monster;
 class PC;
 class Windows;
 class Object;
typedef struct heap heap_t;
typedef struct heap_node heap_node_t;

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define pcmappair(pair) (d->pc->pc_map[pair[dim_y]][pair[dim_x]])
#define vismonsterpair(pair) (d->pc->vis_monsters[pair[dim_y]][pair[dim_x]])
#define vismonsterxy(x,y) (d->pc->vis_monsters[y][x])
#define visobjectmappair(pair) (d->pc->visObj[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define dhardnesspair(pair) (hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])
#define character_mappair(pair) (d->character_map[pair[dim_y]][pair[dim_x]])
#define character_mapxy(x, y) (d->character_map[y][x])
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_stairs,
  ter_stairs_up,
  ter_stairs_down
} terrain_type_t;

class Room {
public:
  pair_t position;
  pair_t size;
};

class Stair{
public:
  pair_t position;
  terrain_type_t direction;
};

class Monster_Path {
public:
    heap_node_t *hn;
    //TODO change to a pair_t
    uint8_t pos[2];
    uint32_t cost;
};

class Dungeon {
public:
    uint32_t num_rooms;
    Room *rooms;
    std::vector<Monster*> monsters;
    Stair *stairs;
    terrain_type_t map[DUNGEON_Y][DUNGEON_X];
    Character *character_map[DUNGEON_Y][DUNGEON_X];
    uint8_t hardness[DUNGEON_Y][DUNGEON_X];
    PC *pc;
    int num_monsters;
    Monster_Path non_tun_path[DUNGEON_Y][DUNGEON_X];
    Monster_Path tun_path[DUNGEON_Y][DUNGEON_X];
    Windows *windows;
    int numObjects;
    Object **objects;
    Object *objMap[DUNGEON_Y][DUNGEON_X];
    int fow;
    Dungeon(int numMon);
    void gen_monsters();
    uint32_t is_open_space(int16_t y, int16_t x);
    int empty_dungeon();
    void new_dungeon(heap_t *h);
protected:
    uint32_t adjacent_to_room(int16_t y, int16_t x);
    void dijkstra_corridor(pair_t from, pair_t to);
    void dijkstra_corridor_inv(pair_t from, pair_t to);
    int smoothhardness(Dungeon *);
    int make_rooms();
    int place_rooms();
    int connect_two_rooms(Room *r1, Room *r2);
    int create_cycle();
    int connect_rooms();
    void place_stairs();
};

uint32_t in_room(Room r, Character *c);
uint32_t is_open_space(Dungeon *d, int16_t y, int16_t x);
int gen_dungeon(Dungeon *d);
void render_ncurses(Dungeon *d, WINDOW *scr,int render_items);
void render_fow(Dungeon *d);
void render_dungeon(Dungeon *d);
void render_teleport_select(Dungeon *d, heap_t *h);
void render(Dungeon *d);
void delete_dungeon(Dungeon *d, heap_t *h);
int load_dungeon(Dungeon *d);
int save_dungeon(Dungeon *d);
void new_dungeon(Dungeon *d, heap_t *h);

#endif

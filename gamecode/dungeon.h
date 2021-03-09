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

#include "dim.h"
#include "character.h"
#include "heap.h"

#define DUNGEON_X              80
#define DUNGEON_Y              21
#define MIN_ROOMS              6
#define MAX_ROOMS              10
#define ROOM_MIN_X             4
#define ROOM_MIN_Y             3
#define ROOM_MAX_X             20
#define ROOM_MAX_Y             15
#define PC_SPEED               10
#define SEMANTIC_FILE_MARKER   "RLG327-S2021"
#define SAVE_DIR               ".rlg327"
#define SAVE_FILE              "dungeon"
#define FILE_VERSION           5

 typedef struct monster monster_t;
 typedef struct pc pc_t;

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
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

typedef struct room {
  pair_t position;
  pair_t size;
} room_t;

typedef struct stair{
  pair_t position;
  //down=0 up =1
  int up_down;
}stair_t;

typedef struct monster_path {
    heap_node_t *hn;
    uint8_t pos[2];
    int32_t cost;
} monster_path_t;

typedef struct dungeon {
  uint32_t num_rooms;
  room_t *rooms;
  character_t **characters;
  stair_t stairs[255];
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  character_t *character_map[DUNGEON_Y][DUNGEON_X];
   /*Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performance   *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.*/
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint16_t stairs_up;
  uint16_t stairs_down;
  character_t pc;
  int num_monsters;
  monster_path_t non_tun_path[DUNGEON_Y][DUNGEON_X];
  monster_path_t tun_path[DUNGEON_Y][DUNGEON_X];
} dungeon_t;

uint32_t in_room(room_t r, character_t c);
uint32_t is_open_space(dungeon_t *d, int16_t y, int16_t x);
int gen_dungeon(dungeon_t *d);
void render_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
int load_dungeon(dungeon_t *d);
void init_dungeon(dungeon_t *d);
int save_dungeon(dungeon_t *d);
int play_game(dungeon_t *d);

#endif

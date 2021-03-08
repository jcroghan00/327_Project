#ifndef GAMECODE_CHARACTER_H
#define GAMECODE_CHARACTER_H

#include "dim.h"

typedef struct dungeon dungeon_t;
typedef struct pc pc_t;
typedef struct monster monster_t;


typedef struct character {
    char display_char;
    pair_t pos;
    pc_t *pc;
    monster_t *monster;
    uint32_t turn;
    uint32_t sd;
    int living;
}character_t;

#endif

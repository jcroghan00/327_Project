#ifndef CHARACTER_H
# define CHARACTER_H

#include <stdint.h>
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
    uint32_t speed;
}character_t;

void define_characters(dungeon_t *d);
int32_t character_cmp(const void *key, const void *with);

#endif

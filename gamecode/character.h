#ifndef CHARACTER_H
# define CHARACTER_H

#include <stdint.h>
#include "dim.h"

class Dungeon;
class Pc;
class Monster;

class Character {
public:
    char display_char;
    pair_t pos;
    Pc *pc;
    Monster *monster;
    uint32_t turn;
    uint32_t sd;
    int living;
    uint32_t speed;
};

void define_characters(Dungeon *d);
int32_t character_cmp(const void *key, const void *with);
void delete_characters(Character **c, Dungeon *d);

#endif



#ifndef GAMECODE_RLG327_H
#define GAMECODE_RLG327_H
#include <vector>
#include "object.h"
#include "Monster.h"
class Dungeon;

typedef struct heap heap_t;

extern vector<Monstertype> monster_types;
extern vector<ObjectType> object_types;

void play_game(Dungeon *d, heap_t *h);
#endif //GAMECODE_RLG327_H

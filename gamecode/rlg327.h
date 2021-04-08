

#ifndef GAMECODE_RLG327_H
#define GAMECODE_RLG327_H
#include <vector>
class Dungeon;
class Monstertype;
class Objecttype;
typedef struct heap heap_t;

extern vector<Monstertype> monster_types();
extern vector<Objecttype> object_types();

void play_game(Dungeon *d, heap_t *h);
#endif //GAMECODE_RLG327_H

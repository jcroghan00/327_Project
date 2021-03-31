#ifndef GAMECODE_MONSTER_H
#define GAMECODE_MONSTER_H
#include "dungeon.h"
#include "Character.h"

class Monster: public Character {
public:
    Monster();
    int intelligent;
    int telepath;
    int tunneling;
    int erratic;
    pair_t pc_last_loc;
    Monster_Path path_to_pc[DUNGEON_Y][DUNGEON_X];
    pair_t last_seen;
};

class Dif{
public:
    int x;
    int y;
};

void update_last_seen(Dungeon *d);
void monster_list(Dungeon *d);
int gen_monsters(Dungeon *d);
void move_monster(Monster *m, Dungeon *d);

//can probably be moved the Character
int bresenham_LOS(Dungeon *d,Character *c);
void bresenham_move(Dungeon *d,Character *c, Dif *dif);


#endif

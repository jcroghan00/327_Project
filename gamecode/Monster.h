#ifndef GAMECODE_MONSTER_H
#define GAMECODE_MONSTER_H

#include "dungeon.h"
#include "Character.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>

using namespace std;

typedef struct abil{
    int SMART;
    int TELE;
    int TUNNEL;
    int ERRATIC;
    int PASS;
    int PICKUP;
    int DESTROY;
    int UNIQ;
    int BOSS;
}abil_t;

class Dif{
public:
    int x;
    int y;
};

class Monstertype{
public:
    string name;
    string desc;
    string color;
    Dice speed;
    Dice hp;
    Dice dam;
    string abil;
    abil_t abilities;
    string symb;
    int rrty;
    int inUse;

    void print();
    Monster* createMonster();
    int getColor();
};

class Monster: public Character {
public:
    Monster();
    //pair_t last_seen;
    pair_t destination;
    int attack_monster(Dungeon *d,int damage);
    void move_monster(Dungeon *d);
    int create_monster(Monstertype *t);
    void isMonster(Dungeon *d, Dif *dif);

protected:
    //struct of the monsters abilities
    abil_t abil;
    // distance map to destination tun/non tun by monster
    Monster_Path path_to_dest[DUNGEON_Y][DUNGEON_X];

private:
    Monstertype* pick_type();
    void get_monster_path(Dungeon *d);
    void erratic_move(Dungeon *d);
    void tun_rock_check(Dungeon *d, int *dx, int *dy);
    void move_line(Dungeon *d, Dif *dif);
    void final_move(Dungeon *d,int dx,int dy);
};

void update_last_seen(Dungeon *d);
void monster_list(Dungeon *d);
int gen_monsters(Dungeon *d);
extern vector<Monstertype> monster_types;
int new_gen_monster(Dungeon *d);

#endif

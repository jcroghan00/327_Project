#ifndef GAMECODE_MONSTER_H
#define GAMECODE_MONSTER_H

#include "dungeon.h"
#include "Character.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>
#include <vector>

using namespace std;

class Dif{
public:
    int x;
    int y;
};
class Monster: public Character {
public:
    Monster();
    pair_t last_seen;
    void move_monster(Dungeon *d);
protected:
    int intelligent;
    int telepath;
    int tunneling;
    int erratic;
    pair_t pc_last_loc;
    Monster_Path path_to_pc[DUNGEON_Y][DUNGEON_X];

private:
    void set_monster_char();
    void get_monster_path(Dungeon *d);
    void erratic_move(Dungeon *d);
    void tun_rock_check(Dungeon *d, int *dx, int *dy);
    void move_line(Dungeon *d, Dif *dif);
    void final_move(Dungeon *d,int dx,int dy);
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
    string symb;
    int rrty;

    void print();
};

extern vector<Monstertype> monster_types();

void update_last_seen(Dungeon *d);
void monster_list(Dungeon *d);
int gen_monsters(Dungeon *d);

#endif

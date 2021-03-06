#ifndef GAMECODE_CHARACTER_H
#define GAMECODE_CHARACTER_H

#include <stdint.h>
#include <string>
#include "dim.h"

class Dungeon;
class Pc;
class Monster;
class Dif;

class Dice{
public:
    int base;
    int numDice;
    int numSides;

    int roll();
    void print();
};
using namespace std;
class Character {
protected:
    uint32_t sd;
    uint32_t turn;
    int sees_player(Dungeon *d);
    void bresenham_move(Dungeon *d, Dif *dif);
    int living;
public:
    Character();
    Character(int speed);
    uint32_t getSd();
    char display_char;
    void setSd(uint32_t sd);
    void setDisplayChar(char c);
    char getDisplayChar();
    void setLiving(int l);
    int isLiving();
    uint32_t getTurn();
    void setSpeed(int speed);
    uint32_t getSpeed();
    void setNextTurn();
    int isPc();
    int dispColor;
    string name;
    string desc;
    pair_t pos;
    uint32_t speed;
    int hitpoints;
    Dice damage;
    Pc *pc;
};

int32_t character_cmp(const void *key, const void *with);

#endif //GAMECODE_CHARACTER_H

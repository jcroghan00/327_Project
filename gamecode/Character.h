
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
    string name;
    string desc;
    int dispColor;
    uint32_t speed;
    uint32_t hitpoints;
    Dice damage;
    char display_char;
    uint32_t sd;
    uint32_t turn;
    int bresenham_LOS(Dungeon *d);
    void bresenham_move(Dungeon *d, Dif *dif);
public:
    Character();
    Character(int speed);
    uint32_t getSd();
    void setSd(uint32_t sd);
    void setDisplayChar(char c);
    char getDisplayChar();
    void setLiving(int l);
    int isLiving();
    void setTurn(int turn);
    uint32_t getTurn();
    void setSpeed(int speed);
    uint32_t getSpeed();
    void setNextTurn();
    int living;

    pair_t pos;
    Pc *pc;
};

int32_t character_cmp(const void *key, const void *with);
void delete_characters(Character **c, Dungeon *d);

#endif //GAMECODE_CHARACTER_H

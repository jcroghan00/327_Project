
#ifndef GAMECODE_CHARACTER_H
#define GAMECODE_CHARACTER_H

#include <stdint.h>
#include "dim.h"


class Dungeon;
class Pc;
class Monster;
class Dif;

class Character {
protected:
    uint32_t sd;
    char display_char;
    uint32_t turn;
    uint32_t speed;
    uint32_t hitpoints;
    uint32_t damage;
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

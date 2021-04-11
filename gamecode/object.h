#ifndef GAMECODE_OBJECT_H
#define GAMECODE_OBJECT_H
#include "Character.h"

class Dice;
class Dungeon;
class Object;

typedef struct type{
    unsigned int WEAPON:1;
    unsigned int OFFHAND:1;
    unsigned int RANGED:1;
    unsigned int ARMOR:1;
    unsigned int HELMET:1;
    unsigned int CLOAK:1;
    unsigned int GLOVES:1;
    unsigned int BOOTS:1;
    unsigned int RING:1;
    unsigned int AMULET:1;
    unsigned int LIGHT:1;
    // below this is not considered equipment
    unsigned int SCROLL:1;
    unsigned int BOOK:1;
    unsigned int FLASK:1;
    unsigned int GOLD:1;
    unsigned int AMMUNITION:1;
    unsigned int FOOD:1;
    unsigned int WAND:1;
    unsigned int CONTAINER:1;
    
}type_t;

class ObjectType{
public:
    Object* createObj();
    int getColor();

    string name;
    string desc;
    type_t type;
    string color;
    Dice hit;
    Dice dam;
    Dice dodge;
    Dice def;
    Dice weight;
    Dice speed;
    Dice attr;
    Dice val;
    int art;
    int rrty;
    int inUse;

    void print();
};

class Object{
    public:
        int createObj(ObjectType *t);
        void setObjChar();

    string name;
    string desc;
    type_t type;
    int color;
    int hit;
    Dice dam;
    int dodge;
    int def;
    int weight;
    int speed;
    int attr;
    int val;
    int art;
    int inUse;
    int rrty;
    char displayChar;
};

void gen_objects(Dungeon *d);

#endif //GAMECODE_OBJECT_H

//
// Created by ryan on 4/7/21.
//

#include "object.h"
#include "dungeon.h"
#include "rlg327.h"

#include <iostream>

using namespace std;

void ObjectType::print()
{
    cout << "NAME: " + this->name << endl;

    cout << "COLOR: " + this->color << endl;

    cout << "WEIGHT: ";
    this->weight.print();

    cout << "HIT: ";
    this->hit.print();

    cout << "DAM: ";
    this->dam.print();

    cout << "ATTR: ";
    this->attr.print();

    cout << "VAL: ";
    this->val.print();

    cout << "DODGE: ";
    this->dodge.print();

    cout << "DEF: ";
    this->def.print();

    cout << "SPEED: ";
    this->speed.print();

    cout << "DESC: " << endl;
    cout << this->desc;

    cout << "RRTY: ";
    cout << this->rrty << endl;

    cout << "ART: ";
    cout << this->art << endl;
    cout << "\n";
}

int Object::createObj(ObjectType *obj){

     name = obj->name;
     desc = obj->desc;
     type = obj->type;
     color = obj->getColor();
     hit  = obj->hit.roll();
     dam = obj->dam;
     dodge = obj->dodge.roll();
     def = obj->def.roll();
     weight = obj->weight.roll();
     speed = obj->speed.roll();
     attr = obj->attr.roll();
     val = obj->val.roll();
     art = obj->art;
     inUse = obj->inUse;
     rrty = obj->rrty;
     setObjChar();

     return 0;

}

void Object::setObjChar()
{
    if(type.AMMUNITION){
        displayChar = '/';
    }
    else if(type.AMULET){
        displayChar ='"';
    }
    else if(type.ARMOR){
        displayChar ='[';
    }
    else if(type.BOOK){
        displayChar ='?';
    }
    else if(type.BOOTS){
        displayChar ='\\';
    }
    else if(type.CLOAK){
        displayChar ='(';
    }
    else if(type.CONTAINER){
        displayChar ='%';
    }
    else if(type.FOOD){
        displayChar =',';
    }
    else if(type.FLASK){
        displayChar ='!';
    }
    else if(type.GLOVES){
        displayChar ='{';
    }
    else if(type.GOLD){
        displayChar ='$';
    }
    else if(type.HELMET){
        displayChar =']';
    }
    else if(type.LIGHT){
        displayChar ='_';
    }
    else if(type.OFFHAND){
        displayChar =')';
    }
    else if(type.RANGED){
        displayChar ='}';
    }
    else if(type.RING){
        displayChar ='=';
    }
    else if(type.SCROLL){
        displayChar ='~';
    }
    else if(type.WAND){
        displayChar ='-';
    }
    else if(type.WEAPON){
        displayChar ='|';
    }
}

int ObjectType::getColor(){
    string dispColor;

    for (int i = 0; i < (int)this->color.length();i++){
        if (this->color.at(i) == ' '){
            break;
        }
        dispColor.push_back(this->color.at(i));
    }

    if (dispColor == "RED"){
        return COLOR_RED;
    } else if (dispColor == "GREEN"){
        return COLOR_GREEN;
    }else if (dispColor == "BLUE"){
        return COLOR_BLUE;
    }else if (dispColor == "CYAN"){
        return COLOR_CYAN;
    }else if (dispColor == "YELLOW"){
        return COLOR_YELLOW;
    }else if (dispColor == "MAGENTA"){
        return COLOR_MAGENTA;
    }else{
        return COLOR_CYAN;
    }
}

void gen_objects(Dungeon *d){

    d->numObjects = rand() % (14 - 10 + 1) + 10;

    d->objects = (Object**)calloc((d->numObjects),sizeof(Object) * (d->numObjects));

    int i = 0;
    ObjectType num;

       for(i = 0; i < d->numObjects; i++)
    {
    do {

        num =  object_types.at(rand() % object_types.size());

        if(num.art && num.inUse){
            num.rrty = -1;
        }
        num.inUse = 1;
    } while (num.rrty < rand() % 100);
        d->objects[i] = num.createObj();
    }

    i = 0;

    while(i < d->numObjects){
        int x = rand() % 79;
        int y = rand() %21;

        if(d->map[y][x] > ter_floor){
            d->objMap[y][x] = d->objects[i];
            ++i;
        }


    }
}
Object* ObjectType::createObj(){
    Object *obj = new Object();
    obj->createObj(this);
    return obj;
}
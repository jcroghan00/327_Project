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
     color = obj->color;
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

     return 0;

}

void gen_objects(Dungeon *d){

    d->numObjects = rand() % (14 - 10 + 1) + 10;

    d->objects = (Object**)calloc((d->numObjects),sizeof(Object) * (d->numObjects));

    int i = 0;
    ObjectType num;

       for(i = 0; i < d->num_monsters; i++)
    {
    do {

        num =  object_types.at(rand() % object_types.size());

        if(num.art && num.inUse){
            num.rrty = -1;
        }
        num.inUse = 1;
        //TODO if its a uniq monster, make it not gen again
    } while (num.rrty < rand() % 100);

        d->objects[i] = num.createObj();
        // d->monsters[i]->setDisplayChar(mon.symb);

    }

    i = 0;

    while(i < d->numObjects){
        int x = rand() % 79;
        int y = rand() %21;

        if(d->map[y][x] != ter_wall_immutable && d->map[y][x] != ter_wall){
            d->objMap[dim_y + y][dim_x + x] = d->objects[i];
        }


    }

    


    




}
Object* ObjectType::createObj(){
    Object *obj = new Object();
    obj->createObj(this);
    return obj;
}
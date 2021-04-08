//
// Created by ryan on 4/7/21.
//

#include "object.h"
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
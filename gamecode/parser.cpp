//
// Created by ryan on 3/31/21.
//

#include "parser.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void monster_parser(){
    int num_monster_types = count_monster_types();
    cout << num_monster_types << endl;
    ifstream inFile("monsters.txt");

    if(!inFile){
        cerr << "cannot open \"monster.txt\" for output\n";
        return;
    }

    std::string metadata;
    getline(inFile, metadata);
    cout << metadata << endl;

    if(!metadata.compare("RLG327 MONSTER DESCRIPTION 1\n")){
        inFile.close();
        return;
    }

    inFile.close();
}
void item_parser(){

}

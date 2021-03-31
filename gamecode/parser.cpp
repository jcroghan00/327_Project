
#include "parser.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

void monster_parser(){
    char *filepath = (char *)malloc(sizeof(getenv("HOME")) + sizeof("/.rlg327/monster_desc.txt"));
    strcat(filepath, getenv("HOME"));
    strcat(filepath, "/.rlg327/monster_desc.txt");
    ifstream inFile(filepath);

    if(!inFile){
        cerr << "cannot open \"monster_desc.txt\" for output\n";
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

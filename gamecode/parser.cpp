
#include "parser.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

int count_types(std::string filepath, std::string delimiter)
{
    int num_types = 0;
    ifstream inFile(filename);

    std::string line;
    while(getline(inFile, line))
    {
        if(!line.compare(delimiter)){
            ++num_types;
        }
    }

    inFile.close();
    return num_types;
}

void monster_parser(){
    char *filepath = (char *)malloc(sizeof(getenv("HOME")) + sizeof("/.rlg327/monster_desc.txt"));
    strcat(filepath, getenv("HOME"));
    strcat(filepath, "/.rlg327/monster_desc.txt");

    int num_monster_types = count_types(filepath, "BEGIN MONSTER");
    cout << num_monster_types << endl;

    ifstream inFile(filepath);

    if(!inFile){
        cerr << "cannot open \"monster_desc.txt\" for output\n";
        return;
    }

    std::string metadata;
    getline(inFile, metadata);
    if(metadata.compare("RLG327 MONSTER DESCRIPTION 1")){
        inFile.close();
        return;
    }

    cout << metadata << endl;

    /*
    std::string line;
    while(getline(inFile, line))
    {
        cout << line << endl;
    }
     */
    inFile.close();
}
void item_parser(){

}

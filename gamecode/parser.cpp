
#include "parser.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>

using namespace std;

//hash values for use with the case statement
//the case statement must be able to see the values at compile time so now we have this.. yay
#define name 1052782581
#define symb -1147521460
#define color -1272051187
#define desc 1285376905
#define speed -1876027316
#define dam 2061701898
#define hp -941208014
#define abil -813300525
#define rrty -1614022120

int count_types(string filepath, string delimiter)
{
    int num_types = 0;
    ifstream inFile(filepath);

    string line;
    while(getline(inFile, line))
    {
        if(!line.compare(delimiter)){
            ++num_types;
        }
    }

    inFile.close();
    return num_types;
}


void monster_parser()
{
    char *filepath = (char *)malloc(sizeof(getenv("HOME")) + sizeof("/.rlg327/monster_desc.txt"));
    strcat(filepath, getenv("HOME"));
    strcat(filepath, "/.rlg327/monster_desc.txt");
    string delimiter = "d";

    int num_monster_types = count_types(filepath, "BEGIN MONSTER");
    cout << num_monster_types << endl;

    ifstream inFile(filepath);

    if(!inFile){
        cerr << "cannot open \"monster_desc.txt\", are you sure it's located in HOME/.rlg327/?" << endl;
        inFile.close();
        return;
    }

    string metadata;
    getline(inFile, metadata);
    if(!metadata.compare("RLG327 MONSTER DESCRIPTION 1\n")) {
        cerr << "metadata does not match \"RLG327 MONSTER DESCRIPTION 1\"" << endl;
        inFile.close();
        return;
    }

    hash<string> hasher;

    string line;
    while(getline(inFile, line))
    {
        if(!line.compare("END")){
            cout << "\n";
            continue;
        }
        else if(!line.compare("\n"))
        {
            continue;
        }

        string token = line.substr(0,line.find(" "));

        switch(static_cast<int>(hasher(token)))
        {
            case name:
            {
                string monster_name = line.substr(line.find(" ") + 1, line.find("\n"));

                cout << "Name: " + monster_name << endl;
                break;
            }

            case symb:
            {
                string monster_symb = line.substr(line.find(" ") + 1, line.find("\n"));

                cout << "Symbol: " + monster_symb << endl;
                break;
            }

            case color:
            {
                string monster_color = line.substr(line.find(" ") + 1, line.find("\n"));
                
                cout << "Color: " + monster_color << endl;
                break;
            }

            case desc:
            {
                string monster_desc;

                cout << "Description: " << endl;
                getline(inFile, line);
                while(line.compare("."))
                {
                    monster_desc = monster_desc + line + "\n";
                    getline(inFile, line);
                }
                cout << monster_desc;
                break;
            }

            case speed:
            {
                string monster_speed = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_speed.find("+");
                int delim2 = monster_speed.find("d");
                int delim3 = monster_speed.find("\n");

                string baseSpeed = monster_speed.substr(0, delim1);
                string numDice = monster_speed.substr(delim1 + 1, delim2 - (delim1 + 1));
                string numSides = monster_speed.substr(delim2 + 1, delim3);

                cout << "Base Speed: " + baseSpeed << endl;
                cout << "Num Dice: " + numDice << endl;
                cout << "Num Sides: " + numSides<< endl;


                cout << "Speed: " + monster_speed << endl;
                break;
            }

            case dam:
            {
                string monster_dam = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_dam.find("+");
                int delim2 = monster_dam.find("d");
                int delim3 = monster_dam.find("\n");

                string baseDam = monster_dam.substr(0, delim1);
                string numDice = monster_dam.substr(delim1 + 1, delim2 - (delim1 + 1));
                string numSides = monster_dam.substr(delim2 + 1, delim3);

                cout << "Base Damage: " + baseDam << endl;
                cout << "Num Dice: " + numDice << endl;
                cout << "Num Sides: " + numSides<< endl;

                cout << "Damage: : " + monster_dam << endl;
                break;
            }

            case hp:
            {
                string monster_hp = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_hp.find("+");
                int delim2 = monster_hp.find("d");
                int delim3 = monster_hp.find("\n");

                string baseHp = monster_hp.substr(0, delim1);
                string numDice = monster_hp.substr(delim1 + 1, delim2 - (delim1 + 1));
                string numSides = monster_hp.substr(delim2 + 1, delim3);

                cout << "Base Hp: " + baseHp << endl;
                cout << "Num Dice: " + numDice << endl;
                cout << "Num Sides: " + numSides<< endl;

                cout << "Health Points: " + monster_hp << endl;
                break;
            }

            case abil:
            {
                string monster_abil = line.substr(line.find(" ") + 1, line.find("\n"));

                cout << "Ability(s): " + monster_abil << endl;
                break;
            }

            case rrty:
            {
                string monster_rrty = line.substr(line.find(" ") + 1, line.find("\n"));

                cout << "Rarity: " + monster_rrty << endl;
                break;
            }
        }
    }
}


void item_parser()
{
    char *filepath = (char *)malloc(sizeof(getenv("HOME")) + sizeof("/.rlg327/monster_desc.txt"));
    strcat(filepath, getenv("HOME"));
    strcat(filepath, "/.rlg327/object_desc.txt");
    ifstream inFile(filepath);

    if(!inFile){
        cerr << "cannot open \"object_desc.txt\", are you sure it's located in HOME/.rlg327/?";
        return;
    }

}

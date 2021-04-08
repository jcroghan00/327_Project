
#include "parser.h"
#include "Monster.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>

using namespace std;

//hash values for use with the case statement
//the case statement must be able to see the values at compile time so now we have this.. yay
#define nameid 1052782581
#define symbid -1147521460
#define colorid -1272051187
#define descid 1285376905
#define speedid -1876027316
#define damid 2061701898
#define hpid -941208014
#define abilid -813300525
#define rrtyid -1614022120

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

    int num_monster_types = count_types(filepath, "BEGIN MONSTER");

    vector<Monstertype> monster_types(num_monster_types);

    ifstream inFile(filepath);

    free(filepath);
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

    int i = 0;
    string line;
    while(getline(inFile, line))
    {
        if(!line.compare("END")){
            ++i;
            continue;
        }
        else if(!line.compare("\n"))
        {
            continue;
        }

        string token = line.substr(0,line.find(" "));

        switch(static_cast<int>(hasher(token)))
        {
            case nameid:
            {
                monster_types.at(i).name = line.substr(line.find(" ") + 1, line.find("\n"));

                break;
            }

            case symbid:
            {
                monster_types.at(i).symb = line.substr(line.find(" ") + 1, line.find("\n"));

                break;
            }

            case colorid:
            {
                monster_types.at(i).color = line.substr(line.find(" ") + 1, line.find("\n"));

                break;
            }

            case descid:
            {
                string monster_desc;

                getline(inFile, line);
                while(line.compare("."))
                {
                    monster_desc = monster_desc + line + "\n";
                    getline(inFile, line);
                }

                monster_types.at(i).desc = monster_desc;
                break;
            }

            case speedid:
            {
                string monster_speed = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_speed.find("+");
                int delim2 = monster_speed.find("d");
                int delim3 = monster_speed.find("\n");

                monster_types.at(i).speed.base = stoi(monster_speed.substr(0, delim1));
                monster_types.at(i).speed.numDice = stoi(monster_speed.substr(delim1 + 1, delim2 - (delim1 + 1)));
                monster_types.at(i).speed.numSides = stoi(monster_speed.substr(delim2 + 1, delim3));

                break;
            }

            case damid:
            {
                string monster_dam = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_dam.find("+");
                int delim2 = monster_dam.find("d");
                int delim3 = monster_dam.find("\n");

                monster_types.at(i).dam.base = stoi(monster_dam.substr(0, delim1));
                monster_types.at(i).dam.numDice = stoi(monster_dam.substr(delim1 + 1, delim2 - (delim1 + 1)));
                monster_types.at(i).dam.numSides = stoi(monster_dam.substr(delim2 + 1, delim3));

                break;
            }

            case hpid:
            {
                string monster_hp = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = monster_hp.find("+");
                int delim2 = monster_hp.find("d");
                int delim3 = monster_hp.find("\n");

                monster_types.at(i).hp.base = stoi(monster_hp.substr(0, delim1));
                monster_types.at(i).hp.numDice = stoi(monster_hp.substr(delim1 + 1, delim2 - (delim1 + 1)));
                monster_types.at(i).hp.numSides = stoi(monster_hp.substr(delim2 + 1, delim3));

                break;
            }

            case abilid:
            {
                monster_types.at(i).abil = line.substr(line.find(" ") + 1, line.find("\n"));
                string temp = monster_types.at(i).abil ;
                int position;
                while(!temp.empty()){
                     position = temp.find(" ");

                    if(line.substr(0, position).compare("PICKUP")){
                        monster_types.at(i).pickup = 1;
                    }
                    if(line.substr(0, position).compare("ERRATIC")){
                        monster_types.at(i).erratic = 1;
                    }
                    if(line.substr(0, position).compare("TELE")){
                        monster_types.at(i).tele = 1;
                    }
                    if(line.substr(0, position).compare("UNIQ")){
                        monster_types.at(i).uniq = 1;                        
                    }
                    if(line.substr(0, position).compare("DESTROY")){
                        monster_types.at(i).destroy = 1;     
                    }
                    if(line.substr(0, position).compare("TUNNEL")){
                        monster_types.at(i).tunneling = 1;                        
                    }
                    if(line.substr(0, position).compare("BOSS")){
                        monster_types.at(i).boss= 1;                        
                    }
                    temp.erase(0, position + 1);

                }

                break;
            }

            case rrtyid:
            {
                monster_types.at(i).rrty = stoi(line.substr(line.find(" ") + 1, line.find("\n")));

                break;
            }
        }
    }
    for(int j = 0; j < num_monster_types; ++j)
    {
        monster_types.at(j).print();
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

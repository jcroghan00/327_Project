
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
                string abil_string = line.substr(line.find(" ") + 1, line.find("\n"));
                monster_types.at(i).abil = abil_string;

                vector<string> abils;

                size_t start;
                size_t end = 0;

                while ((start = abil_string.find_first_not_of(" ", end)) != string::npos)
                {
                    end = abil_string.find(" ", start);
                    abils.push_back(abil_string.substr(start, end - start));
                }

                monster_types.at(i).abilities.SMART = 0;
                monster_types.at(i).abilities.TELE = 0;
                monster_types.at(i).abilities.TUNNEL = 0;
                monster_types.at(i).abilities.ERRATIC = 0;
                monster_types.at(i).abilities.PASS = 0;
                monster_types.at(i).abilities.PICKUP = 0;
                monster_types.at(i).abilities.DESTROY = 0;
                monster_types.at(i).abilities.UNIQ = 0;
                monster_types.at(i).abilities.BOSS = 0;

                for(size_t j = 0; j < abils.size(); ++j)
                {
                    if(!abils.at(j).compare("SMART"))
                    {
                        monster_types.at(i).abilities.SMART = 1;
                    }
                    else if(!abils.at(j).compare("TELE"))
                    {
                        monster_types.at(i).abilities.TELE = 1;
                    }
                    else if(!abils.at(j).compare("TUNNEL"))
                    {
                        monster_types.at(i).abilities.TUNNEL = 1;
                    }
                    else if(!abils.at(j).compare("ERRATIC"))
                    {
                        monster_types.at(i).abilities.ERRATIC = 1;
                    }
                    else if(!abils.at(j).compare("PASS"))
                    {
                        monster_types.at(i).abilities.PASS = 1;
                    }
                    else if(!abils.at(j).compare("PICKUP"))
                    {
                        monster_types.at(i).abilities.PICKUP = 1;
                    }
                    else if(!abils.at(j).compare("DESTROY"))
                    {
                        monster_types.at(i).abilities.SMART = 1;
                    }
                    else if(!abils.at(j).compare("UNIQ"))
                    {
                        monster_types.at(i).abilities.UNIQ = 1;
                    }
                    else if(!abils.at(j).compare("BOSS"))
                    {
                        monster_types.at(i).abilities.BOSS = 1;
                    }
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


#include "parser.h"
#include "Monster.h"
#include "object.h"
#include "rlg327.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>
#include <sstream>

using namespace std;

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

void monster_parser()
{
    stringstream filepath;
    filepath << getenv("HOME");
    filepath << "/.rlg327/monster_desc.txt";

    int num_monster_types = count_types(filepath.str(), "BEGIN MONSTER");

    monster_types.resize(num_monster_types);

    ifstream inFile(filepath.str());


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
                while(temp.size() > 0){
                     position = temp.find(" ");
                     if(position == -1){
                         position = temp.size() +1;
                     }
                    cout << temp.substr(0, position) << endl;
                    cout << temp.substr(0, position).compare("SMART") << endl;
                    if(!temp.substr(0, position).compare("PICKUP")){
                        monster_types.at(i).abilities.PICKUP = 1;
                    }
                    else if(!temp.substr(0, position).compare("ERRATIC")){
                        monster_types.at(i).abilities.ERRATIC = 1;
                    }
                    else if(!temp.substr(0, position).compare("TELE")){
                        monster_types.at(i).abilities.TELE = 1;
                    }
                    else if(!temp.substr(0, position).compare("UNIQ")){
                        monster_types.at(i).abilities.UNIQ = 1;
                    }
                    else if(!temp.substr(0, position).compare("DESTROY")){
                        monster_types.at(i).abilities.DESTROY = 1;
                    }
                    else if(!temp.substr(0, position).compare("TUNNEL")){
                        monster_types.at(i).abilities.TUNNEL = 1;
                    }
                    else if(!temp.substr(0, position).compare("BOSS")){
                        monster_types.at(i).abilities.BOSS = 1;
                    }
                    else if(temp.substr(0, position).compare("SMART") == 0){
                        cout << "HELLO" << endl;
                        monster_types.at(i).abilities.SMART = 1;
                    }
                    else if(!temp.substr(0, position).compare("PASS")){
                        monster_types.at(i).abilities.PASS = 1;
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
    inFile.close();

    for(int j = 0; j < num_monster_types; ++j)
    {
        monster_types.at(j).print();
    }
}

#define typeid 1896164134
#define weightid 728966913
#define hitid -1679241014
#define attrid -14621479
#define valid -217068150
#define dodgeid 113340632
#define defid -972294821
#define artid 1711590550

void object_parser()
{
    stringstream filepath;
    filepath << getenv("HOME");
    filepath << "/.rlg327/object_desc.txt";

    int num_object_types = count_types(filepath.str(), "BEGIN OBJECT");

    object_types.resize(num_object_types);

    ifstream inFile(filepath.str());


    if (!inFile) {
        cerr << "cannot open \"object_desc.txt\", are you sure it's located in HOME/.rlg327/?" << endl;
        inFile.close();
        return;
    }

    string metadata;
    getline(inFile, metadata);
    if (!metadata.compare("RLG327 OBJECT DESCRIPTION 1\n")) {
        cerr << "metadata does not match \"RLG327 OBJECT DESCRIPTION 1\"" << endl;
        inFile.close();
        return;
    }

    hash<string> hasher;
    
    int i = 0;
    string line;
    while (getline(inFile, line)) {
        if (!line.compare("END")) {
            ++i;
            continue;
        } else if (!line.compare("\n")) {
            continue;
        }

        string token = line.substr(0, line.find(" "));

        switch (static_cast<int>(hasher(token))) {
            case nameid: {
                object_types.at(i).name = line.substr(line.find(" ") + 1, line.find("\n"));

                break;
            }

            case typeid: {
                break;
            }

            case colorid: {
                object_types.at(i).color = line.substr(line.find(" ") + 1, line.find("\n"));

                break;
            }

            case weightid: {
                string object_weight = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_weight.find("+");
                int delim2 = object_weight.find("d");
                int delim3 = object_weight.find("\n");

                object_types.at(i).weight.base = stoi(object_weight.substr(0, delim1));
                object_types.at(i).weight.numDice = stoi(object_weight.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).weight.numSides = stoi(object_weight.substr(delim2 + 1, delim3));

                break;
            }

            case hitid:
            {
                string object_hit = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_hit.find("+");
                int delim2 = object_hit.find("d");
                int delim3 = object_hit.find("\n");

                object_types.at(i).hit.base = stoi(object_hit.substr(0, delim1));
                object_types.at(i).hit.numDice = stoi(object_hit.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).hit.numSides = stoi(object_hit.substr(delim2 + 1, delim3));

                break;
            }

            case damid: {
                string object_dam = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_dam.find("+");
                int delim2 = object_dam.find("d");
                int delim3 = object_dam.find("\n");

                object_types.at(i).dam.base = stoi(object_dam.substr(0, delim1));
                object_types.at(i).dam.numDice = stoi(object_dam.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).dam.numSides = stoi(object_dam.substr(delim2 + 1, delim3));

                break;
            }

            case attrid: {
                string object_attr = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_attr.find("+");
                int delim2 = object_attr.find("d");
                int delim3 = object_attr.find("\n");

                object_types.at(i).attr.base = stoi(object_attr.substr(0, delim1));
                object_types.at(i).attr.numDice = stoi(object_attr.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).attr.numSides = stoi(object_attr.substr(delim2 + 1, delim3));

                break;
            }
                
            case valid: {
                string object_val = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_val.find("+");
                int delim2 = object_val.find("d");
                int delim3 = object_val.find("\n");

                object_types.at(i).val.base = stoi(object_val.substr(0, delim1));
                object_types.at(i).val.numDice = stoi(object_val.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).val.numSides = stoi(object_val.substr(delim2 + 1, delim3));

                break;
            }

            case dodgeid: {
                string object_dodge = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_dodge.find("+");
                int delim2 = object_dodge.find("d");
                int delim3 = object_dodge.find("\n");

                object_types.at(i).dodge.base = stoi(object_dodge.substr(0, delim1));
                object_types.at(i).dodge.numDice = stoi(object_dodge.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).dodge.numSides = stoi(object_dodge.substr(delim2 + 1, delim3));

                break;
            }

            case defid: {
                string object_def = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_def.find("+");
                int delim2 = object_def.find("d");
                int delim3 = object_def.find("\n");

                object_types.at(i).def.base = stoi(object_def.substr(0, delim1));
                object_types.at(i).def.numDice = stoi(object_def.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).def.numSides = stoi(object_def.substr(delim2 + 1, delim3));

                break;
            }

            case speedid: {
                string object_speed = line.substr(line.find(" ") + 1, line.find("\n"));

                int delim1 = object_speed.find("+");
                int delim2 = object_speed.find("d");
                int delim3 = object_speed.find("\n");

                object_types.at(i).speed.base = stoi(object_speed.substr(0, delim1));
                object_types.at(i).speed.numDice = stoi(object_speed.substr(delim1 + 1, delim2 - (delim1 + 1)));
                object_types.at(i).speed.numSides = stoi(object_speed.substr(delim2 + 1, delim3));

                break;
            }

            case descid: {
                string object_desc;

                getline(inFile, line);
                while (line.compare(".")) {
                    object_desc = object_desc + line + "\n";
                    getline(inFile, line);
                }

                object_types.at(i).desc = object_desc;
                break;
            }

            case rrtyid: {
                object_types.at(i).rrty = stoi(line.substr(line.find(" ") + 1, line.find("\n")));

                break;
            }

            case artid: {
                string art_str = line.substr(line.find(" ") + 1, line.find("\n"));

                if(!art_str.compare("TRUE"))
                {
                    object_types.at(i).art = 1;
                }
                else {
                    object_types.at(i).art = 0;
                }
                break;
            }
        }
    }

    inFile.close();

    for(size_t i = 0; i < object_types.size(); ++i){
        object_types.at(i).print();
    }
}
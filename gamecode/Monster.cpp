#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "Monster.h"
#include "dungeon.h"
#include "Character.h"
#include "windows.h"
#include "pc.h"
#include "rlg327.h"


//default constructor to make a new random monster
Monster:: Monster()
    :Character(rand() % 16 + 5){
    intelligent = rand() % 2;
    telepath = rand() % 2;
    tunneling = rand() % 2;
    erratic = rand() % 2;
    last_seen[dim_x] = -1;
    last_seen[dim_y] = -1;
    set_monster_char();
}

void Monster::get_monster_path(Dungeon *d){
    int x,y;
    if (tunneling) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path_to_pc[y][x] = d->tun_path[y][x];
            }
        }
    } else {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path_to_pc[y][x] = d->non_tun_path[y][x];
            }
        }
    }
}
void Monster::set_monster_char()
{
    char binary_char[4] = {(char)('0'+(char)erratic),
                           (char)('0'+(char)tunneling),
                           (char)('0'+(char)telepath),
                           (char)('0'+(char)intelligent)};

    int binary = atoi(binary_char),decimal = 0,base = 1,rem;
    while (binary > 0){
        rem = binary % 10;
        decimal = decimal + rem * base;
        binary = binary/10;
        base = base * 2;
    }
    if (decimal <= 9){
        display_char = (char)('0' + decimal);}
    else {
        decimal -= 10;
        display_char = (char)('A' + decimal);
    }
}
void Monster::erratic_move(Dungeon *d){
    int moved = 0,counter = 0,dx,dy;
    while(!moved) {
        if (counter > 500) {return;} //if it cant find a move, give up
        counter++;
        dx = (rand() % 3) - 1;
        dy = (rand() % 3) - 1;
        //wall and tunneling check dont need to happen anymore but continue still needs to be present
        if (mapxy(pos[dim_x]+dx,pos[dim_y]+dy) == ter_wall) {
            if(tunneling) {
                tun_rock_check(d,&dx,&dy);
            } else {
                continue;
            }
        }
        if (mapxy(pos[dim_x]+dx,pos[dim_y]+dy) != ter_wall_immutable) {
            final_move(d, dx, dy);
            moved = 1;
        }
    }
}
void Monster::tun_rock_check(Dungeon *d, int *dx, int *dy){
    if (d->map[pos[dim_y]+*dy][pos[dim_x]+*dx] == ter_wall){
        if (tunneling) {
            int hardness = (hardnessxy(pos[dim_x] + *dx, pos[dim_y] + *dy)) - 85;
            hardnessxy(pos[dim_x] + *dx, pos[dim_y] + *dy) = MAX(0, hardness);
            if (is_open_space(d, pos[dim_y] + *dy, pos[dim_x] + *dx)) {
                mapxy(pos[dim_x] + *dx, pos[dim_y] + *dy) = ter_floor_hall;
            } else {
                *dx = *dy = 0;
            }
        } else {
            *dx = *dy = 0;
        }
    }
}
void Monster::move_line(Dungeon *d, Dif *dif)
{
    if(pos[dim_x] < pc_last_loc[dim_x]){
        dif->x = 1;
    } else if(pos[dim_x] > pc_last_loc[dim_x]){
        dif->x = -1;
    } else{
        dif->x = 0;
    }

    if(pos[dim_y] < pc_last_loc[dim_y]){
        dif->y = 1;
    } else if(pos[dim_y] > pc_last_loc[dim_y]){
        dif->y = -1;
    } else{
        dif->y = 0;
    }

    if(d->map[pos[dim_y] + dif->y][pos[dim_x] + dif->x] == ter_wall || d->map[pos[dim_y] + dif->y][pos[dim_x] + dif->x] == ter_wall_immutable)
    {
        if(d->map[pos[dim_y]][pos[dim_x] + dif->x] == ter_wall || d->map[pos[dim_y]][pos[dim_x] + dif->x] == ter_wall_immutable){
            dif->x = 0;
        }
        if(d->map[pos[dim_y] + dif->y][pos[dim_x] ] == ter_wall || d->map[pos[dim_y] + dif->y][pos[dim_x]] == ter_wall_immutable){
            dif->y = 0;
        }
    }
}
void Monster::final_move(Dungeon *d, int dx, int dy)
{
    //set current space to null
    d->character_map[pos[dim_y]][pos[dim_x]] = NULL;
    // if there's another character in destination, kill it
    if (d->character_map[pos[dim_y]+dy][pos[dim_x]+dx]){
        d->character_map[pos[dim_y]+dy][pos[dim_x]+dx]->setLiving(0);
    }
    //set future position and update the map
    pos[dim_y] = pos[dim_y]+dy;
    pos[dim_x] = pos[dim_x]+dx;
    d->character_map[pos[dim_y]][pos[dim_x]] = this;
}
void Monster::move_monster(Dungeon *d)
{
    int sees_player = 0,dx = 0,dy = 0;

    Dif dif;
    dif.x = 0;
    dif.y = 0;
    // 50% that if a monster is erratic it make an erratic move
    if (erratic && rand() % 2) {
        erratic_move(d);
    }
    //if the player is seen
    if (telepath || bresenham_LOS(d))
    {
        sees_player = 1;
        // can probably rewrite to use current pc location
        pc_last_loc[dim_x] = d->pc->pos[dim_x];
        pc_last_loc[dim_y] = d->pc->pos[dim_y];
        get_monster_path(d);

    }
    if (intelligent) {
        //test if path_to_pc has been filled
        if (path_to_pc[0][0].cost == INT_MAX) {
            uint32_t cost = INT_MAX;
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    if (path_to_pc[pos[dim_y] + y][pos[dim_x] + x].cost < cost) {
                        dy = y;
                        dx = x;
                        cost = path_to_pc[pos[dim_y] + y][pos[dim_x] + x].cost;
                    }
                }
            }
            tun_rock_check(d,&dx,&dy);
        }
    }
    else if (sees_player) {
        // sees player but non monster->intelligent
        if(tunneling){
            bresenham_move(d,&dif);
            dx = dif.x;
            dy = dif.y;
        }
        else {
            move_line(d, &dif);
            dx = dif.x;
            dy = dif.y;
        }
        tun_rock_check(d,&dx,&dy);
    }
    if (mapxy(pos[dim_x]+dx,pos[dim_y]+dy) != ter_wall_immutable) {
        final_move(d, dx, dy);
    }
}
void Monster::setAbilities(string abil){
    //TODO parse abil and set monster abilities
}
int Monster::create_monster(Monstertype *t){
    name = t->name;
    desc = t->desc;
    dispColor = t->pickColor();
    speed = t->speed.roll();
    setAbilities(t->abil);
    hitpoints = t->hp.roll();
    damage = t->dam;
    display_char = t->symb[0];

return 0;
}

void update_last_seen(Dungeon *d)
{
    for(int i = 0; i < d->num_monsters; ++i){

        if(d->monsters[i]->living &&
                d->monsters[i]->pos[dim_x] >= d->pc->pos[dim_x] - 2 &&
                d->monsters[i]->pos[dim_x] <= d->pc->pos[dim_x] + 2 &&
                d->monsters[i]->pos[dim_y] >= d->pc->pos[dim_y] - 2 &&
                d->monsters[i]->pos[dim_y] <= d->pc->pos[dim_y] + 2)
        {
            if(d->monsters[i]->last_seen[dim_x] == -1){
                d->pc->vis_monsters[d->monsters[i]->pos[dim_y]][d->monsters[i]->pos[dim_x]] = d->monsters[i];

                d->monsters[i]->last_seen[dim_x] = d->monsters[i]->pos[dim_x];
                d->monsters[i]->last_seen[dim_y] = d->monsters[i]->pos[dim_y];
            }
            else{
                d->pc->vis_monsters[d->monsters[i]->last_seen[dim_y]][d->monsters[i]->last_seen[dim_x]] = NULL;

                d->monsters[i]->last_seen[dim_x] = d->monsters[i]->pos[dim_x];
                d->monsters[i]->last_seen[dim_y] = d->monsters[i]->pos[dim_y];

                d->pc->vis_monsters[d->monsters[i]->pos[dim_y]][d->monsters[i]->pos[dim_x]] = d->monsters[i];
            }
        }
    }
}
//TODO update monster list with new fields
void write_monster_list(Dungeon *d, int index){
#define win d->windows->monster_list_win

    wclear(win);

    wmove(win,2,0);
    for(int i = index; i < index + 16 && i < d->num_monsters + 1; ++i)
    {
        wprintw(win, " %3i: ", i);
        wprintw(win, "%c: ", d->monsters[i]->getDisplayChar());

        int dy = d->pc->pos[dim_y] - d->pc->pos[dim_y];
        if(dy < 0) {
            wprintw(win, "%2d South ", abs(dy));
        }
        else{
            wprintw(win, "%2d North ", abs(dy));
        }

        int dx = d->pc->pos[dim_x] - d->pc->pos[dim_x];
        if(dx < 0) {
            wprintw(win, "by %2d East\n", abs(dx));
        }
        else{
            wprintw(win, "by %2d West\n", abs(dx));
        }
    }

    box(win,0,0);
    mvwprintw(win,19,2,"Arrows to scroll Q to exit");
    mvwprintw(win,0,4, "List of Known Monsters");

    wrefresh(win);
}

void monster_list(Dungeon *d) {

    int index = 1;
    write_monster_list(d,index);
    touchwin(d->windows->monster_list_win);
    int visible = 1;
    while (visible) {
        int val = wgetch(d->windows->monster_list_win);
        switch (val) {
            case KEY_UP:
                if (index > 1){
                    index--;
                }
                write_monster_list(d,index);
                break;

            case KEY_DOWN:
                if (index < d->num_monsters-15){
                    index++;
                }
                write_monster_list(d,index);
                break;

            case 'Q':
                visible = 0;
                touchwin(stdscr);
                break;


            default:
                break;
        }
    }
}

int new_gen_monster(Dungeon *d){
    //TODO add new monster gen rules

    //same rules for determining number of monsters in dungeon
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}
    d->monsters = (Monster**)calloc((d->num_monsters),sizeof(Monster) * (d->num_monsters));
    Monstertype mon;
    for(int i = 0; i < d->num_monsters; i++)
    {
    do {

        mon = monster_types.at(rand() % monster_types.size());

        if(mon.uniq == 1 && mon.inUse == 1){
            mon.rrty = 0;
        }
        mon.inUse = 1;
        //TODO if its a uniq monster, make it not gen again
    } while (mon.rrty < rand() % 100);

        d->monsters[i] = mon.createMonster();
        cout<<"hello"<<endl;
        // d->monsters[i]->setDisplayChar(mon.symb);

    }
    // do {
    //     mon = monster_types.at(rand() % monster_types.size());
    //     //TODO if its a uniq monster, make it not gen again
    // } while (mon.rrty < rand() % 100);

    // for(int i = 0; i < d->num_monsters; i++)
    // {
    //     d->monsters[i] = mon.createMonster();

    //     // d->monsters[i]->setDisplayChar(mon.symb);

    // }

    int pcRoomNum;
    int totalArea = 0;
    for(uint32_t i = 0; i < d->num_rooms; ++i)
    {
        if (in_room(d->rooms[i],d->pc)){
            pcRoomNum = i;
        } else {
            totalArea += d->rooms[i].size[dim_x] * d->rooms[i].size[dim_y];
        }
    }

    int totalMonsters = 0;
    while(totalMonsters < d->num_monsters)
    {
        if(totalMonsters == totalArea){break;}

        int randRoom = rand() % d->num_rooms;

        if(randRoom == pcRoomNum){continue;}

        int x = rand() % d->rooms[randRoom].size[dim_x];
        int y = rand() % d->rooms[randRoom].size[dim_y];

        if(d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] != NULL){continue;}

        d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] = d->monsters[totalMonsters];

        d->monsters[totalMonsters]->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
        d->monsters[totalMonsters]->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;

        ++totalMonsters;
    }
    return 0;
}
int gen_monsters(Dungeon *d)
{
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}
    d->monsters = (Monster**)calloc((d->num_monsters),sizeof(Monster) * (d->num_monsters));

    for(int i = 0; i < d->num_monsters; i++)
    {
        d->monsters[i] = new Monster();
    }
    int pcRoomNum;
    int totalArea = 0;
    for(uint32_t i = 0; i < d->num_rooms; ++i)
    {
        if (in_room(d->rooms[i],d->pc)){
            pcRoomNum = i;
        } else {
            totalArea += d->rooms[i].size[dim_x] * d->rooms[i].size[dim_y];
        }
    }

    int totalMonsters = 0;
    while(totalMonsters < d->num_monsters)
    {
        if(totalMonsters == totalArea){break;}

        int randRoom = rand() % d->num_rooms;

        if(randRoom == pcRoomNum){continue;}

        int x = rand() % d->rooms[randRoom].size[dim_x];
        int y = rand() % d->rooms[randRoom].size[dim_y];

        if(d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] != NULL){continue;}

        d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] = d->monsters[totalMonsters];

        d->monsters[totalMonsters]->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
        d->monsters[totalMonsters]->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;

        ++totalMonsters;
    }
    return 0;
}




Monster* Monstertype::createMonster(){
    Monster *monster = new Monster();
    monster->create_monster(this);
    return monster;
}

int Monstertype::pickColor(){
    //TODO pick a color from the monsters color list
    // note things like COLOR_CYAN is technically an int so return that
    // Color are already defined in rlg327.cpp
    return 0;
}

void Monstertype::print()
{
    cout << "NAME: " + this->name << endl;
    cout << "DESC: " << endl;
    cout << this->desc;
    cout << "COLOR: " + this->color << endl;

    cout<< "SMART:  " << endl;
    cout<< this->smart << endl;

    cout<< "TUNNELING:  " << endl;
    cout<< this->tunneling << endl;

    cout<< "PICKUP:  " << endl;
    cout<< this->pickup << endl;

    cout<< "UNIQUE:  " << endl;
    cout<< this->uniq << endl;

    cout<< "ERRATIC:  " << endl;
    cout<< this->erratic << endl;

    cout<< "DESTROY:  " << endl;
    cout<< this->destroy << endl;

    cout<< "TELE:  " << endl;
    cout<< this->tele << endl;

    cout<< "PATH:  " << endl;
    cout<< this->path << endl;

    cout<< "BOSS:  " << endl;
    cout<< this->boss << endl;


    cout << "SPEED: ";
    this->speed.print();

    cout << "ABIL: " + this->abil << endl;

    cout << "HP: ";
    this->hp.print();

    cout << "DAM: ";
    this->dam.print();

    cout << "SYMB: " + this->symb << endl;
    cout << "RRTY: ";
    cout << this->rrty << endl;
    cout << "\n";
}

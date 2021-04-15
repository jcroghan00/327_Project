#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sstream>

#include "Monster.h"
#include "dungeon.h"
#include "Character.h"
#include "windows.h"
#include "pc.h"
#include "rlg327.h"

//default constructor to make a new random monster
Monster:: Monster()
    :Character(rand() % 16 + 5){
    create_monster(pick_type());
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            path_to_dest[i][j].cost = INT_MAX;
        }
    }

}

Monstertype* Monster::pick_type(){
    // pick a random monster type and if it can be spawned return it
    Monstertype* mon;
    do {
        mon = &monster_types.at(rand() % monster_types.size());
        //TODO if the unique monster didnt die it can spawn again
        if(mon->abilities.UNIQ && mon->inUse) {
            mon->rrty = -1;
        }
    } while (mon->rrty < rand() % 100);
    mon->inUse = 1;
    return mon;
}

int Monster::attack_monster(Dungeon *d,int damage){
    hitpoints -= damage;
    if (hitpoints <= 0){
        character_mappair(pos) = NULL;
        setLiving(0);
        return 1;
    }
    return 0;
}

void Monster::get_monster_path(Dungeon *d){
    int x,y;
    if (abil.TUNNEL) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path_to_dest[y][x] = d->tun_path[y][x];
            }
        }
    } else {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path_to_dest[y][x] = d->non_tun_path[y][x];
            }
        }
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
            if(abil.TUNNEL) {
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
        if (abil.TUNNEL) {
            int hardness = (hardnessxy(pos[dim_x] + *dx, pos[dim_y] + *dy)) - 85;
            hardnessxy(pos[dim_x] + *dx, pos[dim_y] + *dy) = MAX(0, hardness);
            if (d->is_open_space(pos[dim_y] + *dy, pos[dim_x] + *dx)) {
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
    if(pos[dim_x] < destination[dim_x]){
        dif->x = 1;
    } else if(pos[dim_x] > destination[dim_x]){
        dif->x = -1;
    } else{
        dif->x = 0;
    }

    if(pos[dim_y] < destination[dim_y]){
        dif->y = 1;
    } else if(pos[dim_y] > destination[dim_y]){
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
    int sees_pc = 0,dx = 0,dy = 0;
    Dif dif;
    dif.x = 0;
    dif.y = 0;

    //if erratic 50% chance of erratic move
    if (abil.ERRATIC && rand() % 2) {
        erratic_move(d);
        return;
    }
    //update destination and map if the pc can be seen
    if (abil.TELE || sees_player(d)){
        sees_pc = 1; //should be removed when done
        destination[dim_y] = d->pc->pos[dim_y];
        destination[dim_x] = d->pc->pos[dim_x];
        get_monster_path(d);
    } else if (!abil.SMART){ return;} //if not smart and cant see player  end move

    if (abil.SMART) {
        //if destination is not yourself
        if (destination[dim_y] != pos[dim_y] && destination[dim_x] != pos[dim_x]) {
            uint32_t cost = INT_MAX;
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    if (path_to_dest[pos[dim_y] + y][pos[dim_x] + x].cost < cost) {
                        dy = y;
                        dx = x;
                        cost = path_to_dest[pos[dim_y] + y][pos[dim_x] + x].cost;
                    }
                }
            }
            tun_rock_check(d,&dx,&dy);
        }
    }
    else if (sees_pc) {
        // sees player but non monster->intelligent
        if(abil.TUNNEL){
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

void Monster:: isMonster(Dungeon *d, Dif *dif){
    int count = NULL;
    for(int i = 0; i < d->num_monsters; i++){
        int monstX = d->monsters[i]->pos[dim_x];
        int monstY = d->monsters[i]->pos[dim_y];

        if(pos[dim_x] + dif->x == monstX && pos[dim_y] + dif->y == monstY){
            count = i;
            break;
        }
    }

    if(count){
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                if(mapxy(d->monsters[count]->pos[dim_x]+x,d->monsters[count]->pos[dim_y]+y) ==ter_floor_hall || mapxy(d->monsters[count]->pos[dim_x] + x, d->monsters[count]->pos[dim_y] + y) == ter_floor_room || mapxy(d->monsters[count]->pos[dim_x] + x, d->monsters[count]->pos[dim_y] + y) == ter_floor ){
                    d->monsters[count]->pos[dim_x] = d->monsters[count]->pos[dim_x] + x;
                    d->monsters[count]->pos[dim_y] = d->monsters[count]->pos[dim_y] + y;
                    break;

                }
            }
        
            }
    }

}

int Monster::create_monster(Monstertype *t){
    name = t->name;
    desc = t->desc;
    dispColor = t->getColor();
    speed = t->speed.roll();
    abil = t->abilities;

    hitpoints = t->hp.roll();
    damage = t->dam;
    display_char = t->symb[0];

return 0;
}

void update_last_seen(Dungeon *d)
{
    for(int i = 0; i < d->num_monsters; ++i){

        if(d->monsters[i]->isLiving() &&
                d->monsters[i]->pos[dim_x] >= d->pc->pos[dim_x] - 2 &&
                d->monsters[i]->pos[dim_x] <= d->pc->pos[dim_x] + 2 &&
                d->monsters[i]->pos[dim_y] >= d->pc->pos[dim_y] - 2 &&
                d->monsters[i]->pos[dim_y] <= d->pc->pos[dim_y] + 2)
        {
            if(d->monsters[i]->destination[dim_x] == -1){
                d->pc->vis_monsters[d->monsters[i]->pos[dim_y]][d->monsters[i]->pos[dim_x]] = d->monsters[i];

                d->monsters[i]->destination[dim_x] = d->monsters[i]->pos[dim_x];
                d->monsters[i]->destination[dim_y] = d->monsters[i]->pos[dim_y];
            }
            else{
                d->pc->vis_monsters[d->monsters[i]->destination[dim_y]][d->monsters[i]->destination[dim_x]] = NULL;

                d->monsters[i]->destination[dim_x] = d->monsters[i]->pos[dim_x];
                d->monsters[i]->destination[dim_y] = d->monsters[i]->pos[dim_y];

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

Monster* Monstertype::createMonster(){
    Monster *monster = new Monster();
    monster->create_monster(this);
    return monster;
}

int Monstertype::getColor(){
    string dispColor;

    for (int i = 0; i < (int)this->color.length();i++){
        if (this->color.at(i) == ' '){
            break;
        }
        dispColor.push_back(this->color.at(i));
    }

    if (dispColor == "RED"){
        return COLOR_RED;
    } else if (dispColor == "GREEN"){
        return COLOR_GREEN;
    }else if (dispColor == "BLUE"){
        return COLOR_BLUE;
    }else if (dispColor == "CYAN"){
        return COLOR_CYAN;
    }else if (dispColor == "YELLOW"){
        return COLOR_YELLOW;
    }else if (dispColor == "MAGENTA"){
        return COLOR_MAGENTA;
    }else{
        return COLOR_WHITE;
    }
}

void Monstertype::print()
{
    cout << "NAME: " + this->name << endl;
    cout << "DESC: " << endl;
    cout << this->desc;
    cout << "COLOR: " + this->color << endl;

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

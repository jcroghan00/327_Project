#include "Character.h"
#include "dungeon.h"
#include "Monster.h"
#include "pc.h"

Character:: Character(){
    living = 1;
    turn = 0;
}
Character:: Character(int s){
    living = 1;
    turn = 0;
    speed = s;
}

void Character::setSd(uint32_t sd2){sd = sd2;}
uint32_t Character::getSd(){return sd;}
void Character::setDisplayChar(char c) {display_char = c;}
char Character::getDisplayChar() {return display_char;}
void Character::setLiving(int l){living = l;}
int Character::isLiving(){return living;}
void Character::setTurn(int t){turn = t;}
uint32_t Character::getTurn(){return turn;}
void Character::setSpeed(int s){speed = s;}
uint32_t Character::getSpeed(){return speed;}

int Character::bresenham_LOS(Dungeon *d)
{
    int x0 = pos[dim_x];
    int y0 = pos[dim_y];
    int x1 = d->pc->pos[dim_x];
    int y1 = d->pc->pos[dim_y];

    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    while(1)
    {
        if(mapxy(x0, y0) == ter_wall || mapxy(x0, y0) == ter_wall_immutable)
        {
            return 0;
        }
        if(x0 == x1 && y0 == y1) {return 1;}
        e2 = err * 2;
        if(e2 >= dy){
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx){
            err += dx;
            y0 += sy;
        }
    }
}
void Character::bresenham_move(Dungeon *d, Dif *dif)
{
    int x0 = pos[dim_x];
    int y0 = pos[dim_y];
    int x1 = d->pc->pos[dim_x];
    int y1 = d->pc->pos[dim_y];

    int dx = abs(x1 - x0);
    int sx = x0<x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0<y1 ? 1 : -1;

    int err = dx + dy;
    int e2 = 0;

    e2 = err * 2;
    if(e2 >= dy){
        dif->x = sx;
    }
    if(e2 <= dx){
        dif->y = sy;
    }
}


int32_t character_cmp(const void *key, const void *with) {
    if (((Character *) key)->getTurn() != ((Character *) with)->getTurn()){
        return ((Character *) key)->getTurn() - ((Character *) with)->getTurn();
    }
    else{
        return ((Character *) key)->getSd() - ((Character *) with)->getSd();
    }
}
/*
void delete_characters(Character **c, Dungeon *d)
{
    character_mappair(c[0]->pos) = NULL;
    free(c[0]);
    for(int i = 1; i < d->num_monsters + 1; ++i)
    {
        character_mappair(c[i]->pos) = NULL;
        free(c[i]);
    }
}
 */
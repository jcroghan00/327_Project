#include "Character.h"
#include "dungeon.h"

using namespace std;
Character:: Character(){
    //these dont do anything right now
    living = 1;
    turn = 0;
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
using namespace std;

//TODO can this be put in the constructor?
void define_characters(Dungeon *d)
{
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}
    d->characters = (Character**)calloc((d->num_monsters+1),sizeof(Character) * (d->num_monsters + 1));
}

int32_t character_cmp(const void *key, const void *with) {
    if (((Character *) key)->getTurn() != ((Character *) with)->getTurn()){
        return ((Character *) key)->getTurn() - ((Character *) with)->getTurn();
    }
    else{
        return ((Character *) key)->getSd() - ((Character *) with)->getSd();
    }
}

void delete_characters(Character **c, Dungeon *d)
{
    character_mappair(c[0]->pos) = NULL;
    free(c[0]);
    for(int i = 1; i < d->num_monsters + 1; ++i)
    {
        character_mappair(c[i]->pos) = NULL;
        free(c[i]->monster);
        free(c[i]);
    }
}
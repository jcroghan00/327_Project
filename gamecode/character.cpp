#include "character.h"
#include "dungeon.h"

void define_characters(Dungeon *d)
{
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}
    d->characters = (Character**)calloc((d->num_monsters+1),sizeof(Character) * (d->num_monsters + 1));
}

int32_t character_cmp(const void *key, const void *with) {
    if (((Character *) key)->turn != ((Character *) with)->turn){
        return ((Character *) key)->turn - ((Character *) with)->turn;
    }
    else{
        return ((Character *) key)->sd - ((Character *) with)->sd;
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

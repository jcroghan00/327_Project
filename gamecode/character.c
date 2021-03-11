#include "character.h"
#include "dungeon.h"

void define_characters(dungeon_t *d)
{
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}
    d->characters = calloc((d->num_monsters+1),sizeof(character_t) * (d->num_monsters + 1));
}

int32_t character_cmp(const void *key, const void *with) {
    if (((character_t *) key)->turn != ((character_t *) with)->turn){
        return ((character_t *) key)->turn - ((character_t *) with)->turn;
    }
    else{
        return ((character_t *) key)->sd - ((character_t *) with)->sd;
    }
}

void delete_characters(character_t **c, dungeon_t *d)
{
    free(c[0]);
    for(int i = 1; i < d->num_monsters + 1; ++i)
    {
        character_mappair(c[i]->pos) = NULL;
        free(c[i]->monster);
        free(c[i]);
    }
}

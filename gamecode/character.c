#include "character.h"
#include "dungeon.h"

void define_characters(dungeon_t *d)
{
    if (d->num_monsters == -1) {d->num_monsters = d->num_rooms * 2 < 50 ? d->num_rooms * 2 : 50;}

    malloc(sizeof(character_t) * (d->num_monsters + 1));
}

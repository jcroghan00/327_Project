#ifndef PC_H
# define PC_H
#include "dim.h"
#include "config.h"
#include "Character.h"
typedef struct heap heap_t;
typedef enum terrain_type terrain_type_t;

class Dungeon;
class Object;

class PC: public Character{
public:
    terrain_type_t pc_map[DUNGEON_Y][DUNGEON_X];
    Character *vis_monsters[DUNGEON_Y][DUNGEON_X];
    Object *visObj[DUNGEON_Y][DUNGEON_X];
    Object *equipSlots[12];
    Object *carrySlots[10];
    PC();
    void pickup_item(Dungeon *d);
    void wear_item(Dungeon *d, int slot);
    void update_pc_map(Dungeon *d);
    void update_vis_objects(Dungeon *d);
    void fight_monster(Dungeon *d, int dx, int dy);
    int move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport);
};

void config_pc(Dungeon *d);
void place_pc(Dungeon *d);
void move_pc_ncurses(Dungeon *d, heap_t *h);
int pc_next_pos(Dungeon *d);

#endif
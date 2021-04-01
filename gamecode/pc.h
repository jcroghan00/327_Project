#ifndef PC_H
# define PC_H
#include "Character.h"

class Dungeon;

class PC: public Character{
public:
    PC();
};

void config_pc(Dungeon *d);
void place_pc(Dungeon *d);
int move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport);
void move_pc_ncurses(Dungeon *d, heap_t *h);
int pc_next_pos(Dungeon *d);
void init_pc_map(Dungeon *d);
void update_pc_map(Dungeon *d);

#endif
#ifndef PC_H
# define PC_H
#include "Character.h"
typedef struct heap heap_t;

class Dungeon;

class PC: public Character{
public:
    PC();
    void update_pc_map(Dungeon *d);
    int move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport);
};

void config_pc(Dungeon *d);
void place_pc(Dungeon *d);
void move_pc_ncurses(Dungeon *d, heap_t *h);
int pc_next_pos(Dungeon *d);
void init_pc_map(Dungeon *d);

#endif
#ifndef PC_H
# define PC_H

class Dungeon;


class Pc {
public:
};


int pc_is_alive(Dungeon *d);
void place_pc(Dungeon *d);
void config_pc(Dungeon *d);
void move_pc_ncurses(Dungeon *d, heap_t *h);
int pc_next_pos(Dungeon *d);


#endif
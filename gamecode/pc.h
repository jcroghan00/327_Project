#ifndef PC_H
# define PC_H

typedef struct dungeon dungeon_t;


typedef struct pc {
}pc_t;


int pc_is_alive(dungeon_t *d);
void place_pc(dungeon_t *d);
void config_pc(dungeon_t *d);
void move_pc_ncurses(dungeon_t *d, heap_t *h);
int pc_next_pos(dungeon_t *d);


#endif
#ifndef PC_H
# define PC_H

typedef struct dungeon dungeon_t;


typedef struct pc {
}pc_t;


int is_pc_alive(dungeon_t *d);
void place_pc(dungeon_t *d);
void config_pc(dungeon_t *d);
int pc_next_pos(dungeon_t *d);


#endif
#ifndef MONSTER_H
# define MONSTER_H

#include "dungeon.h"


class Monster {
public:
    /*
     *  Intelligent monsters understand the dungeon layout and move on the shortest path (as
     *  per path finding) toward the PC. Unintelligent monsters move in a straight line toward the PC (which
     *  may trap them in a corner or force them to tunnel through the hardest rocks). Intelligent monsters also
     *  can remember the last known position of a previously seen PC
     */
    int intelligent;
    /*
     * Telepathic monsters always know where the PC is. Non-telepathic monsters only know
     * the position of the PC if they have line of sight (smart ones can remember the position where they
     * last saw the PC when line of sight is broken). Telepathic monsters will always move toward the PC.
     * Non-telepathic monsters will only move toward a visible PC or a remembered last position
     */
    int telepath;
    /*
     *  Tunneling monsters can tunnel through rock. Non-tunneling monsters cannot.
     *  These two classes will use different distance maps. Tunneling monsters when attempting to move
     *  through rock will subtract 85 from its hardness, down to a minimum of zero. When the hardness
     *  reaches zero, the rock becomes corridor and the monster immediately moves. Reducing the hardness
     *  of rock forces a tunneling distance map recalculation. Converting rock to corridor forces tunneling
     *  and non-tunneling distance map recalculations
     */
    int tunneling;
    /*
     * Erratic monsters have a 50% chance of moving as per their other characteristics.
     * Otherwise they move to a random neighboring cell (tunnelers may tunnel, but non-tunnelers must
     * move to an open cell)
     */
    int erratic;
    pair_t pc_last_loc;
    Monster_Path path_to_pc[DUNGEON_Y][DUNGEON_X];
};

class Dif{
public:
    int x;
    int y;
};

void monster_list(Dungeon *d);
int gen_monsters(Dungeon *d);
void move_monster(Character *c, Dungeon *d);
int bresenham_LOS(Dungeon *d,Character *c);
void bresenham_move(Dungeon *d,Character *c, Dif *dif);



#endif

#include <ncurses.h>

#include "dungeon.h"
#include "Character.h"
#include "Monster.h"
#include "windows.h"
#include "pc.h"
typedef struct heap heap_t;

PC::PC(){
    setDisplayChar('@');
    setSpeed(PC_SPEED);
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            pc_map[i][j] = ter_wall;
            vis_monsters[i][j] = NULL;
        }
    }

}

void PC::update_pc_map(Dungeon *d){
    int x = pos[dim_x];
    int y = pos[dim_y];

    for(int i = -2; i <= 2; ++i){
        for(int j = -2; j <= 2; ++j){
            if((x + i >= 0 && x + i < DUNGEON_X) && (y + j >= 0 && y + j < DUNGEON_Y)){
                this->pc_map[y + j][x + i] = d->map[y + j][x + i];
            }
            else{
                continue;
            }
        }
    }
}

void PC::update_vis_objects(Dungeon *d)
{
    int x = pos[dim_x];
    int y = pos[dim_y];

    for(int i = -2; i <= 2; ++i){
        for(int j = -2; j <= 2; ++j){
            if((x + i >= 0 && x + i < DUNGEON_X) && (y + j >= 0 && y + j < DUNGEON_Y)){
                this->visObj[y + j][x + i] = d->objMap[y + j][x + i];
            }
            else{
                continue;
            }
        }
    }
}

int PC::move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport = 0){

    if(d->map[pos[dim_y] + dy][pos[dim_x] + dx] < ter_floor &&
       !teleport){
        const char *msg = "There's a wall there!";
        mvprintw(0, (COLS/2 - strlen(msg)/2), msg);
        move_pc_ncurses(d,h);
        return -1;
    }

    d->character_map[d->pc->pos[dim_y]][pos[dim_x]] = NULL;
    vis_monsters[d->pc->pos[dim_y]][pos[dim_x]] = NULL;
    pos[dim_y] += dy;
    pos[dim_x] += dx;
    if (d->character_map[pos[dim_y]][pos[dim_x]] != NULL){
        d->character_map[pos[dim_y]][pos[dim_x]]->setLiving(0);
        d->num_monsters--;
    }
    d->character_map[pos[dim_y]][pos[dim_x]] = this;
    update_pc_map(d);
    update_vis_objects(d);
    return 0;
}

void place_pc(Dungeon *d)
{
    int randRoom = rand() % d->num_rooms;
    int x = rand() % d->rooms[randRoom].size[dim_x];
    int y = rand() % d->rooms[randRoom].size[dim_y];
    d->pc->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;
    d->pc->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
    character_mapxy(d->pc->pos[dim_x],d->pc->pos[dim_y]) = d->pc;
}

void config_pc(Dungeon *d)
{
    d->pc = new PC();
    place_pc(d);
}

void move_pc_ncurses(Dungeon *d, heap_t *h)
{
    int val  = wgetch(stdscr);

    clear();
    render(d);
    refresh();

    switch (val)
    {
        // Move up-left
        case KEY_HOME:
        case '7':
        case 'y':
            d->pc->move_pc(d, h, -1, -1); break;

            // Move up
        case KEY_UP:
        case '8':
        case 'k':
            d->pc->move_pc(d, h, -1, 0); break;

            // Move up-right
        case KEY_PPAGE:
        case '9':
        case 'u':
            d->pc->move_pc(d, h, -1, 1); break;

            // Move right
        case KEY_RIGHT:
        case '6':
        case 'l':
            d->pc->move_pc(d, h, 0, 1); break;

            // Move down-right
        case KEY_NPAGE:
        case '3':
        case 'n':
            d->pc->move_pc(d, h, 1, 1); break;

            // Move down
        case KEY_DOWN:
        case '2':
        case 'j':
            d->pc->move_pc(d, h, 1, 0); break;

            // Move down-left
        case KEY_END:
        case '1':
        case 'b':
            d->pc->move_pc(d, h, 1, -1); break;

            // Move left
        case KEY_LEFT:
        case '4':
        case 'h':
            d->pc->move_pc(d, h, 0, -1); break;

            // rest
        case KEY_B2:
        case ' ':
        case '.':
        case '5':
            d->pc->move_pc(d, h, 0, 0); break;

            // Go down stairs
        case '>':
            //TODO start on up stairs
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_down){
                new_dungeon(d, h);
            } else {
                const char *down_msg = "You can't go down here!";
                mvprintw(0, (COLS/2 - strlen(down_msg)/2), down_msg);
                move_pc_ncurses(d,h);
            }
            break;
        case '<':
            //TODO start on down stairs
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_up){
                new_dungeon(d, h);
            } else{
                const char *up_msg = "You can't go up here!";
                mvprintw(0, (COLS/2 - strlen(up_msg)/2), up_msg);
                move_pc_ncurses(d,h);
            }
            break;

            //Display character information
        case 'c':
            render_character_info(d);
            move_pc_ncurses(d, h);
            break;

            //Drop item
        case 'd':
            render_drop(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Display equipment
        case 'e':
            goto jump;

            //Toggle fog-of-war
        case 'f':
            clear();
            if(d->fow){
                const char *fog_off = "Fog-of-war off!";
                mvprintw(0, (COLS/2 - strlen(fog_off)/2), fog_off);
                d->fow = 0;
            }
            else{
                const char *fog_on = "Fog-of-war on!";
                mvprintw(0, (COLS/2 - strlen(fog_on)/2), fog_on);
                d->fow = 1;
            }

            render(d);
            refresh();

            move_pc_ncurses(d, h);
            break;

            //Teleport (goto)
        case 'g':
            render_teleport_select(d,h);
            update_last_seen(d);
            render(d);
            move_pc_ncurses(d, h);
            break;

            //Display inventory
        case 'i':
            render_inventory(d);
            move_pc_ncurses(d, h);
            break;

            // TODO Display monster list seg faults when ran
        case 'm':
            monster_list(d);
            move_pc_ncurses(d, h);
            break;

            //Display the default (terrain) map
        case 's':
            render_terrain_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Take off item
        case 't':
            goto jump;

            //TODO Wear item
        case 'w':
            goto jump;

            //TODO Expunge item
        case 'x':
            goto jump;

            //Display the non-tunneling distance map
        case 'D':
            render_dist_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Inspect equipped item
        case 'E':
            goto jump;

            //Display the hardness map
        case 'H':
            render_hardness_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Inspect inventory item
        case 'I':
            goto jump;

            //Look at monster
        case 'L':
            render_monster_info_win(d);
            move_pc_ncurses(d, h);
            break;

            // Quit the game
        case 'Q':
            endwin();
            delete_dungeon(d, h);
            exit(0);

            //Display the tunneling distance map
        case 'T':
            render_tun_dist_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Print list of controls
        case 'C':
            goto jump;

            jump:
        default:
            const char *invalid_msg = "Invalid Key!";
            mvprintw(0, (COLS/2 - strlen(invalid_msg)/2), invalid_msg);
            move_pc_ncurses(d, h);
            break;
    }
    render(d);
    refresh();
}



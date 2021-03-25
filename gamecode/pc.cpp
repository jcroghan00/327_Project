#include <ncurses.h>

#include "dungeon.h"
#include "character.h"
#include "monster.h"
typedef struct heap heap_t;

int pc_is_alive(Dungeon *d)
{
    return d->pc->living;
}

void init_pc_map(Dungeon *d){
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            d->pc_map[i][j] = ter_wall;
        }
    }
}

void update_pc_map(Dungeon *d){
    int x = d->pc->pos[dim_x];
    int y = d->pc->pos[dim_y];

    for(int i = -2; i <= 2; ++i){
        for(int j = -2; j <= 2; ++j){
            if((x + i >= 0 && x + i < DUNGEON_X) && (y + j >= 0 && y + j < DUNGEON_Y)){
                d->pc_map[y + j][x + i] = d->map[y + j][x + i];
            }
            else{
                continue;
            }
        }
    }
}
void place_pc(Dungeon *d)
{
    int randRoom = rand() % d->num_rooms;
    int x = rand() % d->rooms[randRoom].size[dim_x];
    int y = rand() % d->rooms[randRoom].size[dim_y];

    d->pc->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;
    d->pc->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
    character_mapxy(d->pc->pos[dim_x],d->pc->pos[dim_y]) = d->pc;
    d->pc->living = 1;
}

void config_pc(Dungeon *d)
{
    d->pc = (Character*)malloc(sizeof(Character));
    d->pc->display_char = '@';
    d->pc->living = 1;
    d->pc->speed = PC_SPEED;
    place_pc(d);
    d->characters[0] = d->pc;
}

void move_pc_ncurses(Dungeon *d, heap_t *h);

int move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport = 0){

    if(d->map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx] < ter_floor &&
    !teleport){
        mvprintw(0, 0, "There's a wall there!");
        move_pc_ncurses(d,h);
        return -1;
    }

    d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = NULL;
    d->pc->pos[dim_y] += dy;
    d->pc->pos[dim_x] += dx;
    if (d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] != NULL){
        d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]]->living = 0;
        d->num_monsters--;
    }
    d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = d->pc;
    update_pc_map(d);
    return 0;
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
            move_pc(d, h, -1, -1); break;

            // Move up
        case KEY_UP:
        case '8':
        case 'k':
            move_pc(d, h, -1, 0); break;

            // Move up-right
        case KEY_PPAGE:
        case '9':
        case 'u':
            move_pc(d, h, -1, 1); break;

            // Move right
        case KEY_RIGHT:
        case '6':
        case 'l':
            move_pc(d, h, 0, 1); break;

            // Move down-right
        case KEY_NPAGE:
        case '3':
        case 'n':
            move_pc(d, h, 1, 1); break;

            // Move down
        case KEY_DOWN:
        case '2':
        case 'j':
            move_pc(d, h, 1, 0); break;

            // Move down-left
        case KEY_END:
        case '1':
        case 'b':
            move_pc(d, h, 1, -1); break;

            // Move left
        case KEY_LEFT:
        case '4':
        case 'h':
            move_pc(d, h, 0, -1); break;

            // rest
        case KEY_B2:
        case ' ':
        case '.':
        case '5':
            move_pc(d, h, 0, 0); break;

            // Go down stairs
        case '>':
            //TODO start on up stairs
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_down){
                new_dungeon(d, h);
            } else {
                mvprintw(0, 0, "You can't go down here!");
                move_pc_ncurses(d,h);
            }
            break;
        case '<':
            //TODO start on down stairs
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_up){
                new_dungeon(d, h);
            } else{
                mvprintw(0, 0, "You can't go up here!");
                move_pc_ncurses(d,h);
            }
            break;

            //TODO Display character information
        case 'c':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Drop item
        case 'd':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Display equipment
        case 'e':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Toggle fog-of-war
        case 'f':
            clear();
            if(d->fow){
                mvprintw(0, 0, "Fog-of-war off!");
                d->fow = 0;
            }
            else{
                mvprintw(0, 0, "Fog-of-war on!");
                d->fow = 1;
            }

            render(d);
            refresh();

            move_pc_ncurses(d, h);
            break;

            //Teleport (goto)
        case 'g':
            render_teleport_select(d,h);
            render(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Display inventory
        case 'i':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            // Display monster list
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
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Wear item
        case 'w':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Expunge item
        case 'x':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //Display the non-tunneling distance map
        case 'D':
            render_dist_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Inspect equipped item
        case 'E':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //Display the hardness map
        case 'H':
            render_hardness_map(d);
            move_pc_ncurses(d, h);
            break;

            //TODO Inspect inventory item
        case 'I':
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

            //TODO Look at monster
        case 'L':
            mvprintw(0, 0, "Invalid Key!");
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
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;

        default:
            mvprintw(0, 0, "Invalid Key!");
            move_pc_ncurses(d, h);
            break;
    }
    render(d);
    refresh();
}

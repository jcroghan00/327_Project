#include <ncurses.h>

#include "dungeon.h"
#include "path.h"
#include "character.h"

int is_pc_alive(dungeon_t *d)
{
    return d->pc->living;
}

void place_pc(dungeon_t *d)
{
    int randRoom = rand() % d->num_rooms;
    int x = rand() % d->rooms[randRoom].size[dim_x];
    int y = rand() % d->rooms[randRoom].size[dim_y];

    d->pc->pos[dim_x] = d->rooms[randRoom].position[dim_x] + x;
    d->pc->pos[dim_y] = d->rooms[randRoom].position[dim_y] + y;
    character_mapxy(d->pc->pos[dim_x],d->pc->pos[dim_y]) = d->pc;
    d->pc->living = 1;
}

void config_pc(dungeon_t *d)
{
    d->pc = malloc(sizeof(character_t));
    d->pc->display_char = '@';
    d->pc->living = 1;
    d->pc->speed = PC_SPEED;
    place_pc(d);
    d->characters[0] = d->pc;
}

void move_pc_ncurses(dungeon_t *d){
    int x = 0; 
    int y = 0;

    int val  = getch();

    switch (val)
    {
        // Move up-left
        case KEY_HOME:
        case '7':
        case 'y':
            y = 0; x = -1; break;

        // Move up
        case KEY_UP:
        case '8':
        case 'k':
            y = -1; x = 0; break;

        // Move up-right
        case KEY_PPAGE:
        case '9':
        case 'u':
            y = -1; x = 1; break;

        // Move right
        case KEY_RIGHT:
        case '6':
        case 'l':
            y = 0; x = 1; break;

        // Move down-right
        case KEY_NPAGE:
        case '3':
        case 'n':
            y = 1; x = 1; break;

        // Move down
        case KEY_DOWN:
        case '2':
        case 'j':
            y = 1; x = 0; break;

        // Move down-left
        case KEY_END:
        case '1':
        case 'b':
            y = 1; x = -1; break;

        // Move left
        case KEY_LEFT:
        case '4':
        case 'h':
            y = 0; x = -1; break;

        // rest
        case KEY_B2:
        case ' ':
        case '.':
        case '5':
            y = 0; x = 0; break;

        // Go down stairs
        case '>':
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_up){
                delete_dungeon(d);
                d->num_monsters = 7;
                init_dungeon(d);
                gen_dungeon(d);
                clear();
                render_ncurses(d);
                refresh();
            }
            break;

        // go up stairs
        case '<':
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_down){
                delete_dungeon(d);
                d->num_monsters = 7;
                init_dungeon(d);
                gen_dungeon(d);
                clear();
                render_ncurses(d);
                refresh();
            }
            break;

        //TODO Display character information
        case 'c':
            break;

        //TODO Drop item
        case 'd':
            break;

        //TODO Display equipment
        case 'e':
            break;

        //TODO Toggle fog-of-war
        case 'f':
            break;

        //TODO Teleport (goto)
        case 'g':
            break;

        //TODO Display inventory
        case 'i':
            break;

        //TODO Display monster list
        case 'm':
            break;

        //TODO Display the default (terrain) map
        case 's':
            break;

        //TODO Take off item
        case 't':
            break;

        //TODO Wear item
        case 'w':
            break;

        //TODO Expunge item
        case 'x':
            break;

        //TODO Display the non-tunneling distance map
        case 'D':
            break;

        //TODO Inspect equipped item
        case 'E':
            break;

        //TODO Display the hardness map
        case 'H':
            break;

        //TODO Inspect inventory item
        case 'I':
            break;

        //TODO Look at monster
        case 'L':
            break;

        // Quit the game
        case 'Q':
            endwin();
            delete_dungeon(d);
            exit(0);

        //TODO Display the tunneling distance map
        case 'T':
            break;

        default:
            x = 0;
            y = 0;
            break;
    }

    if(d->map[d->pc->pos[dim_y] + y][d->pc->pos[dim_x] + x] == ter_wall ||
       d->map[d->pc->pos[dim_y] + y][d->pc->pos[dim_x] + x] == ter_wall_immutable)
    {y = 0, x = 0;}



    d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = NULL;
    d->pc->pos[dim_y] += y;
    d->pc->pos[dim_x] += x;
    d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = d->pc;
}

int pc_next_pos(dungeon_t *d)
{
    int hasMoved = 0;
    while(!hasMoved)
    {
        int dx = (rand() % 3) - 1;
        int dy = (rand() % 3) - 1;

   
        if(d->map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx] == ter_wall || d->map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx] == ter_wall_immutable){continue;}


        if (d->character_map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx] != NULL &&
            d->character_map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx] != d->pc)
        {
            d->character_map[d->pc->pos[dim_y] + dy][d->pc->pos[dim_x] + dx]->living = 0;
            d->num_monsters--;
        }
        d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = NULL;
        d->pc->pos[dim_y] += dy;
        d->pc->pos[dim_x] += dx;
        d->character_map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = d->pc;
        //update monster path making
        dijkstra_non_tunneling(d);
        dijkstra_tunneling(d);

        hasMoved = 1;
        
    }
    return 0;
}

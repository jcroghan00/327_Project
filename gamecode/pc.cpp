#include <ncurses.h>

#include "dungeon.h"
#include "Monster.h"
#include "Character.h"
#include "windows.h"
#include "pc.h"
#include "object.h"

typedef struct heap heap_t;

PC::PC():Character(){
    setDisplayChar('@');
    setSpeed(PC_SPEED);
    hitpoints = 1000;
    damage.base = 2;
    damage.numDice = 2;
    damage.numSides = 2;
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            pc_map[i][j] = ter_wall;
            vis_monsters[i][j] = NULL;
            visObj[i][j] = NULL;
        }
    }
    int carrySize = sizeof(carrySlots) / sizeof(carrySlots[0]);
    for (int i = 0; i < carrySize; i++){
        carrySlots[i] = NULL;
    }
    int equipSize = sizeof(equipSlots) / sizeof(equipSlots[0]);
    for (int i = 0; i < equipSize; i++){
        equipSlots[i] = NULL;
    }
}

void PC::reset_maps()
{
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            pc_map[i][j] = ter_wall;
            vis_monsters[i][j] = NULL;
            visObj[i][j] = NULL;
        }
    }
}

void PC::pickup_item(Dungeon *d){
    int size = sizeof d->pc->carrySlots / sizeof d->pc->carrySlots[0];
    Object *object = d->objMap[d->pc->pos[dim_y]][d->pc->pos[dim_x]];
    for (int i = 0; i < size;i++){
        if (!d->pc->carrySlots[i]){
            d->pc->carrySlots[i] = object;

            mvprintw(LINES-2,0,"Placed ");
            attron(COLOR_PAIR(object->color));
            printw(object->name.c_str());
            printw(" \'%c\'",object->displayChar);
            attroff(COLOR_PAIR(object->color));
            printw(" in slot %i",i);
            d->objMap[d->pc->pos[dim_y]][d->pc->pos[dim_x]] = NULL;
            return;
        }
    }
    mvprintw(LINES-2,0,"Not enough room to pickup  ");
    attron(COLOR_PAIR(object->color));
    printw(object->name.c_str());
    printw(" \'%c\'",object->displayChar);
    attroff(COLOR_PAIR(object->color));
    printw("!");
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

int PC::fight_monster(Dungeon *d, int dx, int dy, heap_t *h){
    // implement combat logic
    Monster *monster = (Monster*)character_mapxy(pos[dim_x] + dx,pos[dim_y] + dy);
    int damageDone = getDamage();
    monster->hitpoints -= damageDone;
    if (monster->hitpoints <= 0){
        if(monster->abil.BOSS){
            render_end_game(d, h);
        }

        monster->hitpoints = 0;
        mvprintw(LINES-1, 0, "Killed %s",monster->name.c_str());

        monster->setLiving(0);

        for (int i = 0; i < (int)d->monsters.size(); i++){
            if (d->monsters.at(i)->hitpoints == 0){
                d->monsters.erase(d->monsters.begin()+i);
            }
        }

        return 1;
    } else {
        mvprintw(LINES-2, 0, "You hit %s for %i damage!",
                 monster->name.c_str(), damageDone);
        mvprintw(LINES-1, 0, "Monster Health: %d",monster->hitpoints);
        return 0;
    }
}

int PC::move_pc(Dungeon *d, heap_t *h, int dy, int dx, int teleport = 0){
    // if not moving, end function
    if(!dy && !dx){
        return 0;
    }
    // disp wall message
    if(d->map[pos[dim_y] + dy][pos[dim_x] + dx] < ter_floor &&
       !teleport){
        const char *msg = "There's a wall there!";
        mvprintw(0, (COLS/2 - strlen(msg)/2), msg);
        move_pc_ncurses(d,h);
        return -1;
    }
    //Check for combat
    //TODO update combat
    if (d->character_map[pos[dim_y]+dy][pos[dim_x]+dx] != NULL){
        if(fight_monster(d, dx, dy, h)){
            d->character_map[d->pc->pos[dim_y]][pos[dim_x]] = NULL;
            pos[dim_y] += dy;
            pos[dim_x] += dx;
            vis_monsters[d->pc->pos[dim_y]][pos[dim_x]] = NULL;
            d->character_map[pos[dim_y]][pos[dim_x]] = this;
        }
    }
    else{
        d->character_map[d->pc->pos[dim_y]][pos[dim_x]] = NULL;
        pos[dim_y] += dy;
        pos[dim_x] += dx;
        d->character_map[pos[dim_y]][pos[dim_x]] = this;
    }

    //Check for object on the ground
    if (d->objMap[d->pc->pos[dim_y]][d->pc->pos[dim_x]]){
        pickup_item(d);
    }

    update_pc_map(d);
    update_vis_objects(d);
    return 0;
}

int PC::getDamage()
{
    int base;
    if(this->equipSlots[0]){
        base = equipSlots[0]->dam.roll();
    }
    else{
        base = this->damage.roll();
    }

    int size = sizeof this->equipSlots / sizeof this->equipSlots[0];
    int damage = base;

    for(int i = 1; i < size; ++i) {
        if (this->equipSlots[i]) {
            damage += this->equipSlots[i]->dam.roll();
        }
    }

    return damage;
}

int PC::getSpeed()
{
    int speed = 10;
    for(int i = 0; i < 12; ++i){
        if(equipSlots[i])
        {
            speed += equipSlots[i]->speed;
        }
    }
    return speed;
}

void move_pc_ncurses(Dungeon *d, heap_t *h)
{
    int val  = wgetch(stdscr);
    if (!d->pc->isLiving()){
        return;
    }
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
                d->new_dungeon(h);
            } else {
                const char *down_msg = "You can't go down here!";
                mvprintw(0, (COLS/2 - strlen(down_msg)/2), down_msg);
                move_pc_ncurses(d,h);
            }
            break;
        case '<':
            //TODO start on down stairs
            if(d->map[d->pc->pos[dim_y]][d->pc->pos[dim_x]] == ter_stairs_up){
                d->new_dungeon(h);
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

            // Display equipment
        case 'e':
            render_equipment(d);
            move_pc_ncurses(d, h);
            break;

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

            // Take off item
        case 't':
            render_unequip(d);
            move_pc_ncurses(d, h);
            goto jump;

            // Wear item
        case 'w':
            render_equip(d);
            move_pc_ncurses(d, h);
            break;

            //Expunge item
        case 'x':
            render_expunge(d);
            move_pc_ncurses(d, h);
            break;

            //Display the non-tunneling distance map
        case 'D':
            render_dist_map(d);
            move_pc_ncurses(d, h);
            break;

            // Inspect equipped item
        case 'E':
            render_inspect_equipment(d);
            move_pc_ncurses(d, h);
            goto jump;

            //Display the hardness map
        case 'H':
            render_hardness_map(d);
            move_pc_ncurses(d, h);
            break;

            //Inspect inventory item
        case 'I':
            render_inspect(d);
            move_pc_ncurses(d, h);
            break;

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
            d->pc->setLiving(0); //TODO testing
            
            break;

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

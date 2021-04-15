#include <ncurses.h>

#include "dungeon.h"
#include "heap.h"
#include "Monster.h"
#include "pc.h"
#include "Character.h"
#include "path.h"
#include "windows.h"
#include "object.h"
#include "parser.h"

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

#define malloc(size) ({        \
  void *_tmp;                  \
  assert((_tmp = malloc(size)));		\
  _tmp;                        \
})

uint32_t in_room(Room r, Character *c)
{
    if(c->pos[dim_x] >= r.position[dim_x] && c->pos[dim_x] < r.position[dim_x] + r.size[dim_x]
       && c->pos[dim_y] >= r.position[dim_y] && c->pos[dim_y] < r.position[dim_y] + r.size[dim_y])
    {
        return 1;
    }
    return 0;
}

int gaussian[5][5] = {
  {  1,  4,  7,  4,  1 },
  {  4, 16, 26, 16,  4 },
  {  7, 26, 41, 26,  7 },
  {  4, 16, 26, 16,  4 },
  {  1,  4,  7,  4,  1 }
};

typedef struct queue_node {
  int x, y;
  struct queue_node *next;
} queue_node_t;

int gen_dungeon(Dungeon *d)
{
  gen_objects(d);
  d->pc->update_pc_map(d);
  d->pc->update_vis_objects(d);
  //TODO should be moved outside of dungeon so windows can be game wide not dungeon specific
  d->windows = (Windows*)malloc(sizeof(Windows));
  create_windows(d);
  d->fow = 1;
  return 0;
}

int load_dungeon(Dungeon *d)
{
    // NON FUNCTIONING
    uint32_t version, file_size;
    char semantic[sizeof(SEMANTIC_FILE_MARKER)];
  char *home = getenv("HOME");
  const char *game_dir = ".rlg327";
  const char *save_file = "dungeon";

  char *path = (char*)malloc(strlen(home) + strlen(game_dir) + strlen(save_file) + 3);
  sprintf(path,"%s/%s/%s", home, game_dir, save_file);
  FILE *file = fopen(path,"r");
  free(path);

  fread(semantic, 1, sizeof(SEMANTIC_FILE_MARKER), file);

  if(strcmp(semantic, "RLG327-S2021")){
    return -1;
  }
  
  //version
  fread(&version, 4, 1, file);
  version = be32toh(version);
  //file size
  fread(&file_size, 4, 1, file);
  file_size = be32toh(file_size);

  //pc location
  fread(&d->pc->pos[dim_x], 1, 1, file);
  fread(&d->pc->pos[dim_y], 1, 1, file);

  //hardness
  fread(&d->hardness, 1, 1680, file);

  //rooms
  fread(&d->num_rooms, 2, 1, file);
  d->num_rooms = be16toh(d->num_rooms);

  d->rooms = (Room*)malloc(sizeof (*d->rooms) * d->num_rooms);

  for(uint32_t i = 0; i < d->num_rooms; ++i)
  {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;

    fread(&x, 1, 1, file);
    fread(&y, 1, 1, file);
    fread(&width, 1, 1, file);
    fread(&height, 1, 1, file);

    d->rooms[i].position[dim_x] = x;
    d->rooms[i].position[dim_y] = y;
    d->rooms[i].size[dim_x] = width;
    d->rooms[i].size[dim_y] = height;

    for(int j = y; j < y + height; j++){
      for(int k = x; k < x + width; k++){
	mapxy(k, j) = ter_floor_room;
      }
    }
  }

  for(int i = 0; i < DUNGEON_Y; i++){
    for(int j = 0; j < DUNGEON_X; j++){
      if((d->hardness[i][j] == 0) && (d->map[i][j] != ter_floor_room)){
	mapxy(j, i) = ter_floor_hall;
      }
    }
  }

  //stairs
  //TODO with stairs being malloced now make need to store stair locations in a buffer so the stairs array can be malloced
    uint16_t stairs_down = 0, stairs_up = 0;
  fread(&stairs_up, 2, 1, file);
  stairs_up = be16toh(stairs_up);

  int i;
  for(i = 0; i < stairs_up; ++i)
  {
    uint8_t x;
    uint8_t y;

    fread(&x, 1, 1, file);
    fread(&y, 1, 1, file);

    
    d->stairs[i].position[dim_x] = x;
    d->stairs[i].position[dim_y] = y;
    d->stairs[i].direction = mapxy(x, y) = ter_stairs_up;
  }
  fread(&stairs_down, 2, 1, file);
  stairs_down = be16toh(stairs_down);
  for(i = stairs_up; i < stairs_down+stairs_up; ++i)
  {
    uint8_t x;
    uint8_t y;

    fread(&x, 1, 1, file);
    fread(&y, 1, 1, file);

    
    d->stairs[i].position[dim_x] = x;
    d->stairs[i].position[dim_y] = y;
    d->stairs[i].direction = mapxy(x, y) = ter_stairs_down;
  }
    if (d->num_monsters == -1){d->num_monsters = d->num_rooms*2 < 50 ? d->num_rooms*2 : 50;}
    //config_pc(d);

  //monster path making
  //adds monsters to the dungeon
  //gen_monsters(d);

  return 0;
}

//renders the entire game board to a a given screen, stdscr by default
void render_ncurses(Dungeon *d, WINDOW *scr=stdscr, int render_items=1)
{
    pair_t p;
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            if (character_mappair(p))
            {
                if(character_mappair(p)->getDisplayChar() == '@')
                {
                    mvwaddch(scr,p[dim_y] + 1, p[dim_x], (character_mappair(p)->getDisplayChar()));
                } else{
                    wattron(scr,COLOR_PAIR(character_mappair(p)->dispColor));
                    mvwaddch(scr,p[dim_y] + 1, p[dim_x], (character_mappair(p)->getDisplayChar()));
                    wattroff(scr,COLOR_PAIR(character_mappair(p)->dispColor));
                }
            }
            else if(d->objMap[p[dim_y]][p[dim_x]] && render_items)
            {
                wattron(scr,COLOR_PAIR(d->objMap[p[dim_y]][p[dim_x]]->color));
                mvwaddch(scr,p[dim_y] + 1, p[dim_x], (d->objMap[p[dim_y]][p[dim_x]]->displayChar));
                wattroff(scr,COLOR_PAIR(d->objMap[p[dim_y]][p[dim_x]]->color));
            }
            else {
                switch (mappair(p)) {
                    case ter_wall:
                    case ter_wall_immutable:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],' ');
                        break;
                    case ter_floor:
                    case ter_floor_room:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],'.');
                        break;
                    case ter_floor_hall:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],'#');
                        break;
                    case ter_debug:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],'*');
                        //fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
                        break;
                    case ter_stairs_up:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],'<');
                        break;
                    case ter_stairs_down:
                        mvwaddch(scr,p[dim_y] + 1, p[dim_x],'>');
                        break;
                    default:
                        break;
                }
            }
        }
    }
    mvwprintw(scr, LINES - 1, 0, "HP: %d", d->pc->hitpoints);
}

void render_fow(Dungeon *d)
{
    start_color();
    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);

    pair_t p;
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
        for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
            if(p[dim_x] == d->pc->pos[dim_x] && p[dim_y] == d->pc->pos[dim_y])
            {
                mvaddch(p[dim_y] + 1, p[dim_x], '@');
            }
            else if (vismonsterpair(p))
            {
                //TODO render with monster color if currently visible, else white
                if(p[dim_x] >= d->pc->pos[dim_x] - 2 &&
                        p[dim_x] <= d->pc->pos[dim_x] + 2 &&
                        p[dim_y] >= d->pc->pos[dim_y] - 2 &&
                        p[dim_y] <= d->pc->pos[dim_y] + 2)
                {
                    attron(COLOR_PAIR(character_mappair(p)->dispColor));
                    mvaddch(p[dim_y] + 1, p[dim_x], (vismonsterpair(p)->getDisplayChar()));
                    attroff(COLOR_PAIR(character_mappair(p)->dispColor));
                } else{
                    mvaddch(p[dim_y] + 1, p[dim_x], (character_mappair(p)->getDisplayChar()));
                }
            }
            else if (visobjectmappair(p))
            {
                attron(COLOR_PAIR(d->objMap[p[dim_y]][p[dim_x]]->color));
                mvaddch(p[dim_y] + 1, p[dim_x], (visobjectmappair(p)->displayChar));
                attroff(COLOR_PAIR(d->objMap[p[dim_y]][p[dim_x]]->color));
            }
            else {
                switch (pcmappair(p)) {
                    case ter_wall:
                    case ter_wall_immutable:
                        mvaddch(p[dim_y] + 1, p[dim_x],' ');
                        break;
                    case ter_floor:
                    case ter_floor_room:
                        mvaddch(p[dim_y] + 1, p[dim_x],'.');
                        break;
                    case ter_floor_hall:
                        mvaddch(p[dim_y] + 1, p[dim_x],'#');
                        break;
                    case ter_debug:
                        mvaddch(p[dim_y] + 1, p[dim_x],'*');
                        //fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
                        break;
                    case ter_stairs_up:
                        mvaddch(p[dim_y] + 1, p[dim_x],'<');
                        break;
                    case ter_stairs_down:
                        mvaddch(p[dim_y] + 1, p[dim_x],'>');
                        break;
                    default:
                        break;
                }
            }
        }
    }
    mvprintw(LINES - 1, 0, "HP: %d", d->pc->hitpoints);
}

void render_teleport_select(Dungeon *d, heap_t *h){
    pair_t cursor;
    cursor[dim_x] = d->pc->pos[dim_x];
    cursor[dim_y] = d->pc->pos[dim_y];
    WINDOW *teleport_win = d->windows->teleport_win;
    const char *msg = "Press \'Q\' to cancel teleport";
    mvwprintw(teleport_win,0, (COLS/2 - strlen(msg)/2), msg);
    touchwin(teleport_win);
    int visible = 1;
    while (visible) {
        cursor[dim_x] = 0 < cursor[dim_x] ? cursor[dim_x] : 1;
        cursor[dim_y] = 0 < cursor[dim_y] ? cursor[dim_y] : 1;
        cursor[dim_x] = cursor[dim_x] < DUNGEON_X ? cursor[dim_x] : DUNGEON_X-1;
        cursor[dim_y] = cursor[dim_y] < DUNGEON_Y ? cursor[dim_y] : DUNGEON_Y-1;
        mvwprintw(teleport_win,0, (COLS/2 - strlen(msg)/2), msg);
        render_ncurses(d, teleport_win);
        mvwaddch(teleport_win,cursor[dim_y] + 1, cursor[dim_x],'*');
        int val = wgetch(teleport_win);
        switch (val) {
            // Quit the window
            case 'Q':
                visible = 0;
                touchwin(stdscr);
                break;
                // Move up-left
            case KEY_HOME:
            case '7':
            case 'y':
                cursor[dim_y]--;
                cursor[dim_x]--;
                break;
                // Move up
            case KEY_UP:
            case '8':
            case 'k':
                cursor[dim_y]--;
                break;
                // Move up-right
            case KEY_PPAGE:
            case '9':
            case 'u':
                cursor[dim_y]--;
                cursor[dim_x]++;
                break;
                // Move right
            case KEY_RIGHT:
            case '6':
            case 'l':
                cursor[dim_x]++;
                break;

                // Move down-right
            case KEY_NPAGE:
            case '3':
            case 'n':
                cursor[dim_y]++;
                cursor[dim_x]++;
                break;

                // Move down
            case KEY_DOWN:
            case '2':
            case 'j':
                cursor[dim_y]++;
                break;
                // Move down-left
            case KEY_END:
            case '1':
            case 'b':
                cursor[dim_y]++;
                cursor[dim_x]--;
                break;
                // Move left
            case KEY_LEFT:
            case '4':
            case 'h':
                cursor[dim_x]--;
                break;
            case 'r':
                visible = 0;
                touchwin(stdscr);
                cursor[dim_x] = rand() % (DUNGEON_X-1) + 1;
                cursor[dim_y] = rand() % (DUNGEON_Y-1) + 1;
                d->pc->move_pc(d, h,cursor[dim_y]-d->pc->pos[dim_y],cursor[dim_x]-d->pc->pos[dim_x],1);
                break;
            case 'g':
                visible = 0;
                touchwin(stdscr);
                d->pc->move_pc(d, h,cursor[dim_y] - d->pc->pos[dim_y],cursor[dim_x]- d->pc->pos[dim_x],1);
                break;
            default:
                break;
        }
    }
}

void render(Dungeon *d)
{
    if(d->fow){
        render_fow(d);
    }
    else{
        render_ncurses(d);
    }
}

void delete_dungeon(Dungeon *d, heap_t *h)
{
    heap_delete(h);
    free(d->rooms);
    for (int i = 0; i < d->num_monsters;i++){
        delete d->monsters[i];
    }
    delete d->pc;
    free(d->monsters);
}

uint16_t count_up_stairs(Dungeon *d){
    uint16_t x,y, up_stairs = 0;
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            if (mapxy(x,y) == ter_stairs_up){
                up_stairs++;
            }
        }
    }
    return up_stairs;
}

uint16_t count_down_stairs(Dungeon *d){
    uint16_t x,y, down_stairs = 0;
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            if (mapxy(x,y) == ter_stairs_down){
                down_stairs++;
            }
        }
    }
    return down_stairs;
}

uint32_t calc_file_size(Dungeon *d){
    return (1708 + (d->num_rooms * 4) +
            (count_up_stairs(d) * 2)  +
            (count_down_stairs(d) * 2));
}

int save_dungeon(Dungeon *d)
{
    FILE *file;
    uint32_t version, file_size, be32;
  char *home = getenv("HOME");
  version = htobe32(FILE_VERSION);
  file_size = calc_file_size(d);

  char *path = (char*)malloc(strlen(home) + sizeof(SAVE_DIR) + sizeof(SAVE_FILE) + 4);
  sprintf(path,"%s/%s/%s", home, SAVE_DIR, SAVE_FILE);
  if (!(file = fopen(path,"w"))){
        return -1;
    }
  free(path);

  // write the semantic file marker: 12 bytes
  fwrite(SEMANTIC_FILE_MARKER, 1, sizeof(SEMANTIC_FILE_MARKER)-1,file);

   be32= htobe32(version);
  fwrite(&be32,sizeof(be32),1,file);

  be32 = htobe32(file_size);
  fwrite(&be32,sizeof(be32),1,file);

  fwrite(&d->pc->pos[dim_x],1,1,file);
  fwrite(&d->pc->pos[dim_y],1,1,file);

  
  for (int i=0; i<DUNGEON_Y;i++)
    {
      for(int j=0; j<DUNGEON_X;j++)
	    {
	        fwrite(&d->hardness[i][j],1,1,file);
	    }
    }


  int tempnumrooms = d->num_rooms;
  d->num_rooms = htobe16(d->num_rooms);
  fwrite(&d->num_rooms,2,1,file);
  
  for (int i=0; i <tempnumrooms;i++)
    {
        fwrite(&d->rooms[i].position[dim_x],1,1,file);
        fwrite(&d->rooms[i].position[dim_y],1,1,file);
        fwrite(&d->rooms[i].size[dim_x],1,1,file);
        fwrite(&d->rooms[i].size[dim_y],1,1,file);
    }

//im sorry
 uint16_t stairs_up = count_up_stairs(d);
 uint16_t stairs_down = count_down_stairs(d);
 
  stairs_up = htobe16(stairs_up);
  fwrite(&stairs_up,2,1,file);
  //may be issue with stairs being set to size of 255, should be malloced
  for (uint32_t i =0; i <(sizeof(d->stairs));i++)
    {
      //write only the up stairs
      if (d->stairs[i].direction == ter_stairs_up){

      fwrite(&d->stairs[i].position[dim_x],1,1,file);
      fwrite(&d->stairs[i].position[dim_y],1,1,file);
      }
    }
  
  stairs_down = htobe16(stairs_down);
  fwrite(&stairs_down,2,1,file);
  for (uint32_t i =0; i <(sizeof(d->stairs));i++)
    {
      //write only the down stairs
      if (d->stairs[i].direction == ter_stairs_down){
      fwrite(&d->stairs[i].position[dim_x],1,1,file);
      fwrite(&d->stairs[i].position[dim_y],1,1,file);
      }
    }
  return 0;
}

Dungeon::Dungeon(int numMon){
    monster_parser();
    object_parser();

    //generate the dungeon
    empty_dungeon();
    do {
        make_rooms();
    } while (place_rooms());
    connect_rooms();
    place_stairs();

    //initialize maps
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            character_map[i][j] = NULL;
            objMap[i][j] = NULL;
        }
    }

    //generate the pc
    pc = new PC();
    int randRoom = rand() % num_rooms;
    int x = rand() % rooms[randRoom].size[dim_x];
    int y = rand() % rooms[randRoom].size[dim_y];
    pc->pos[dim_x] = rooms[randRoom].position[dim_x] + x;
    pc->pos[dim_y] = rooms[randRoom].position[dim_y] + y;
    character_map[pc->pos[dim_y]][pc->pos[dim_x]] = pc;

    //generate monsters
    num_monsters = numMon;
    gen_monsters();
}

void Dungeon::new_dungeon(heap_t *h)
{
    delete_dungeon(this, h);
    num_monsters = -1;

    //generate the dungeon
    empty_dungeon();
    do {
        make_rooms();
    } while (place_rooms());
    connect_rooms();
    place_stairs();

    //initialize maps
    for(int i = 0; i < DUNGEON_Y; i++){
        for(int j = 0; j < DUNGEON_X; j++){
            character_map[i][j] = NULL;
            objMap[i][j] = NULL;
        }
    }

    //generate the pc
    pc = new PC();
    int randRoom = rand() % num_rooms;
    int x = rand() % rooms[randRoom].size[dim_x];
    int y = rand() % rooms[randRoom].size[dim_y];
    pc->pos[dim_x] = rooms[randRoom].position[dim_x] + x;
    pc->pos[dim_y] = rooms[randRoom].position[dim_y] + y;
    character_map[pc->pos[dim_y]][pc->pos[dim_x]] = pc;

    //generate monsters
    num_monsters = -1;
    gen_monsters();

    gen_dungeon(this);

    heap_init(h,character_cmp,NULL);
    for(int i = 0; i < num_monsters; i++)
    {
        monsters[i]->setSd(i);
        heap_insert(h,monsters[i]);
    }

    clear();
    render_ncurses(this);
    refresh();
}

void Dungeon::gen_monsters(){
    if (num_monsters == -1) {num_monsters = num_rooms * 2 < 50 ? num_rooms * 2 : 50;}
    monsters = (Monster**)calloc((num_monsters),sizeof(Monster) * (num_monsters));
    for(int i = 0; i < num_monsters; i++) {
        monsters[i] = new Monster();
    }
    int pcRoomNum;
    int totalArea = 0;
    for(uint32_t i = 0; i < num_rooms; ++i)
    {
        if (in_room(rooms[i],pc)){
            pcRoomNum = i;
        } else {
            totalArea += rooms[i].size[dim_x] * rooms[i].size[dim_y];
        }
    }

    int totalMonsters = 0;
    while(totalMonsters < num_monsters)
    {
        if(totalMonsters == totalArea){break;}

        int randRoom = rand() % num_rooms;

        if(randRoom == pcRoomNum){continue;}

        int x = rand() % rooms[randRoom].size[dim_x];
        int y = rand() % rooms[randRoom].size[dim_y];

        if(character_map[rooms[randRoom].position[dim_y] + y][rooms[randRoom].position[dim_x] + x] != NULL){continue;}

        character_map[rooms[randRoom].position[dim_y] + y][rooms[randRoom].position[dim_x] + x] = monsters[totalMonsters];

        monsters[totalMonsters]->pos[dim_y] = rooms[randRoom].position[dim_y] + y;
        monsters[totalMonsters]->pos[dim_x] = rooms[randRoom].position[dim_x] + x;
        monsters[totalMonsters]->destination[dim_y] = monsters[totalMonsters]->pos[dim_y];
        monsters[totalMonsters]->destination[dim_x] = monsters[totalMonsters]->pos[dim_x];

        ++totalMonsters;
    }
}

uint32_t Dungeon::adjacent_to_room(int16_t y, int16_t x)
{
    return (map[y][x-1] == ter_floor_room ||
            map[y][x+1] == ter_floor_room ||
            map[y-1][x] == ter_floor_room ||
            map[y+1][x] == ter_floor_room);
}

uint32_t Dungeon::is_open_space(int16_t y, int16_t x){return !hardness[y][x];}

void Dungeon::dijkstra_corridor(pair_t from, pair_t to)
{
    static Corridor_Path path[DUNGEON_Y][DUNGEON_X], *p;
    static uint32_t initialized = 0;
    heap_t h;
    uint32_t x, y;

    if (!initialized) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path[y][x].pos[dim_y] = y;
                path[y][x].pos[dim_x] = x;
            }
        }
        initialized = 1;
    }

    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            path[y][x].cost = INT_MAX;
        }
    }

    path[from[dim_y]][from[dim_x]].cost = 0;

    heap_init(&h, corridor_path_cmp, NULL);

    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            if (map[y][x] != ter_wall_immutable) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            } else {
                path[y][x].hn = NULL;
            }
        }
    }

    while ((p = (Corridor_Path*)heap_remove_min(&h))) {
        p->hn = NULL;

        if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
            for (x = to[dim_x], y = to[dim_y];
                 (x != (unsigned)from[dim_x]) || (y != (unsigned)from[dim_y]);
                 p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
                if (map[y][x] != ter_floor_room) {
                    map[y][x] = ter_floor_hall;
                    hardness[y][x] = 0;
                }
            }
            heap_delete(&h);
            return;
        }

        if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
            (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
             p->cost + dhardnesspair(p->pos))) {
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
                    p->cost + dhardnesspair(p->pos);
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
            [p->pos[dim_x]    ].hn);
        }
        if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
            (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
             p->cost + dhardnesspair(p->pos))) {
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
                    p->cost + dhardnesspair(p->pos);
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
            [p->pos[dim_x] - 1].hn);
        }
        if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
            (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
             p->cost + dhardnesspair(p->pos))) {
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
                    p->cost + dhardnesspair(p->pos);
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
            [p->pos[dim_x] + 1].hn);
        }
        if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
            (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
             p->cost + dhardnesspair(p->pos))) {
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
                    p->cost + dhardnesspair(p->pos);
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
            [p->pos[dim_x]    ].hn);
        }
    }
}

void Dungeon::dijkstra_corridor_inv(pair_t from, pair_t to)
{
    /* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
    static Corridor_Path path[DUNGEON_Y][DUNGEON_X], *p;
    static uint32_t initialized = 0;
    heap_t h;
    uint32_t x, y;

    if (!initialized) {
        for (y = 0; y < DUNGEON_Y; y++) {
            for (x = 0; x < DUNGEON_X; x++) {
                path[y][x].pos[dim_y] = y;
                path[y][x].pos[dim_x] = x;
            }
        }
        initialized = 1;
    }

    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            path[y][x].cost = INT_MAX;
        }
    }

    path[from[dim_y]][from[dim_x]].cost = 0;

    heap_init(&h, corridor_path_cmp, NULL);

    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            if (map[y][x] != ter_wall_immutable) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            } else {
                path[y][x].hn = NULL;
            }
        }
    }

    while ((p = (Corridor_Path*)heap_remove_min(&h))) {
        p->hn = NULL;

        if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
            for (x = to[dim_x], y = to[dim_y];
                 (x != (unsigned)from[dim_x]) || (y != (unsigned)from[dim_y]);
                 p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
                if (map[y][x] != ter_floor_room) {
                    map[y][x] = ter_floor_hall;
                    hardness[y][x] = 0;
                }
            }
            heap_delete(&h);
            return;
        }

#define hardnesspair_inv(p) (is_open_space(p[dim_y], p[dim_x]) ? 127 :     \
                             (adjacent_to_room(p[dim_y], p[dim_x]) ? 191 : \
                              (255 - dhardnesspair(p))))

        if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
            (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
             p->cost + hardnesspair_inv(p->pos))) {
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
                    p->cost + hardnesspair_inv(p->pos);
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
            [p->pos[dim_x]    ].hn);
        }
        if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
            (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
             p->cost + hardnesspair_inv(p->pos))) {
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
                    p->cost + hardnesspair_inv(p->pos);
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
            [p->pos[dim_x] - 1].hn);
        }
        if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
            (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
             p->cost + hardnesspair_inv(p->pos))) {
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
                    p->cost + hardnesspair_inv(p->pos);
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
            [p->pos[dim_x] + 1].hn);
        }
        if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
            (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
             p->cost + hardnesspair_inv(p->pos))) {
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
                    p->cost + hardnesspair_inv(p->pos);
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
            [p->pos[dim_x]    ].hn);
        }
    }
}

int Dungeon::smoothhardness(Dungeon *d){
    int32_t i, x, y;
    int32_t s, t, p, q;
    queue_node_t *head, *tail, *tmp;
    FILE *out;
    uint8_t hardness_t[DUNGEON_Y][DUNGEON_X];

    memset(&hardness_t, 0, sizeof (hardness_t));

    /* Seed with some values */
    for (i = 1; i < 255; i += 20) {
        do {
            x = rand() % DUNGEON_X;
            y = rand() % DUNGEON_Y;
        } while (hardness_t[y][x]);
        hardness_t[y][x] = i;
        if (i == 1) {
            head = tail = (queue_node_t*)malloc(sizeof (*tail));
        } else {
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
        }
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
    }

    out = fopen("seeded.pgm", "w");
    fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
    fwrite(&hardness_t, sizeof (hardness_t), 1, out);
    fclose(out);

    /* Diffuse the values to fill the space */
    while (head) {
        x = head->x;
        y = head->y;
        i = hardness_t[y][x];

        if (x - 1 >= 0 && y - 1 >= 0 && !hardness_t[y - 1][x - 1]) {
            hardness_t[y - 1][x - 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x - 1;
            tail->y = y - 1;
        }
        if (x - 1 >= 0 && !hardness_t[y][x - 1]) {
            hardness_t[y][x - 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x - 1;
            tail->y = y;
        }
        if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness_t[y + 1][x - 1]) {
            hardness_t[y + 1][x - 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x - 1;
            tail->y = y + 1;
        }
        if (y - 1 >= 0 && !hardness_t[y - 1][x]) {
            hardness_t[y - 1][x] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x;
            tail->y = y - 1;
        }
        if (y + 1 < DUNGEON_Y && !hardness_t[y + 1][x]) {
            hardness_t[y + 1][x] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x;
            tail->y = y + 1;
        }
        if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness_t[y - 1][x + 1]) {
            hardness_t[y - 1][x + 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x + 1;
            tail->y = y - 1;
        }
        if (x + 1 < DUNGEON_X && !hardness_t[y][x + 1]) {
            hardness_t[y][x + 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x + 1;
            tail->y = y;
        }
        if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness_t[y + 1][x + 1]) {
            hardness_t[y + 1][x + 1] = i;
            tail->next = (queue_node_t*)malloc(sizeof (*tail));
            tail = tail->next;
            tail->next = NULL;
            tail->x = x + 1;
            tail->y = y + 1;
        }

        tmp = head;
        head = head->next;
        free(tmp);
    }

    /* And smooth it a bit with a gaussian convolution */
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            for (s = t = p = 0; p < 5; p++) {
                for (q = 0; q < 5; q++) {
                    if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
                        x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
                        s += gaussian[p][q];
                        t += hardness_t[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
                    }
                }
            }
            d->hardness[y][x] = t / s;
        }
    }
    /* Let's do it again, until it's smooth like Kenny G. */
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            for (s = t = p = 0; p < 5; p++) {
                for (q = 0; q < 5; q++) {
                    if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
                        x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
                        s += gaussian[p][q];
                        t += hardness_t[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
                    }
                }
            }
            d->hardness[y][x] = t / s;
        }
    }


    out = fopen("diffused.pgm", "w");
    fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
    fwrite(&hardness_t, sizeof (hardness_t), 1, out);
    fclose(out);

    out = fopen("smoothed.pgm", "w");
    fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
    fwrite(&d->hardness, sizeof (d->hardness), 1, out);
    fclose(out);

    return 0;
}

int Dungeon::empty_dungeon(){
    uint8_t x, y;
    smoothhardness(this);
    for (y = 0; y < DUNGEON_Y; y++) {
        for (x = 0; x < DUNGEON_X; x++) {
            map[y][x] = ter_wall;
            if (y == 0 || y == DUNGEON_Y - 1 ||
                x == 0 || x == DUNGEON_X - 1) {
                map[y][x] = ter_wall_immutable;
                hardness[y][x] = 255;
            }
        }
    }
    return 0;
}

int Dungeon::make_rooms(){
    uint32_t i;

    for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(5, 8); i++)
        ;
    num_rooms = i;
    rooms = (Room*)malloc(sizeof (*rooms) * num_rooms);

    for (i = 0; i < num_rooms; i++) {
        rooms[i].size[dim_x] = ROOM_MIN_X;
        rooms[i].size[dim_y] = ROOM_MIN_Y;
        while (rand_under(3, 5) && rooms[i].size[dim_x] < ROOM_MAX_X) {
            rooms[i].size[dim_x]++;
        }
        while (rand_under(3, 5) && rooms[i].size[dim_y] < ROOM_MAX_Y) {
            rooms[i].size[dim_y]++;
        }
    }

    return 0;
}

int Dungeon::place_rooms(){
    pair_t p;
    uint32_t i;
    int success;
    Room *r;

    for (success = 0; !success; ) {
        success = 1;
        for (i = 0; success && i < num_rooms; i++) {
            r = rooms + i;
            r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
            r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
            for (p[dim_y] = r->position[dim_y] - 1;
                 success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
                 p[dim_y]++) {
                for (p[dim_x] = r->position[dim_x] - 1;
                     success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
                     p[dim_x]++) {
                    if (map[p[dim_y]][p[dim_x]] >= ter_floor) {
                        success = 0;
                        empty_dungeon();
                    } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                               (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                               (p[dim_x] != r->position[dim_x] - 1)              &&
                               (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
                        map[p[dim_y]][p[dim_x]] = ter_floor_room;
                        hardness[p[dim_y]][p[dim_x]] = 0;
                    }
                }
            }
        }
    }

    return 0;
}

int Dungeon::connect_two_rooms(Room *r1, Room *r2)
{
    /* Chooses a random point inside each room and connects them with a *
    * corridor.  Random internal points prevent corridors from exiting *
    * rooms in predictable locations.                                  */
    pair_t e1, e2;

    e1[dim_y] = rand_range(r1->position[dim_y],
                           r1->position[dim_y] + r1->size[dim_y] - 1);
    e1[dim_x] = rand_range(r1->position[dim_x],
                           r1->position[dim_x] + r1->size[dim_x] - 1);
    e2[dim_y] = rand_range(r2->position[dim_y],
                           r2->position[dim_y] + r2->size[dim_y] - 1);
    e2[dim_x] = rand_range(r2->position[dim_x],
                           r2->position[dim_x] + r2->size[dim_x] - 1);

    /*  return connect_two_points_recursive(d, e1, e2);*/
    dijkstra_corridor(e1, e2);

    return 0;
}

int Dungeon::create_cycle()
{
    /* Find the (approximately) farthest two rooms, then connect *
     * them by the shortest path using inverted hardnesses.      */

    uint32_t max, tmp, i, j, p, q;
    pair_t e1, e2;

    for (i = max = 0; i < num_rooms - 1; i++) {
        for (j = i + 1; j < num_rooms; j++) {
            tmp = (((rooms[i].position[dim_x] - rooms[j].position[dim_x])  *
                    (rooms[i].position[dim_x] - rooms[j].position[dim_x])) +
                   ((rooms[i].position[dim_y] - rooms[j].position[dim_y])  *
                    (rooms[i].position[dim_y] - rooms[j].position[dim_y])));
            if (tmp > max) {
                max = tmp;
                p = i;
                q = j;
            }
        }
    }

    /* Can't simply call connect_two_rooms() because it doesn't *
     * use inverse hardnesses, so duplicate it here.            */
    e1[dim_y] = rand_range(rooms[p].position[dim_y],
                           (rooms[p].position[dim_y] +
                            rooms[p].size[dim_y] - 1));
    e1[dim_x] = rand_range(rooms[p].position[dim_x],
                           (rooms[p].position[dim_x] +
                            rooms[p].size[dim_x] - 1));
    e2[dim_y] = rand_range(rooms[q].position[dim_y],
                           (rooms[q].position[dim_y] +
                            rooms[q].size[dim_y] - 1));
    e2[dim_x] = rand_range(rooms[q].position[dim_x],
                           (rooms[q].position[dim_x] +
                            rooms[q].size[dim_x] - 1));

    dijkstra_corridor_inv(e1, e2);

    return 0;
}

int Dungeon::connect_rooms(){
    uint32_t i;

    for (i = 1; i < num_rooms; i++) {
        connect_two_rooms( rooms + i - 1, rooms + i);
    }

    create_cycle();

    return 0;
}

void Dungeon::place_stairs()
{
    pair_t p;
    int i = 0;
    Stair *s;
    uint16_t stairs_up = rand() % 3 + 1, stairs_down = rand() % 3 + 1;
    stairs = (Stair*)malloc((stairs_up+stairs_down)*sizeof(Stair));
    do {
        while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
               (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
               ((map[p[dim_y]][p[dim_x]] < ter_floor) || (map[p[dim_y]][p[dim_x]] > ter_stairs)));
        s = stairs + i;
        i++;
        s->position[dim_y] = p[dim_y];
        s->position[dim_x] = p[dim_x];
        s->direction = map[p[dim_y]][p[dim_x]] = ter_stairs_down;
    } while (i < stairs_down);

    do {
        while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
               (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
               ((map[p[dim_y]][p[dim_x]] < ter_floor)                 ||
                (map[p[dim_y]][p[dim_x]] > ter_stairs)))

            ;
        s = stairs + i;
        i++;
        s->position[dim_y] = p[dim_y];
        s->position[dim_x] = p[dim_x];
        s->direction = map[p[dim_y]][p[dim_x]] = ter_stairs_up;
    } while (i < stairs_up + stairs_down);
}

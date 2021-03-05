
#include "dungeon.h"
#include "heap.h"
#include "monster.h"

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))
# define UNUSED(f) ((void) f)

#define malloc(size) ({        \
  void *_tmp;                  \
  assert((_tmp = malloc(size)));		\
  _tmp;                        \
})

typedef struct file_info {
  char file_type[13];
  uint32_t version;
  uint32_t file_size;
  int load;
  int save;
  int ren_non_tun_dist_map;
  int ren_tun_dist_map;
} file_info_t;


static int32_t character_cmp(const void *key, const void *with) {
    if (((character_t *) key)->turn != ((character_t *) with)->turn){
        return ((character_t *) key)->turn - ((character_t *) with)->turn;
    }
    else{
        return ((character_t *) key)->sd - ((character_t *) with)->sd;
    }
}


typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} corridor_path_t;

static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position[dim_x]) &&
        (x < (d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x])) &&
        (y >= d->rooms[i].position[dim_y]) &&
        (y < (d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y]))) {
      return 1;
    }
  }

  return 0;
}

static uint32_t adjacent_to_room(dungeon_t *d, int16_t y, int16_t x)
{
  return (mapxy(x - 1, y) == ter_floor_room ||
          mapxy(x + 1, y) == ter_floor_room ||
          mapxy(x, y - 1) == ter_floor_room ||
          mapxy(x, y + 1) == ter_floor_room);
}

static uint32_t is_open_space(dungeon_t *d, int16_t y, int16_t x)
{
  return !hardnessxy(x, y);
}

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
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
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

/* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
static void dijkstra_corridor_inv(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
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
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

#define hardnesspair_inv(p) (is_open_space(d, p[dim_y], p[dim_x]) ? 127 :     \
                             (adjacent_to_room(d, p[dim_y], p[dim_x]) ? 191 : \
                              (255 - hardnesspair(p))))

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

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
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
  dijkstra_corridor(d, e1, e2);

  return 0;
}

static int create_cycle(dungeon_t *d)
{
  /* Find the (approximately) farthest two rooms, then connect *
   * them by the shortest path using inverted hardnesses.      */

  int32_t max, tmp, i, j, p, q;
  pair_t e1, e2;

  for (i = max = 0; i < d->num_rooms - 1; i++) {
    for (j = i + 1; j < d->num_rooms; j++) {
      tmp = (((d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])  *
              (d->rooms[i].position[dim_x] - d->rooms[j].position[dim_x])) +
             ((d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])  *
              (d->rooms[i].position[dim_y] - d->rooms[j].position[dim_y])));
      if (tmp > max) {
        max = tmp;
        p = i;
        q = j;
      }
    }
  }

  /* Can't simply call connect_two_rooms() because it doesn't *
   * use inverse hardnesses, so duplicate it here.            */
  e1[dim_y] = rand_range(d->rooms[p].position[dim_y],
                         (d->rooms[p].position[dim_y] +
                          d->rooms[p].size[dim_y] - 1));
  e1[dim_x] = rand_range(d->rooms[p].position[dim_x],
                         (d->rooms[p].position[dim_x] +
                          d->rooms[p].size[dim_x] - 1));
  e2[dim_y] = rand_range(d->rooms[q].position[dim_y],
                         (d->rooms[q].position[dim_y] +
                          d->rooms[q].size[dim_y] - 1));
  e2[dim_x] = rand_range(d->rooms[q].position[dim_x],
                         (d->rooms[q].position[dim_x] +
                          d->rooms[q].size[dim_x] - 1));

  dijkstra_corridor_inv(d, e1, e2);

  return 0;
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
  }

  create_cycle(d);

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

static int smooth_hardness(dungeon_t *d)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  FILE *out;
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];

  memset(&hardness, 0, sizeof (hardness));

  /* Seed with some values */
  for (i = 1; i < 255; i += 20) {
    do {
      x = rand() % DUNGEON_X;
      y = rand() % DUNGEON_Y;
    } while (hardness[y][x]);
    hardness[y][x] = i;
    if (i == 1) {
      head = tail = malloc(sizeof (*tail));
    } else {
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);

  /* Diffuse the vaules to fill the space */
  while (head) {
    x = head->x;
    y = head->y;
    i = hardness[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !hardness[y - 1][x - 1]) {
      hardness[y - 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !hardness[y][x - 1]) {
      hardness[y][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness[y + 1][x - 1]) {
      hardness[y + 1][x - 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !hardness[y - 1][x]) {
      hardness[y - 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < DUNGEON_Y && !hardness[y + 1][x]) {
      hardness[y + 1][x] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness[y - 1][x + 1]) {
      hardness[y - 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < DUNGEON_X && !hardness[y][x + 1]) {
      hardness[y][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness[y + 1][x + 1]) {
      hardness[y + 1][x + 1] = i;
      tail->next = malloc(sizeof (*tail));
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
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
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
            t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      d->hardness[y][x] = t / s;
    }
  }


  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&hardness, sizeof (hardness), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
  fwrite(&d->hardness, sizeof (d->hardness), 1, out);
  fclose(out);

  return 0;
}

static int empty_dungeon(dungeon_t *d)
{
  uint8_t x, y;

  smooth_hardness(d);
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = ter_wall;
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = ter_wall_immutable;
        hardnessxy(x, y) = 255;
      }
    }
  }

  return 0;
}

static int place_rooms(dungeon_t *d)
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
      r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
      for (p[dim_y] = r->position[dim_y] - 1;
           success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
           p[dim_y]++) {
        for (p[dim_x] = r->position[dim_x] - 1;
             success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
             p[dim_x]++) {
          if (mappair(p) >= ter_floor) {
            success = 0;
            empty_dungeon(d);
          } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                     (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                     (p[dim_x] != r->position[dim_x] - 1)              &&
                     (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
            mappair(p) = ter_floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }

  return 0;
}

static void place_stairs(dungeon_t *d)
{
  pair_t p;
  int i = 0;
  stair_t *s;

  //I really don't know why this is needed
  d->stairs_up = 0;
  
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      ;
    s = d->stairs + i;
    d->stairs_down = d->stairs_down +1;
    i++;
    s->position[dim_y] = p[dim_y];
    s->position[dim_x] = p[dim_x];
    s->up_down = 0;
    mappair(p) = ter_stairs_down;
  } while (rand_under(1, 3));

  d->stairs_up = 0;
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      
      ;
    s = d->stairs + i;
    d->stairs_up = d->stairs_up +1;
    i++;
    s->position[dim_y] = p[dim_y];
    s->position[dim_x] = p[dim_x];
    s->up_down = 1;
    mappair(p) = ter_stairs_up;
  } while (rand_under(2, 4));
}

static int make_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(5, 8); i++)
    ;
  d->num_rooms = i;
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size[dim_x] = ROOM_MIN_X;
    d->rooms[i].size[dim_y] = ROOM_MIN_Y;
    while (rand_under(3, 5) && d->rooms[i].size[dim_x] < ROOM_MAX_X) {
      d->rooms[i].size[dim_x]++;
    }
    while (rand_under(3, 5) && d->rooms[i].size[dim_y] < ROOM_MAX_Y) {
      d->rooms[i].size[dim_y]++;
    }
  }

  return 0;
}

static void place_pc(dungeon_t *d)
{
    //if num monsters not specified, set it to twice the number of rooms or 50
    //whichever is smaller
    if (d->num_monsters == -1){d->num_monsters = d->num_rooms*2 < 50 ? d->num_rooms*2 : 50;}

    int randRoom = rand() % d->num_rooms;
    int x = rand() % d->rooms[randRoom].size[dim_x];
    int y = rand() % d->rooms[randRoom].size[dim_y];

    d->characters[0].x = d->rooms[randRoom].position[dim_x] + x;
    d->characters[0].y = d->rooms[randRoom].position[dim_y] + y;

    d->characters[0].pc = &d->pc;
    d->characters[0].display_char = get_display_char(&d->characters[0]);
    d->characters[0].turn = 0;
    d->characters[0].sd = 0;
    d->character_map[d->characters[0].y][d->characters[0].x] = &d->characters[0];

    d->characters[0].living = 1;

    printf("pcx: %d\n", d->characters[0].x);
    printf("pcy: %d\n", d->characters[0].y);
}
// Function to add monsters to the dungeon
int gen_monsters(dungeon_t *d)
{
    for(int i = 1; i <= d->num_monsters; i++)
    {
        monster_t m;
        d->characters[i].monster = &m;
        d->characters[i].living = 1;
        //d->characters[i].monster->intelligent = rand() % 2;
        d->characters[i].monster->intelligent = 0;
        //d->characters[i].monster->telepath = rand() % 2;
        d->characters[i].monster->telepath = 1;
        //d->characters[i].monster->tunneling = rand() % 2;
        d->characters[i].monster->tunneling = 1;
        //d->characters[i].monster->erratic = rand() % 2;
        d->characters[i].monster->erratic = 0;
        d->characters[i].monster->speed = rand() % 16 + 5;

        d->characters[i].display_char = get_display_char(&d->characters[i]);
        d->characters[i].turn = 0;
        d->characters[i].sd = i;
    }
    int pcRoomNum;
    int totalArea = 0;
    for(int i = 0; i < d->num_rooms; ++i)
    {
        if(d->characters[0].x >= d->rooms[i].position[dim_x] && d->characters[0].x < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x]
                && d->characters[0].y >= d->rooms[i].position[dim_y] && d->characters[0].y < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y])
        {
            pcRoomNum = i;
        }
        else{
            totalArea += d->rooms[i].size[dim_x] * d->rooms[i].size[dim_y];
        }

    }

    int totalMonsters = 1;
    while(totalMonsters <= d->num_monsters)
    {
        if(totalMonsters == totalArea){break;}

        int randRoom = rand() % d->num_rooms;

        if(randRoom == pcRoomNum){continue;}

        int x = rand() % d->rooms[randRoom].size[dim_x];
        int y = rand() % d->rooms[randRoom].size[dim_y];

        if(d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x]){continue;}

        d->character_map[d->rooms[randRoom].position[dim_y] + y][d->rooms[randRoom].position[dim_x] + x] = &d->characters[totalMonsters];
        d->characters[totalMonsters].y = d->rooms[randRoom].position[dim_y] + y;
        d->characters[totalMonsters].x = d->rooms[randRoom].position[dim_x] + x;
        ++totalMonsters;
    }
    return 0;
}

int gen_dungeon(dungeon_t *d)
{
  empty_dungeon(d);

  do {
    make_rooms(d);
  } while (place_rooms(d));
  connect_rooms(d);
  place_stairs(d);
  place_pc(d);
  dijkstra_non_tunneling(d);
  dijkstra_tunneling(d);
  gen_monsters(d);
  return 0;
}

int load_dungeon(dungeon_t *d, file_info_t *f)
{
  char *home = getenv("HOME");
  char *game_dir = ".rlg327";
  char *save_file = "dungeon";

  char *path = malloc(strlen(home) + strlen(game_dir) + strlen(save_file) + 3);
  sprintf(path,"%s/%s/%s", home, game_dir, save_file);
  FILE *file = fopen(path,"r");
  free(path);

  fread(f->file_type, 1, 12, file);

  if(strcmp(f->file_type, "RLG327-S2021")){
    return -1;
  }
  
  //version
  fread(&f->version, 4, 1, file);
  f->version = be32toh(f->version);
  //file size
  fread(&f->file_size, 4, 1, file);
  f->file_size = be32toh(f->file_size);

  //pc location
  fread(&d->characters[0].x, 1, 1, file);
  fread(&d->characters[0].y, 1, 1, file);

  //hardness
  fread(&d->hardness, 1, 1680, file);

  //rooms
  fread(&d->num_rooms, 2, 1, file);
  d->num_rooms = be16toh(d->num_rooms);

  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for(int i = 0; i < d->num_rooms; ++i)
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
  fread(&d->stairs_up, 2, 1, file);
  d->stairs_up = be16toh(d->stairs_up);

  int i;
  for(i = 0; i < d->stairs_up; ++i)
  {
    uint8_t x;
    uint8_t y;

    fread(&x, 1, 1, file);
    fread(&y, 1, 1, file);

    
    d->stairs[i].position[dim_x] = x;
    d->stairs[i].position[dim_y] = y;
    d->stairs[i].up_down = 1;
    
    mapxy(x, y) = ter_stairs_up;
  }
  
  fread(&d->stairs_down, 2, 1, file);
  d->stairs_down = be16toh(d->stairs_down);
  for(i = d->stairs_up; i < d->stairs_down+d->stairs_up; ++i)
  {
    uint8_t x;
    uint8_t y;

    fread(&x, 1, 1, file);
    fread(&y, 1, 1, file);

    
    d->stairs[i].position[dim_x] = x;
    d->stairs[i].position[dim_y] = y;
    d->stairs[i].up_down = 0;
    
    mapxy(x, y) = ter_stairs_down;
  }
    if (d->num_monsters == -1){d->num_monsters = d->num_rooms*2 < 50 ? d->num_rooms*2 : 50;}
    d->characters = malloc(sizeof(character_t) * (d->num_monsters + 1));
    d->characters[0].pc = &d->pc;
    d->characters[0].display_char = get_display_char(&d->characters[0]);

  //monster pathmaking
  dijkstra_non_tunneling(d);
  dijkstra_tunneling(d);
  //adds monsters to the dungeon
  gen_monsters(d);

  return 0;
}

void render_dungeon(dungeon_t *d, file_info_t *f)
{
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
      for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
          if (character_mappair(p))
          {
            putchar((character_mappair(p)->display_char));
          }
          else {
              switch (mappair(p)) {
                  case ter_wall:
                  case ter_wall_immutable:
                      putchar(' ');
                      break;
                  case ter_floor:
                  case ter_floor_room:
                      putchar('.');
                      break;
                  case ter_floor_hall:
                      putchar('#');
                      break;
                  case ter_debug:
                      putchar('*');
                      //fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
                      break;
                  case ter_stairs_up:
                      putchar('<');
                      break;
                  case ter_stairs_down:
                      putchar('>');
                      break;
                  default:
                      break;
              }
          }
      }
      putchar('\n');
  }
    //render the non-tunneling distance map if specified
  if (f->ren_non_tun_dist_map){
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
      for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
          if(d->non_tun_path[p[dim_y]][p[dim_x]].cost == 0){
              printf("@");
          }
	else if(d->non_tun_path[p[dim_y]][p[dim_x]].cost == INT_MAX){
	  if(d->hardness[p[dim_y]][p[dim_x]] == 0){
	    printf("X");
	  }
	  else{
	    printf(" ");
	  }
	}
	else{
	  printf("%d", d->non_tun_path[p[dim_y]][p[dim_x]].cost % 10);
	}
      }
      printf("\n");
    }
  }

  if (f->ren_tun_dist_map){
    for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
      for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
          if(d->tun_path[p[dim_y]][p[dim_x]].cost == 0){
              printf("@");
          }
	else if(d->tun_path[p[dim_y]][p[dim_x]].cost == INT_MAX){
	  printf(" ");
	}
	else{
	  printf("%d", d->tun_path[p[dim_y]][p[dim_x]].cost % 10);
	}
      }
    }
    printf("\n");
  }
}

void delete_dungeon(dungeon_t *d)
{
    free(d->rooms);
    free(d->characters);
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);
  /*
  //character_t c = {.display_char = '*'};
    for (int y = 0; y < DUNGEON_Y; y++) {
        for (int x = 0; x < DUNGEON_X; x++) {
            //d->character_map[y][x] = &c;
        }
    }
    */
}

int save_dungeon(dungeon_t *d, file_info_t *f)
{
  char *home = getenv("HOME");
  char *game_dir = ".rlg327";
  char *save_file = "dungeon";
  sprintf(f->file_type,"RLG327-S2021");
  f->version = htobe32(0);
  f->file_size = 1704+(d->num_rooms*4)+((d->stairs_up+d->stairs_down)*2);

  char *path = malloc(strlen(home) + strlen(game_dir) + strlen(save_file) + 3);
  sprintf(path,"%s/%s/%s", home, game_dir, save_file);
  FILE *file = fopen(path,"w");
  free(path);

  fwrite(f->file_type, 1, 12, file);

   f->version = htobe32(f->version);
  fwrite(&f->version,4,1,file);

  f->file_size = htobe32(f->file_size);
  fwrite(&f->file_size,4,1,file);

  fwrite(&d->characters[0].x,1,1,file);
  fwrite(&d->characters[0].y,1,1,file);

  
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
 int stairs_up2 = d->stairs_up;
 int stairs_down2 = d->stairs_down;
 
  d->stairs_up = htobe16(d->stairs_up);
  fwrite(&d->stairs_up,2,1,file);
  for (int i =0; i <(stairs_up2+stairs_down2);i++)
    {
      //write only the up stairs
      if (d->stairs[i].up_down){

      fwrite(&d->stairs[i].position[dim_x],1,1,file);
      fwrite(&d->stairs[i].position[dim_y],1,1,file);
      }
    }
  
  d->stairs_down = htobe16(d->stairs_down);
  fwrite(&d->stairs_down,2,1,file);
  for (int i =0; i <(stairs_up2+stairs_down2);i++)
    {
      //write only the down stairs
      if (!d->stairs[i].up_down){
      fwrite(&d->stairs[i].position[dim_x],1,1,file);
      fwrite(&d->stairs[i].position[dim_y],1,1,file);
      }
    }
  return 0;
}

void move_pc(dungeon_t *d)
{
    int hasMoved = 0;
    while(!hasMoved)
    {
        printf("made it here2\n");
        int dx = (rand() % 3) - 1;
        int dy = (rand() % 3) - 1;

        if(d->map[d->characters[0].y + dy][d->characters[0].x + dx] != ter_wall &&
            d->map[d->characters[0].y + dy][d->characters[0].x + dx] != ter_wall_immutable)
        {
            printf("made it here3\n");
            // dont know how to check if it exists
            int testx = d->characters[0].x + dx;
            int testy = d->characters[0].y + dy;
            printf("testx: %d   testy: %d\n",testx,testy);
            if (d->character_map[testy][testx]->display_char != '*') {
                printf("made it here4\n");
                character_mapxy(d->characters[0].x + dx,d->characters[0].y + dy)->living = 0;
                printf("made it here5\n");
                printf("living: %d", d->character_map[d->characters[0].y][d->characters[0].x]->living);

            }
            d->character_map[d->characters[0].y + dy][d->characters[0].x + dx] = &d->characters[0];
            d->character_map[d->characters[0].y][d->characters[0].x] = NULL;
            d->characters[0].y += dy;
            d->characters[0].x += dx;

            d->character_map[d->characters[0].y][d->characters[0].x] = &d->characters[0];
            hasMoved = 1;
        }
    }
}

int play_game(dungeon_t *d, file_info_t *f)
{
    heap_t h;
    heap_init(&h,character_cmp,NULL);
    for(int i = 0; i < d->num_monsters+1; i++)
    {
        d->characters[i].hn = heap_insert(&h,&d->characters[i]);
    }
    character_t *c;
    while(d->characters[0].living)
    {
        c = heap_remove_min(&h);
        c->hn = NULL;
        //if the node is a pc
        if (c->sd == 0) {
            //do whatever the pc needs to do
            move_pc(d);
            c->turn = c->turn + 1;
            if (!heap_peek_min(&h))
            {
                break;
            }
            c->hn = heap_insert(&h, c);
            printf("turn: %d\n",c->turn);
            render_dungeon(d,f);
            usleep(2500);
        }
        else if (c->living){
            move_monster(c,d);
            //TODO change turn with speed
            c->turn = c->turn + 1;
            c->hn = heap_insert(&h, c);
        }
        //else{printf("living: %d\n",c->sd); break;}
    }
    if (!d->characters[0].living)
    {
        printf("\nGAME OVER\nYOU LOST");
    }
    else
    {
        printf("\nGAME OVER\nYOU WON");
    }
    return 0;
}

int main(int argc, char *argv[])
{
  dungeon_t d = { .num_monsters = -1};
  file_info_t f = { .load = 0, .save = 0, .ren_non_tun_dist_map = 0, .ren_tun_dist_map = 0};
  struct timeval tv;
  uint32_t seed = 0;

  UNUSED(in_room);

  for (int i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i],"--save"))         {f.save=1;}
      else if (!strcmp(argv[i],"--load"))    {f.load=1;}
      else if (!strcmp(argv[i],"--distmap")) {f.ren_non_tun_dist_map = 1; f.ren_tun_dist_map = 1;}
      else if (!strcmp(argv[i],"--nummon"))  {d.num_monsters = atoi(argv[++i]);}
      else {seed=atoi(argv[i]);}
    }
   if (!seed)
  {
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }
  
  srand(seed);

  init_dungeon(&d);
  if (f.load){
    load_dungeon(&d, &f);
  }
  else {
    gen_dungeon(&d);
  }
  render_dungeon(&d,&f);
  if (f.save) {
    save_dungeon(&d,&f);
  }
  play_game(&d,&f);
  delete_dungeon(&d);

  return 0;
}

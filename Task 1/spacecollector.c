/*
 * Implement the missing functionality in this file. DO NOT CHANGE ANY OTHER FILE!
 * Add your code only inside the given TODO borders
 */

#include "spacecollector.h"
#include <stdlib.h>

WINDOW *game_window;
char game_map[MAP_LENGTH][MAP_WIDTH];

pthread_t player_tid;
pthread_t enemy_alien_tid[NUMBER_ALIENS];
pthread_t enemy_blackhole_tid[NUMBER_BLACKHOLES];
pthread_t crate_tid;

time_t time1;

position player_position = {MAP_WIDTH / 2, MAP_LENGTH - 2};
position crate_position = {-1, -1};

int lifepoints = 100;
int points = 0;

int crate_collected_flag = 0;

bool autoplay_enabled = false;

void *playerLogic()
{
  game_map[player_position.y_][player_position.x_] = (char)PLAYER;

  char direction = 'l';
  char prev_direction = 'l';

  int c;
  keypad(stdscr, TRUE);
  noecho();
  timeout(1);
  while (true)
  {
    if (!autoplay_enabled)
    {
      c = getch();
      switch (c)
      {
      case 's':
        direction = 'd';
        break;
      case 'w':
        direction = 'u';
        break;
      case 'a':
        direction = 'l';
        break;
      case 'd':
        direction = 'r';
        break;
      default:
        break;
      }
      if (c == 'q')
      {
        lifepoints = 0;
        continue;
      }
    }
    else
    {
      // TODO (11) (BONUS):
      // Implement the logic for the autoplay mode
      // The player should collect crates automatically
      // It is sufficient enough that you don't have to evade enemies. 
      // You can *temporarily* test this behaviour by setting NUMBER_ALIENS and NUMBER_BLACKHOLES to 0. 
      // Make sure to not commit any changes to the header files!
      if(player_position.x_ > crate_position.x_)
        direction = 'l';
      else if(player_position.x_ < crate_position.x_)
        direction = 'r';
      else if(player_position.y_ < crate_position.y_)
        direction = 'd';
      else if(player_position.y_ > crate_position.y_)
        direction = 'u';

      // TODO END
    }
    usleep(100000);

    // TODO (7):
    // - If the player is on a supply crate, collect it. (Hint: getMapValue, SUPPLY_CRATE)
    // - If the crate was collected, the points should be increased and the crate should disappear. (the crate thread should somehow terminate)
    // - also set crate_collected_flag to 1
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (getMapValue(player_position.x_, player_position.y_) == (char)SUPPLY_CRATE)
    {
      points += POINTS_CRATE;
      pthread_cancel(crate_tid);
      crate_collected_flag = 1;
    }
    // TODO END

    movePlayer(&direction, &prev_direction, &player_position, PLAYER);
  }
}

void *enemyBlackhole(parameters *params)
{
  if (params->type_ != ENEMY_BLACKHOLE)
    return (void *)-1;

  // TODO (3):
  // Spawn the black hole correctly in the map. The given position gives the position of the top left field.
  // Make sure to initialize the other fields correctly!
  // spawnEnemy *only* initializes *one* field on the map.
  position enemy_pos = {params->pos_x_, params->pos_y_};
  free(params);
  while (lifepoints > 0)
  {
    usleep(300000);
    spawnEnemy(ENEMY_BLACKHOLE, enemy_pos.x_, enemy_pos.y_);
    spawnEnemy(ENEMY_BLACKHOLE, enemy_pos.x_ +1, enemy_pos.y_);
    spawnEnemy(ENEMY_BLACKHOLE, enemy_pos.x_, enemy_pos.y_ + 1);
    spawnEnemy(ENEMY_BLACKHOLE, enemy_pos.x_ + 1, enemy_pos.y_ + 1);
  }

  // TODO END

  return (void *)999;
}

void *enemyAlien(parameters *params)
{
  if (params->type_ != ENEMY_ALIEN)
    return (void *)-1;

  // TODO (4):
  // -) set the alien position correctly
  // -) spawn the enemy alien correctly in the map

  position enemy_pos = {params->pos_x_, params->pos_y_};
  spawnEnemy(ENEMY_ALIEN, enemy_pos.x_, enemy_pos.y_);
  free(params);

  // TODO END

  int nr_direction = 0;
  char prev_direction = 'l';

  while (lifepoints > 0)
  {
    usleep(300000);

    if (rand() % 5 == 0)
    {
      nr_direction = rand() % 4;
    }
    char direction;
    switch (nr_direction)
    {
    case 0:
      direction = 'l';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)ENEMY_ALIEN);
      break;
    case 1:
      direction = 'r';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)ENEMY_ALIEN);
      break;
    case 2:
      direction = 'u';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)ENEMY_ALIEN);
      break;
    case 3:
      direction = 'd';
      movePlayer(&direction, &prev_direction, &enemy_pos, (char)ENEMY_ALIEN);
      break;
    default:
      break;
    }
  }
  return (void *)999;
}

void init_enemies(unsigned char type, int number_of_enemy_type)
{
  pthread_attr_t enemy_attr;
  pthread_attr_init(&enemy_attr);

  // TODO (2): Implement the whole function:
  // - spawn all enemies of the given type (one thread per entity), make sure each one appears one it's random coordinates and has the type specified in type_
  // - use the attribute for thread creation
  // - use the enemy_alien_tid and enemy_blackhole_tid arrays to store the thread IDs in them
  // - all enemies have to move on their own and concurrently with the others!
  // - look at the provided data structures in the header. Use them!
  // - make sure to get a new random position for each enemy entity!
  // - Arguments needed? generate them via getRandPosX() and getRandPosY()
  // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
  // - Furthermore, make sure to keep the lifetime of variables in mind and to not leak any memory!

  pthread_attr_setdetachstate(&enemy_attr, PTHREAD_CREATE_JOINABLE);
  parameters *params[number_of_enemy_type];
  int i;
  for (i = 0; i < number_of_enemy_type; i++)
  {
    params[i] = malloc(sizeof(parameters));
    params[i]->pos_x_ = getRandPosX();
    params[i]->pos_y_ = getRandPosY();
    params[i]->type_ = type;
    if (type == ENEMY_ALIEN)
    {
      pthread_create(&enemy_alien_tid[i], &enemy_attr, (void *(*)(void *)) &enemyAlien, (void *)params[i]);
    }
    else if (type == ENEMY_BLACKHOLE)
    {
      pthread_create(&enemy_blackhole_tid[i], &enemy_attr, (void *(*)(void *)) &enemyBlackhole, (void *)params[i]);
    }
  }
  // TODO END

  pthread_attr_destroy(&enemy_attr);
}

void *placeCrate(parameters *params)
{
  if (params->type_ != SUPPLY_CRATE)
    return (void *)-1;

  // TODO (6):
  // - In general there should be only one crate on the map at a time
  // - place the crate on the position taken from the function parameters
  // - the crate has to disappear immediately if it gets collected. Think about what mechanism/characteristic which could be used for this.

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  unsigned char position_x = params->pos_x_;
  unsigned char position_y = params->pos_y_;
  crate_position.x_ = position_x;
  crate_position.y_ = position_y;
  free(params);

  // TODO END

  while (1)
  {
    spawnCrate(position_x, position_y);

    // Bonus helper
    crate_position.x_ = position_x;
    crate_position.y_ = position_y;
  }
}

int main(int argc, char* argv[])
{
  void *rvalue_player = 0;
  void *rvalue_enemies[NUMBER_BLACKHOLES + NUMBER_ALIENS];
  void *rvalue_crates = 0;

  init_map();
  srand((unsigned int)time(&time1));

  // TODO (10): If you want to do the bonus, simply check if the program has been started with the argument "bonus" and if so, set the autoplay variable
  if(argc == 1)
    {

    }
  else if(argc == 2)
  {
    if (!strcmp(argv[1], "bonus"))
      autoplay_enabled = true;
  }
  // TODO END

  // TODO (1):
  // - spawn the player and enemies by creating separate threads for each entity
  // - Arguments needed? Attributes needed? Look at the playerLogic and the enemy functions
  // - look at the pthread_t variables from above... USE them!!
  // - Init all aliens with one call to init_enemies and all blackholes with another call to init_enemies
  // - Generate NUMBER_BLACKHOLES blackholes and NUMBER_ALIENS aliens enemies
  // - Make sure to generate all aliens first, then all blackholes
  pthread_create(&player_tid, 0, (void *(*)(void *)) &playerLogic, 0);

  init_enemies(ENEMY_ALIEN, NUMBER_ALIENS);
  init_enemies(ENEMY_BLACKHOLE, NUMBER_BLACKHOLES);
  // TODO END

  refreshMap();

  // TODO (5):
  // - spawn an initial supply crate by creating a supply crate thread (for position call getRandPosX() and getRandPosY())
  // - if a supply crate is collected (there's a flag for that), a new supply crate shall be spawned at a new random position
  // - reset the collected flag ONLY AFTER you spawned a new supply crate
  // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
  // - make sure that all supply crate threads are terminated and store the return value in rvalue_crates - Do this check for termination in this TODO
  // - have a look at the other TODOs from above - there might be interconnects
  // - Arguments needed? generate them via getRandPosX() and getRandPosY()
  parameters *params;
  params = malloc(sizeof(parameters));
  params->pos_x_ = getRandPosX();
  params->pos_y_ = getRandPosY();
  params->type_ = SUPPLY_CRATE;
  pthread_create(&crate_tid, 0, (void *(*)(void *)) &placeCrate, (void *)params);

  // TODO END

  while (lifepoints > 0)
  {
    usleep(10000);

    if (crate_collected_flag == 1)
    {
      // TODO (8): Further implementations from TODO above
      params = malloc(sizeof(parameters));
      params->pos_x_ = getRandPosX();
      params->pos_y_ = getRandPosY();
      params->type_ = SUPPLY_CRATE;
      pthread_create(&crate_tid, 0, (void *(*)(void *)) &placeCrate, (void *)params);
      crate_collected_flag = 0;
      // TODO END
    }

    refreshMap();
  }

  // TODO (9):
  // - make sure that all the running threads are terminated before returning from main and fetch their return values
  // - All threads which are able to return on their own should also do that so and therefore give back the corresponding return value
  // - Force the termination of threads which do not terminate on their own.
  // - have a closer look on the other TODOs. There might be some connections
  // - we want to make sure, that all threads are terminated for sure. But are all threads even joinable?
  //   Maybe the tutor made a stupid mistake somewhere in the upstream code. Fix it.
  for (int i = 0; i < NUMBER_ALIENS; i++)
  {
    pthread_join(enemy_alien_tid[i], &rvalue_enemies[i]);
  }
  for (int i = 0; i < NUMBER_BLACKHOLES; i++)
  {
    pthread_join(enemy_blackhole_tid[i], &rvalue_enemies[i + NUMBER_ALIENS]);
  }

  pthread_cancel(player_tid);
  rvalue_player = PTHREAD_CANCELED;
  pthread_cancel(crate_tid);
  rvalue_crates = PTHREAD_CANCELED;
  // TODO END

  return end_game((void **)rvalue_player, (void *)rvalue_enemies, (void *)rvalue_crates);
}

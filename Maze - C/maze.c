#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/** CONTROLS **/
#define UP 'w'
#define LEFT 'a'
#define RIGHT 'd'
#define DOWN 's'

/** GAME STATES **/
#define EXIT 1
#define LOST 2
#define WON 3
#define IGNORE 4
#define SAVE 5
#define LOAD 6
#define HELP 7

/** Game Variables **/
#define PLAYER '@'
#define MONSTER 'M'
#define TREASURE '*'

/**Question 1**/
char get(unsigned x, unsigned y);
void put(unsigned x, unsigned y, char v);
void print_map();
void populate_map();
void initialize_objects();
/**Question 2**/
struct game_object;
struct list_node;

void put_object(struct game_object *obj);
void add_monster(struct list_node **list, unsigned x, unsigned y);
void free_list(struct list_node *iter);
/**Question 3**/
char move_object(struct game_object *obj, char direction);
int update_game();
/**Question 4**/
void save_map(const char *file_name);
void load_map(const char *file_name);
void parse_map();
/**Unbuffered Input **/
void tty_reset();
int tty_raw();
void clear_sreen();

struct game_object {
  unsigned x;
  unsigned y;
  char type;
};

struct list_node {
  struct list_node *next;
  struct game_object *game;
};

unsigned ROWS = 5;
unsigned COLS = 12;
struct list_node *monsters;
struct game_object *player;
char *map;

char get(unsigned x, unsigned y) { return *(map + (y * COLS) + x); }
void put(unsigned x, unsigned y, char v) { *(map + (y * COLS) + x) = v; }

void print_map() {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      putchar(get(col, row));
    }
    putchar('\n');
  }
}

void populate_map() {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      if (row == 0 || row == ROWS - 1 || col == 0 || col == COLS - 1) {
        put(col, row, '#');
      } else {
        put(col, row, ' ');
      }
    }
  }
}

void initialize_objects() {
  add_monster(&monsters, 2, 3);
  add_monster(&monsters, 4, 2);
  put_object(monsters->game);
  put_object(monsters->next->game);
  put_object(player);
  put(1, 1, TREASURE);
}

void put_object(struct game_object *obj) { put(obj->x, obj->y, obj->type); }

void add_monster(struct list_node **list, unsigned x, unsigned y) {
  struct list_node *temp = (struct list_node *)malloc(sizeof(struct list_node));
  temp->next = (*list);
  temp->game = (struct game_object *)malloc(sizeof(struct game_object));
  temp->game->type = MONSTER;
  temp->game->x = x;
  temp->game->y = y;
  *list = temp;
}

void free_list(struct list_node *iter) {
  struct list_node *temp = iter;
  while (temp != NULL) {
    struct list_node *tempTwo = temp;
    temp = temp->next;
    free(tempTwo->game);
    free(tempTwo);
  }
  monsters = NULL;
}

char move_object(struct game_object *obj, char direction) {
  int x = obj->x;
  int y = obj->y;
  char current = get(x, y);

  if (direction == UP) {
    y--;
  } else if (direction == LEFT) {
    x--;
  } else if (direction == DOWN) {
    y++;
  } else if (direction == RIGHT) {
    x++;
  }

  if (get(x, y) == '#') {
    return current;
  } else {
    put(obj->x, obj->y, ' ');
    obj->y = y;
    obj->x = x;
    current = get(x, y);
    put_object(obj);
    return current;
  }
  return current;
}

int update_game() {
  char input;
  printf("> ");
  scanf(" %c", &input);

  char playerMove = get(player->x, player->y);

  if (input == UP) {
    playerMove = move_object(player, UP);
  } else if (input == LEFT) {
    playerMove = move_object(player, LEFT);
  } else if (input == DOWN) {
    playerMove = move_object(player, DOWN);
  } else if (input == RIGHT) {
    playerMove = move_object(player, RIGHT);
  } else if (input == 'x') {
    return EXIT;
  } else if (input == 'l') {
    return LOAD;
  } else if (input == 'k') {
    return SAVE;
  } else if (input == '\n') {
    return IGNORE;
  } else if (input == 'h') {
    return HELP;
  }

  // Checks for collisions and checks if plays eats treasure
  // or if player collides with monster and vice versa.
  if (input != SAVE || input != LOAD || input != 'x') {
    struct list_node *temp = monsters;
    while (temp != NULL) {
      if (playerMove == TREASURE) {
        return WON;
      }
      if (playerMove == MONSTER) {
        return LOST;
      }
      char value[4] = {UP, DOWN, LEFT, RIGHT};
      int randomValue = rand() % 4;
      char monsterMove = move_object(temp->game, value[randomValue]);
      if (monsterMove == PLAYER) {
        return LOST;
      }
      temp = temp->next;
    }
  }

  return 0;
}

/** Save current game state **/
void save_map(const char *file_name) {
  FILE *file = fopen(file_name, "w");
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      putc(get(col, row), file);
    }
    putc('\n', file);
  }
  fclose(file);
}

/** Gets the new rows and columns for the new map and
allocates enough memory for the map to store the maze*/
void setRowsCols(const char *file_name) {
  free(map);
  FILE *file = fopen(file_name, "r");
  char mychar;
  int tempRows = 0;
  int counter = 0;
  while ((mychar = fgetc(file)) != EOF) {
    if (mychar == '\n') {
      tempRows++;
    } else {
      counter++;
    }
  }
  COLS = (counter / tempRows);
  ROWS = tempRows;
  fclose(file);
  map = (char *)malloc(sizeof *map * ROWS * COLS);
}

/** Calls setRowsCols method to set the rows and columns
then read from the file again and insert into the map*/
void load_map(const char *file_name) {
  setRowsCols(file_name);
  FILE *file = fopen(file_name, "r");
  int row = 0;
  int col = 0;
  char mychar;

  for (row = 0; row < ROWS; row++) {
    for (col = 0; col < COLS; col++) {
      mychar = getc(file);
      put(col, row, mychar);
    }
    mychar = getc(file);
  }
  fclose(file);
  parse_map();
}

/** Places monsters and player into the custom map **/
void parse_map() {
  free_list(monsters);
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      if (get(col, row) == PLAYER) {
        player->x = col;
        player->y = row;
      } else if (get(col, row) == MONSTER) {
        add_monster(&monsters, col, row);
      }
    }
  }
}

void clear_screen() { system("clear"); }

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  map = (char *)malloc(sizeof *map * ROWS * COLS);
  player = (struct game_object *)malloc(sizeof(struct game_object));
  monsters = NULL;

  player->x = 5;
  player->y = 3;
  player->type = PLAYER;
  if (argc < 2) {
    populate_map();
    initialize_objects();
  } else {
    int result = access(argv[1], F_OK); // Tests for existence of file
    if (result != 0) {
      printf("File doesn't exit\n");
      exit(0);
    } else {
      load_map(argv[1]);
    }
  }
  clear_screen();
  print_map(); // Print initial Map

  int isPlaying = 1;
  int game_value = 0;
  while (isPlaying) {
    game_value = update_game();
    clear_screen();
    printf("\n");
    print_map(); // Re-print map after any input.

    if (game_value == EXIT) {
      printf("Thanks for Playing! Goodbye!\n");
      isPlaying = 0;
    } else if (game_value == WON) {
      printf("You Won! You captured the treasure chest!\n");
      isPlaying = 0;
    } else if (game_value == LOST) {
      printf("You Lost! The monster ate you!\n");
      isPlaying = 0;
    } else if (game_value == SAVE) {
      printf("Progress Saved!\n");
      save_map("save_game.txt");
    } else if (game_value == LOAD) {
      printf("Game loaded!\n");
      load_map("save_game.txt");
      clear_screen(); // Clear screen so that previous state is not shown
      print_map();    // reprint map once after game has been loaded.
    } else if (game_value == HELP) {
      printf("Welcome to Maze\n");
      printf("Goal: Grab the treasure (*) before the Monsters do!!\n");
      printf("Controls: \nUp: 'w'\nDown: 's'\nLeft: 'a'\nRight:'d'\n\nLoad "
             "Map: 'l'\nSave Progress: 'k'\nExit: 'x'\n");
    }
  }
  free_list(monsters);
  free(player);
  free(map);
}

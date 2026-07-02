#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R3, -1, A5, A4);

#define BTN_RIGHT 10
#define BTN_DOWN 9
#define BTN_LEFT 7
#define BTN_UP 6

//=== sprites ===
const unsigned char pacman_closemouth_down_and_right_left_bits[] PROGMEM = {0x06, 0x0B, 0x0F, 0x06};
const unsigned char pacman_closemouth_up_bits[] PROGMEM = {0x06, 0x0F, 0x0B, 0x06};

const unsigned char pacman_openmouth_right_bits[] PROGMEM = {0x0E, 0x05, 0x07, 0x0E};
const unsigned char pacman_openmouth_down_bits[] PROGMEM = {0x06, 0x0D, 0x0F, 0x09};
const unsigned char pacman_openmouth_left_bits[] PROGMEM = {0x07, 0x0A, 0x0E, 0x07};
const unsigned char pacman_openmouth_up_bits[] PROGMEM = {0x09, 0x0F, 0x0B, 0x06};

const unsigned char ghost_bits[] PROGMEM = {0x0E, 0x15, 0x1F, 0x1F};


// ==== Direction Constants ====
#define DIR_NONE 0
#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_DOWN 3
#define DIR_LEFT 4

// ===== MAP CONFIGURATION =====
#define E 0  // Empty Space
#define W 1  // Wall
#define P 2  // Pellet (Food)
#define B 3  // Big Power Pellet

#define map_grid 6

uint8_t original_map[19][10] = {
 // 0  1  2  3  4  5  6  7  8  9
  { W, W, W, W, W, W, W, W, W, W },  // 0 
  { W, P, P, P, P, P, P, P, P, W },  // 1
  { W, B, W, W, P, P, W, W, B, W },  // 2 
  { W, P, P, P, P, P, P, P, P, W },  // 3
  { W, W, W, P, W, W, P, W, W, W },  // 4
  { E, E, W, P, W, W, P, W, E, E },  // 5 
  { W, W, W, P, W, W, P, W, W, W },  // 6
  { W, P, P, P, P, P, P, P, P, W },  // 7
  { W, P, W, W, W, W, W, W, P, W },  // 8 
  { W, P, W, E, E, E, E, W, P, W },  // 9 
  { W, P, W, W, W, W, W, W, P, W },  // 10
  { W, P, P, P, P, P, P, P, P, W },  // 11
  { W, W, W, P, W, W, P, W, W, W },  // 12
  { W, B, P, P, W, W, P, P, B, W },  // 13
  { W, P, W, W, W, W, W, W, P, W },  // 14
  { W, P, P, P, P, P, P, P, P, W },  // 15
  { W, W, W, W, P, P, W, W, W, W },  // 16
  { W, P, P, P, P, P, P, P, P, W },  // 17
  { W, W, W, W, W, W, W, W, W, W }   // 18
};

uint8_t game_map[19][10];

//==== Game State ====
bool game_over = false;
bool is_pacman_moving = false;
int score = 0;

//===== Animation configuration ====== 
#define MOVE_DELAY 100

unsigned long pacman_last_move_time = 0 ;
bool pacman_mouth_open = true;


//==== Game objects ====
struct Pacman {
  int x; 
  int y; 
  uint8_t lives;
  uint8_t current_direction;
  uint8_t next_direction;
};

struct Ghost {
  int x;
  int y;
  uint8_t current_direction;
  bool frightened;
  bool in_house;
  bool is_eaten;
  unsigned long frightened_end;
  unsigned long exit_time;
};

Pacman pacman;
Ghost ghost[4];

// Forward declarations
void setup_game(bool on_gameover);
void handle_controls();
void draw_gameover();
void move_pacman();
void draw_game();

void setup() {
  u8g2.begin();
  randomSeed(analogRead(0));

  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);

  setup_game(true);
}

void loop() {
  if (!game_over) {
    handle_controls();
    move_pacman();

    draw_game();

  }else {
    draw_gameover();
    
    if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_RIGHT) == LOW || digitalRead(BTN_DOWN) == LOW || digitalRead(BTN_LEFT) == LOW) { 
      setup_game(true);
    }
  }
}


void setup_game(bool on_gameover){
  if(on_gameover){
    game_over=false;
    score = 0;
    pacman.lives=3;

    //reset  map
    memcpy(game_map, original_map, sizeof(game_map));
  }


  is_pacman_moving= false;

  //init pacman
  pacman.x = 4;
  pacman.y = 15;
  pacman.current_direction=DIR_NONE;
  pacman.next_direction=DIR_NONE;
  pacman_last_move_time=0;
  
  //init ghosts
  //Also some time for animation

}

void handle_controls(){
  if (digitalRead(BTN_UP) == LOW && pacman.current_direction != DIR_DOWN) pacman.next_direction = DIR_UP;
  if (digitalRead(BTN_RIGHT) == LOW && pacman.current_direction != DIR_LEFT) pacman.next_direction = DIR_RIGHT;
  if (digitalRead(BTN_DOWN) == LOW && pacman.current_direction != DIR_UP) pacman.next_direction = DIR_DOWN;
  if (digitalRead(BTN_LEFT) == LOW && pacman.current_direction != DIR_RIGHT) pacman.next_direction = DIR_LEFT;
}

bool is_walkable(int col, int row){
  if (col < 0 || col >= 10 || row < 0 || row >= 19) return false;

  uint8_t tile = game_map[row][col];
  return (tile == E || tile == P || tile == B);
}

void frighten_ghosts(){

}

void chase_pacman(){

}

void move_pacman() {
  unsigned long now = millis();
  if (now - pacman_last_move_time < MOVE_DELAY) return;
  pacman_last_move_time = now;

  int next_x = pacman.x;
  int next_y = pacman.y;

  switch (pacman.next_direction) {
    case DIR_UP:    next_y--; break;
    case DIR_RIGHT: next_x++; break;
    case DIR_DOWN:  next_y++; break;
    case DIR_LEFT:  next_x--; break;
  }

  bool moved = false;

  if (pacman.next_direction != DIR_NONE && is_walkable(next_x, next_y)) {
    pacman.current_direction = pacman.next_direction;
    pacman.x = next_x;
    pacman.y = next_y;
    moved = true;
  } else {
    next_x = pacman.x;
    next_y = pacman.y;
    switch (pacman.current_direction) {
      case DIR_UP:    next_y--; break;
      case DIR_RIGHT: next_x++; break;
      case DIR_DOWN:  next_y++; break;
      case DIR_LEFT:  next_x--; break;
    }

    if (pacman.current_direction != DIR_NONE && is_walkable(next_x, next_y)) {
      pacman.x = next_x;
      pacman.y = next_y;
      moved = true;
    }
  }

  if (moved) {
    pacman_mouth_open = !pacman_mouth_open;

    switch (game_map[pacman.y][pacman.x]) {
      case P: // Food
        game_map[pacman.y][pacman.x] = E;
        score += 10;
        break;
      case B: // Power Pellet
        game_map[pacman.y][pacman.x] = E;
        score += 50;
        frighten_ghosts();
        break;
    }
  }
}

//========= RENDERING ===========
void draw_map(){
  for(int row=0; row <19; row++){
    for(int col=0; col<10; col++){
      //offset score
      int x = 2 + (col * map_grid);
      int y = 14 + (row * map_grid);
      
      uint8_t tile = game_map[row][col];

      switch (tile) {
        case W:
          u8g2.drawBox(x, y, map_grid, map_grid);
        break;
        case P:
          u8g2.drawBox(x + 2, y + 2, 2, 2);
        break;
        case B:
          u8g2.drawBox(x + 1, y + 1, 4, 4);
        break;
      }
    }
  }
}

void draw_ghost(){
  //Fill this later
}

void draw_pacman(){

  //calculating precise pacman position
  int pacman_x = 2 + (pacman.x * map_grid);
  int pacman_y = 14 + (pacman.y * map_grid);

  const unsigned char* selected_sprite;

  if (pacman_mouth_open) {
    switch (pacman.current_direction) {
      case DIR_UP: 
        selected_sprite = pacman_openmouth_up_bits; 
      break;
      case DIR_DOWN: 
        selected_sprite = pacman_openmouth_down_bits; 
      break;
      case DIR_RIGHT: 
        selected_sprite = pacman_openmouth_right_bits; 
      break;
      case DIR_LEFT: 
        selected_sprite = pacman_openmouth_left_bits; 
      break;
      default: 
        selected_sprite = pacman_openmouth_right_bits;
      break;
    }
  } else {
    switch (pacman.current_direction) {
      case DIR_UP: 
        selected_sprite = pacman_closemouth_up_bits; 
      break;
      case DIR_DOWN:
      case DIR_RIGHT:
      case DIR_LEFT: 
        selected_sprite = pacman_closemouth_down_and_right_left_bits; 
      break;
      default: 
        selected_sprite = pacman_closemouth_down_and_right_left_bits;
      break;
    }
  }

  u8g2.drawXBMP(pacman_x + 1, pacman_y + 1, 4, 4, selected_sprite);
}

void draw_game() {
  u8g2.firstPage();
  do {
    //Draw score panel
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(2, 9);
    u8g2.print("Score:");
    u8g2.setCursor(32, 9);
    u8g2.print(score);
    
    //horizenal
    u8g2.drawHLine(0, 12, 64);

    
    draw_pacman();
    draw_map();

  } while (u8g2.nextPage());
}

void draw_pacman_life_page(){
  u8g2.firstPage();
  do {

  } while (u8g2.nextPage());
}

void draw_gameover(){ 

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(10, 30, "GAME OVER");
    u8g2.setCursor(15, 45);
    u8g2.print("SCORE: ");
    u8g2.print(score);
  } while (u8g2.nextPage());
}
#include <U8g2lib.h>
#include <limits.h>

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

const unsigned char ghost_frame_1_bits[] PROGMEM = {0x0E, 0x15, 0x1F, 0x15,};
const unsigned char ghost_frame_2_bits[] PROGMEM = {0x0E, 0x15, 0x1F, 0x0A,};

const unsigned char ghost_frightened_frame_1_bits[] PROGMEM = {0x0E, 0x11, 0x11, 0x1F, };
const unsigned char ghost_frightened_frame_2_bits[] PROGMEM = {0x0E, 0x1F, 0x1F, 0x1F, };
const unsigned char ghost_eyes_bits[] PROGMEM = {0x0A, 0x15, 0x1F, 0x0A, };

const unsigned char heart_bits[] PROGMEM = {0x0A, 0x1F, 0x1F, 0x0E, 0x04,}; //5X5

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
int total_pellets = 76;
uint8_t ghost_mode = 1;//start in scatter mode
uint8_t ghosts_eaten_combo = 0;
unsigned long ghost_mode_start = 0;
unsigned long frightened_start  = 0;

bool reverse = false;
int score = 0;

// timers for ghost modes
const unsigned long SCATTER_DURATION   = 7000;
const unsigned long CHASE_DURATION     = 20000;
const unsigned long FRIGHTENED_DURATION = 8000;

//===== Animation configuration ====== 
#define MOVE_DELAY 100
#define GHOST_ANIM_DELAY 200

unsigned long pacman_last_move_time = 0 ;
bool pacman_mouth_open = true;

unsigned long ghost_last_anim_time = 0;
bool ghost_anim_frame = false;

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
  uint8_t direction;
  bool in_house;
  bool is_eaten;
  unsigned long exit_time;
};

Pacman pacman;
Ghost ghosts[4];

// Forward declarations
void setup_game(bool on_gameover);
void handle_controls();
void draw_gameover();
void move_pacman();
void move_ghosts();
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
    //handle button press
    handle_controls();

    //move objects
    move_pacman();
    move_ghosts();

    //render the game scene
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
  // Blinky — outside the house already
  ghosts[0].x = 4;
  ghosts[0].y = 7;
  ghosts[0].direction = DIR_LEFT;
  ghosts[0].in_house = false;
  ghosts[0].is_eaten = false;
  ghosts[0].exit_time = 0;

  // Pinky — center left of house
  ghosts[1].x = 3;
  ghosts[1].y = 9;
  ghosts[1].direction = DIR_UP;
  ghosts[1].in_house = true;
  ghosts[1].is_eaten = false;
  ghosts[1].exit_time = millis() + 3000;  // exits after 3s

  // Inky — center of house
  ghosts[2].x = 4;
  ghosts[2].y = 9;
  ghosts[2].direction = DIR_UP;
  ghosts[2].in_house = true;
  ghosts[2].is_eaten = false;
  ghosts[2].exit_time = millis() + 6000;  // exits after 6s

  // Clyde — center right of house
  ghosts[3].x = 5;
  ghosts[3].y = 9;
  ghosts[3].direction = DIR_UP;
  ghosts[3].in_house = true;
  ghosts[3].is_eaten = false;
  ghosts[3].exit_time = millis() + 9000;  // exits after 9s

}

void handle_controls(){
  if (digitalRead(BTN_UP) == LOW && pacman.current_direction != DIR_DOWN) pacman.next_direction = DIR_UP;
  if (digitalRead(BTN_RIGHT) == LOW && pacman.current_direction != DIR_LEFT) pacman.next_direction = DIR_RIGHT;
  if (digitalRead(BTN_DOWN) == LOW && pacman.current_direction != DIR_UP) pacman.next_direction = DIR_DOWN;
  if (digitalRead(BTN_LEFT) == LOW && pacman.current_direction != DIR_RIGHT) pacman.next_direction = DIR_LEFT;
}


void check_collisions(Ghost* ghost){
  if(ghost->x== pacman.x && ghost->y==pacman.y){
    if(ghost_mode==2){//It is in frightened mode..so pacman can eat that 
      ghost->is_eaten = true;
      ghosts_eaten_combo++;
      
      // 200 → 400 → 800 → 1600 per consecutive ghost
      score += 100 * (1 << ghosts_eaten_combo);
    }else{//ghost is chase or scatter mode...so..ghost kills Pac-Man
      pacman.lives--;
      
      if(pacman.lives==0){
        game_over=true;
        return;
      }

      //reset positions without resetting score or map
      setup_game(false);
      return;
    }
  }
}

//========MOVEMENT and AI========

bool is_walkable(int col, int row){
  if (col < 0 || col >= 10 || row < 0 || row >= 19) return false;

  uint8_t tile = game_map[row][col];
  return (tile == E || tile == P || tile == B);
}

int calculate_educian_distance(int delta_x , int delta_y){
  return (delta_x*delta_x)+(delta_y*delta_y);
}

uint8_t get_best_direction(Ghost* ghost,int target_x,int target_y) {
  int ghost_x = ghost->x;
  int ghost_y = ghost->y;
  
  uint8_t best_direction = DIR_NONE; 
  int best_distance = INT_MAX;

  int delta_x = ghost_x - target_x;
  int delta_y = ghost_y - target_y;

  for (int i = 0; i < 4; i++) {
    switch (i) {
      case 0: { // up
        if (ghost->direction == DIR_DOWN) continue;

        int new_y = ghost_y - 1;
        if (!is_walkable(ghost_x, new_y)) continue;

        int new_delta_y = new_y - target_y;
        int new_distance = calculate_educian_distance(delta_x, new_delta_y);
        
        if (new_distance < best_distance) {
          best_distance = new_distance;
          best_direction = DIR_UP;
        }
      break;
      }
      case 1: { // left
        if (ghost->direction == DIR_RIGHT) continue;

        int new_x = ghost_x - 1;
        if (!is_walkable(new_x, ghost_y)) continue;

        int new_delta_x = new_x - target_x;
        int new_distance = calculate_educian_distance(new_delta_x, delta_y);

        if (new_distance < best_distance) {
          best_distance = new_distance;
          best_direction = DIR_LEFT;
        }
      break;
      }
      case 2: { // down
        if (ghost->direction == DIR_UP) continue;

        int new_y = ghost_y + 1;
        if (!is_walkable(ghost_x, new_y)) continue;

        int new_delta_y = new_y - target_y;
        int new_distance = calculate_educian_distance(delta_x, new_delta_y);

        if (new_distance < best_distance) {
          best_distance = new_distance;
          best_direction = DIR_DOWN;
        }
      break;
      }
      case 3: { // right
        if (ghost->direction == DIR_LEFT) continue;

        int new_x = ghost_x + 1;
        if (!is_walkable(new_x, ghost_y)) continue;

        int new_delta_x = new_x - target_x;
        int new_distance = calculate_educian_distance(new_delta_x, delta_y);

        if (new_distance < best_distance) {
          best_distance = new_distance;
          best_direction = DIR_RIGHT;
        }
      break;
      }
    }
  }

  return best_direction;
}

void reverce_direction(Ghost* ghost) {
  switch (ghost->direction) {
    case DIR_UP:    
      ghost->direction = DIR_DOWN;  
    break;
    case DIR_LEFT:  
      ghost->direction = DIR_RIGHT; 
    break;
    case DIR_DOWN:  
      ghost->direction = DIR_UP;    
    break; 
    case DIR_RIGHT: 
      ghost->direction = DIR_LEFT;  
    break;
  }
}

void pick_random_direction(Ghost* ghost){
  uint8_t directions[4] = {DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT};

  int ghost_x = ghost->x; //col
  int ghost_y = ghost->y; //row 
  
  for (int i = 3; i >= 0; i--) {
    // Fisher-Yates shuffle
    int j = random(0, i + 1);         
    uint8_t temp = directions[i];
    directions[i] = directions[j];
    directions[j] = temp;

    uint8_t chosen_dir = directions[i];
    int new_x = ghost_x;
    int new_y = ghost_y;

    // Calculate the new coordinates based on the chosen direction
    switch(chosen_dir){
      case DIR_UP:{
        if(ghost->direction == DIR_DOWN) continue;
        new_y--; 
      break;
      }    
      case DIR_DOWN:{
        if(ghost->direction == DIR_UP) continue;
        new_y++; 
      break;
      }
      case DIR_LEFT:{  
        if(ghost->direction == DIR_RIGHT) continue;
        new_x--; 
      break;
      }
      case DIR_RIGHT: {
        if(ghost->direction == DIR_LEFT) continue;
        new_x++;

      break;
      }
    }

    // Check if the new position is valid
    if(is_walkable(new_x, new_y)) {
      ghost->x = new_x;
      ghost->y = new_y;
      ghost->direction = chosen_dir;
      
      return; 
    }
  }
}

void chase_pacman(Ghost* ghost, int ghost_index){
  int target_x, target_y;

  switch (ghost_index) {
    case 0:{//blinky
      target_x = pacman.x;
      target_y = pacman.y;
    break;
    }
    case 1:{//pinky
      target_x = pacman.x;
      target_y = pacman.y;
      
      switch (pacman.current_direction) {
        case DIR_UP:    target_y -= 4; break;
        case DIR_DOWN:  target_y += 4; break;
        case DIR_LEFT:  target_x -= 4; break;
        case DIR_RIGHT: target_x += 4; break;
      }
    break;
    }
    case 2:{//inky
      int pivot_x = pacman.x;
      int pivot_y = pacman.y;
      
      switch (pacman.current_direction) {
        case DIR_UP:    pivot_y -= 2; break;
        case DIR_DOWN:  pivot_y += 2; break;
        case DIR_LEFT:  pivot_x -= 2; break;
        case DIR_RIGHT: pivot_x += 2; break;
      }

      // double the vector from Blinky to pivot
      target_x = pivot_x + (pivot_x - ghosts[0].x);
      target_y = pivot_y + (pivot_y - ghosts[0].y);
    break;
    }
    case 3:{//clyde
      int dx = ghosts[3].x - pacman.x;
      int dy = ghosts[3].y - pacman.y;
      
      if ((dx*dx + dy*dy) > 64) { // >8 tiles away
        target_x = pacman.x;
        target_y = pacman.y;
      } else {// bottom-left scatter corner
        target_x = 0; 
        target_y = 18;
      }
      break;
    }
  }

  //calculate next ghost position
  ghost-> direction = get_best_direction(ghost, target_x, target_y);

  switch (ghost->direction) {
    case DIR_UP:{
      ghost->y--;
    break;
    }
    case DIR_LEFT:{
      ghost->x--;
    break;
    }
    case DIR_DOWN:{
      ghost->y++;
    break;
    }
    case DIR_RIGHT:{
      ghost->x++;
    break;
    }
  }
}

void enter_scatter_mode(Ghost* ghost,int ghost_index){
  
  int target_x, target_y;

  switch (ghost_index) {
    case 0://Blinky — top-right
      target_x = 9;
      target_y = 0;
    break;
    case 1://Pinky — top-left
      target_x = 0;
      target_y = 0;
    break;
    case 2://Inky — bottom-right
      target_x = 9;
      target_y = 18;
    break;
    case 3://Clyde — bottom-left
      target_x = 0;
      target_y = 18;
    break;
  }

  //calculate next ghost position
  ghost-> direction = get_best_direction(ghost, target_x, target_y);

  switch (ghost->direction) {
    case DIR_UP:{
      ghost->y--;
    break;
    }
    case DIR_LEFT:{
      ghost->x--;
    break;
    }
    case DIR_DOWN:{
      ghost->y++;
    break;
    }
    case DIR_RIGHT:{
      ghost->x++;
    break;
    }
  }  
}

void enter_eaten_mode(Ghost* ghost){
  int target_x=4;
  int target_y=7;

  // arrived..go back into house
  if(ghost->x== target_x && ghost->y== target_y){
    ghost-> is_eaten = false;
    ghost->in_house = true;

    return;
  }

  int new_x = ghost->x, new_y = ghost->y;
  uint8_t direction = get_best_direction(ghost, target_x, target_y);
  switch (direction) {
    case DIR_UP:    
      new_y--; 
    break;
    case DIR_DOWN:  
      new_y++; 
    break;
    case DIR_LEFT:  
      new_x--; 
    break;
    case DIR_RIGHT: 
      new_x++; 
    break;
  }
  if (is_walkable(new_x, new_y)) {
    ghost->x = new_x;
    ghost->y = new_y;
    ghost->direction = direction;
  }
}

void move_ghosts(){
  unsigned long now = millis();

  // Handle frightened timeout → back to scatter
  if (ghost_mode == 2 && (now - frightened_start >= FRIGHTENED_DURATION)) {
    ghost_mode = 1;
    ghost_mode_start = now;
    reverse = true;
    ghosts_eaten_combo = 0;
  }

  // Handle scatter/chase cycling
  if (ghost_mode == 0 && (now - ghost_mode_start >= CHASE_DURATION)) {
    ghost_mode = 1;
    ghost_mode_start = now;
    reverse = true;
  } else if (ghost_mode == 1 && (now - ghost_mode_start >= SCATTER_DURATION)) {
    ghost_mode = 0;
    ghost_mode_start = now;
    reverse = true;
  }

  for(int i = 0; i < 4; i++){
    Ghost* ghost = &ghosts[i]; 

    if (ghost->in_house) {
      if (millis() >= ghost->exit_time) {
        ghost->x = 4;    // center exit column
        ghost->y = 7;    // one step outside the house
        ghost->in_house = false;
        ghost->direction = DIR_LEFT;
      }
      continue;  // don't move while still waiting inside
    }

    if (ghost->is_eaten) {     
      enter_eaten_mode(ghost);
      continue;                
    }

    switch (ghost_mode) {
      case 0: //chase

        if(reverse){
          reverce_direction(ghost);
        }

        chase_pacman(ghost, i);

        break;
      case 1: //scatter
        if(reverse){
          reverce_direction(ghost);
        }

        enter_scatter_mode(ghost, i);

        break;
      case 2: { //frighten
        if(reverse){
          reverce_direction(ghost);
        }

        pick_random_direction(ghost);
        
        break;
      }
    }

    check_collisions(ghost);
  }

  //reset global flags
  reverse = false;
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
        total_pellets--;
        break;
      case B: // Power Pellet
        game_map[pacman.y][pacman.x] = E;
        score += 50;
        total_pellets--;
        frightened_start = millis();
        ghost_mode=2;
        reverse = true;
        ghosts_eaten_combo = 0;
        break;
    }

    for (int i = 0; i < 4; i++) {
      if (!ghosts[i].in_house && !ghosts[i].is_eaten) {
        check_collisions(&ghosts[i]);
        if (game_over) return;
      }
    }
  }else {
    pacman.current_direction= DIR_NONE;
    pacman_mouth_open = true;
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

void draw_ghost(Ghost* ghost){
  unsigned long now = millis();
  if (now - ghost_last_anim_time >= GHOST_ANIM_DELAY) {
    ghost_anim_frame = !ghost_anim_frame;
    ghost_last_anim_time = now;
  }

  int gx = 2 + (ghost->x * map_grid);
  int gy = 14 + (ghost->y * map_grid);

  const unsigned char* sprite;

  if (ghost->is_eaten) {
    sprite = ghost_eyes_bits;
  } else if (ghost_mode == 2) {
    sprite = ghost_anim_frame ? ghost_frightened_frame_2_bits: ghost_frightened_frame_1_bits;
  } else {
    sprite = ghost_anim_frame ? ghost_frame_2_bits: ghost_frame_1_bits;
  }

  u8g2.drawXBMP(gx, gy, 5, 4, sprite);  
  
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
    
    //horizenal divider line
    u8g2.drawHLine(0, 12, 64);

    draw_map();
    draw_pacman();

    //draw ghosts
    for(int i =0; i<4; i++){
      Ghost* ghost = &ghosts[i]; 

      draw_ghost(ghost);
    }

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

    //show gameover
    u8g2.drawStr(8, 30, "GAME OVER");

    //show score
    u8g2.setCursor(10, 45);
    u8g2.print("SCORE: ");
    u8g2.print(score);

    //show continue instruction
    u8g2.drawStr(8, 75, "Press any");
    u8g2.drawStr(8, 90, "key..");

  } while (u8g2.nextPage());
}
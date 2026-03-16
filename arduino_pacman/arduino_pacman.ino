#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
// R3 rotation makes the screen 64 pixels wide and 128 pixels tall
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R3, -1, A5, A4);

// Button pin definitions
#define BTN_RIGHT 10
#define BTN_DOWN  9
#define BTN_LEFT  7
#define BTN_UP    6

// The new 6x6 grid size
#define map_grid 6

// ==== Tile Identifiers ====
#define E 0 // Empty Space
#define W 1 // Wall
#define P 2 // Pellet (Food)
#define B 3 // Big Power Pellet

//=== Sprites ===
const unsigned char packman_openmouth_bits[] PROGMEM = {
  0x1F, 0x1A, 0x1C, 0x1E, 0x0F
};
const unsigned char packman_closemouth[] PROGMEM = {
  0x0E, 0x1B, 0x1F, 0x1F, 0x0E
};


//==== Game State ====
bool game_over = false;
int score = 0;

//==== Game objects ====
struct Packman {
  int x; // Tile Column (0-9)
  int y; // Tile Row (0-18)
  uint8_t lives;
  uint8_t current_direction;
  uint8_t next_direction;
};

// ==== Game object init ====
Packman packman;

// ==== The NEW Map Array (10 columns x 19 rows) ====
// Perfectly balanced for a 6x6 grid on a 64x128 screen
uint8_t game_map[19][10] = {
  {W,W,W,W,W,W,W,W,W,W}, // 0 - Top Wall
  {W,P,P,P,P,P,P,P,P,W}, // 1
  {W,B,W,W,P,P,W,W,B,W}, // 2 - Power Pellets
  {W,P,P,P,P,P,P,P,P,W}, // 3
  {W,W,W,P,W,W,P,W,W,W}, // 4
  {E,E,E,P,W,W,P,E,E,E}, // 5 - Side Tunnels
  {W,W,W,P,W,W,P,W,W,W}, // 6
  {W,P,P,P,P,P,P,P,P,W}, // 7
  {W,P,W,W,E,E,W,W,P,W}, // 8 - Ghost Box Top
  {W,P,W,E,E,E,E,W,P,W}, // 9 - Ghost Box Center
  {W,P,W,W,W,W,W,W,P,W}, // 10- Ghost Box Bottom
  {W,P,P,P,P,P,P,P,P,W}, // 11
  {W,W,W,P,W,W,P,W,W,W}, // 12
  {W,B,P,P,W,W,P,P,B,W}, // 13- Power Pellets
  {W,P,W,W,W,W,W,W,P,W}, // 14
  {W,P,P,P,P,P,P,P,P,W}, // 15
  {W,W,W,W,P,P,W,W,W,W}, // 16
  {W,P,P,P,P,P,P,P,P,W}, // 17
  {W,W,W,W,W,W,W,W,W,W}  // 18- Bottom Wall
};

// Forward declaration
void draw_game();
void handle_controls();

void setup() {
  u8g2.begin();

  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_UP,    INPUT_PULLUP);

  // Initialize Pac-Man starting grid position
  packman.x = 4;
  packman.y = 15;
  packman.lives = 3;
}

void loop() {
  if (!game_over) {
    handle_controls();
    // Add movement logic here later
    draw_game();
  }
}

void handle_controls() {
  if (digitalRead(BTN_UP) == LOW)    packman.next_direction = 1;
  if (digitalRead(BTN_RIGHT) == LOW) packman.next_direction = 2;
  if (digitalRead(BTN_DOWN) == LOW)  packman.next_direction = 3;
  if (digitalRead(BTN_LEFT) == LOW)  packman.next_direction = 4;
}

void draw_ui() {
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.setCursor(2, 9);
  u8g2.print("SCORE:");
  u8g2.setCursor(38, 9);
  u8g2.print(score);

  // Divider line
  u8g2.drawHLine(0, 12, 64);
}

void draw_map() {
  for (int row = 0; row < 19; row++) {
    for (int col = 0; col < 10; col++) {

      // X offset by +2 to center the 60px map on the 64px screen
      int x = 2 + (col * map_grid); 
      // Y offset by +14 to leave room for the UI at the top
      int y = 14 + (row * map_grid); 
      
      uint8_t tile = game_map[row][col];

      if (tile == W) {
        // Draw solid 6x6 block for walls
        u8g2.drawBox(x, y, map_grid, map_grid);
      }
      else if (tile == P) {
        // Draw a 2x2 normal pellet in the center of the 6x6 tile
        u8g2.drawBox(x + 2, y + 2, 2, 2);
      }
      else if (tile == B) {
        // Draw a larger 4x4 power pellet
        u8g2.drawBox(x + 1, y + 1, 4, 4);
      }
    }
  }
}

void draw_packman() {
  int px = 2 + (packman.x * map_grid);
  int py = 14 + (packman.y * map_grid);

  u8g2.drawXBMP(px + 1, py + 1, 5, 5, packman_openmouth_bits);
}

void draw_game() {
  u8g2.firstPage();
  do {
    draw_ui();
    draw_map();
    draw_packman();
  } while (u8g2.nextPage());
}

#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, -1, A5, A4);

// Button pin definitions
#define BTN_RIGHT 10
#define BTN_DOWN  9
#define BTN_LEFT  7
#define BTN_UP    6

// Game state
bool game_over = false;

// Forward declaration
void draw_game();
void setup() {
  // put your setup code here, to run once:
  u8g2.begin();

  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_UP,    INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(!game_over){
    draw_game();
  }
}


void draw_borders(){

}
void draw_game(){
  u8g2.firstPage();
  do {
    u8g2.drawVLine(0, 0, 63);
    
    u8g2.drawHLine(0, 0, 15);
    u8g2.drawHLine(0, 63, 15);

    u8g2.drawHLine(30, 0, 10);
    u8g2.drawHLine(30, 63, 10);

    u8g2.drawVLine(40, 0, 8);
    u8g2.drawVLine(40, 53, 10);

    u8g2.drawHLine(40, 10, 15);
    u8g2.drawHLine(40, 53, 15);

    u8g2.drawVLine(54, 0, 10);
    u8g2.drawVLine(54, 53, 10);

    u8g2.drawVLine(69, 0, 10);
    u8g2.drawVLine(69, 53, 10);

    u8g2.drawHLine(69, 10, 15);
    u8g2.drawHLine(69, 53, 15);

    u8g2.drawVLine(84, 0, 10);
    u8g2.drawVLine(84, 53, 10);

    u8g2.drawHLine(84, 0, 10);
    u8g2.drawHLine(84, 63, 10);

    u8g2.drawHLine(109, 0, 10);
    u8g2.drawHLine(109, 63, 10);

    u8g2.drawVLine(118, 0, 64);


    //u8g2.drawFrame(0, 0, 40, 20);
  } while (u8g2.nextPage());
}

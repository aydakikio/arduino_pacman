#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R3, -1, A5, A4);

// Button pin definitions
#define BTN_RIGHT 10
#define BTN_DOWN  9
#define BTN_LEFT  7
#define BTN_UP    6

// Game state
bool game_over = false;

const unsigned char heart_bits[] PROGMEM = {
  0x6C, 0xFE, 0xFE, 0xFE, 0x7C,0x38, 0x10
};
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

  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(1, 6, "Score:\ ");


  u8g2.drawHLine(0, 10, 63);

  u8g2.drawVLine(0, 10, 17);
  u8g2.drawVLine(63, 10, 15);

  u8g2.drawVLine(0, 42, 10);
  u8g2.drawVLine(63, 42, 10);

  u8g2.drawHLine(0, 51, 5);
  u8g2.drawHLine(59, 51, 5);

  u8g2.drawVLine(4, 51, 10);
  u8g2.drawVLine(58, 51, 10);


  u8g2.drawHLine(0, 61, 5);
  u8g2.drawHLine(58, 61, 5);



  u8g2.drawHLine(0, 76, 5);
  u8g2.drawHLine(58, 76, 5);



  u8g2.drawVLine(4, 76, 10);
  u8g2.drawVLine(58, 76, 10);


  u8g2.drawHLine(0, 86, 5);
  u8g2.drawHLine(58, 86, 5);


  u8g2.drawVLine(0, 86, 12);
  u8g2.drawVLine(63, 86, 12);


  u8g2.drawVLine(0, 113, 15);
  u8g2.drawVLine(63, 113, 15);


  u8g2.drawHLine(0, 127, 64);
}


void draw_food(){
  
}

void draw_game(){
  u8g2.firstPage();
  do {

    draw_borders();

    //u8g2.drawHLine(0,10,128);
    //THe ghost container
    //u8g2.drawFrame(20,60 , 24, 16);
    u8g2.drawXBMP(20, 60, 8, 7, heart_bits);
  } while (u8g2.nextPage());
}

// BreakTheBricks.ino
//
// This file defines a single function, runBreakTheBricks(), 
// which contains the full logic of your "Break the Bricks" game.
// It is meant to be called from your main UI.ino menu code.
// 
// No setup() or loop() here – the menu code calls runBreakTheBricks().
// The game runs indefinitely (exit by pressing the Arduino RESET button).
//

// We'll access the same 'lcd' object declared in UI.ino.
// We'll define local versions of read_LCD_buttons() and other 
// helper functions so we don't conflict with the rest of the code.

#include <Arduino.h>  // If needed. The main UI.ino likely has #include <LiquidCrystal.h>
                     // and a global 'lcd' object.

////////////////////
// Button Constants
////////////////////
static const int BB_btnRIGHT  = 0;
static const int BB_btnUP     = 1;
static const int BB_btnDOWN   = 2;
static const int BB_btnLEFT   = 3;
static const int BB_btnSELECT = 4;
static const int BB_btnNONE   = 5;

////////////////////
// BPM & Timing
////////////////////
static const int BPM = 30;
static const int BB_whole     = (60000 / BPM);
static const int BB_half      = (30000 / BPM);
static const int BB_quarter   = (15000 / BPM);
static const int BB_eight     = ( 7500 / BPM);
static const int BB_sixteenth = ( 3750 / BPM);
static const int BB_thirty2   = ( 1875 / BPM);

// If you have no speaker, you can keep this pin but comment out the tone() lines.
static const int BB_musicpin = 3;

////////////////////
// Ball and game variables
////////////////////
static float BB_vballx = 1;
static float BB_vbally = 0.2;
static float BB_xball  = 1;
static float BB_yball  = 1;
static int   BB_xmax   = 80;
static int   BB_delaytime = 60;
static int   BB_score  = 0;

////////////////////
// Graphics arrays
////////////////////
static byte BB_dot[8] = {
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0
};
static byte BB_paddle[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B11000,
  B11000
};
static byte BB_wallFR[8] = {0,0,0,0,0,0,0,0};
static byte BB_wallFL[8] = {0,0,0,0,0,0,0,0};
static byte BB_wallBR[8] = {0,0,0,0,0,0,0,0};
static byte BB_wallBL[8] = {0,0,0,0,0,0,0,0};

// 16 bricks (row major: 4 bricks per row, total 4 rows)
static boolean BB_wallarray[16] = {
  1,1,1,1,
  1,1,1,1,
  1,1,1,1,
  1,1,1,1
};

// Paddle position on that 0..16 Y scale
static int BB_paddle_pos = 1;

// We'll declare these as static so they don't clash with other code
static int BB_adc_key_in = 0;

// Forward declarations of local functions:
static int BB_read_LCD_buttons();
static void BB_getPaddle();
static void BB_drawwalls();
static void BB_placedot(int x, int y);
static void BB_placepaddle(int y);
static void BB_createdot(int x, int y);
static void BB_handlecollisions();
static void BB_arkanoidsong();

//-------------------------------------------------------------------
// The main function to run the game
//-------------------------------------------------------------------
void runBreakTheBricks()
{
  

  // Optional short beep or delay
  delay(200);

  // Indefinite game loop – exit by pressing RESET on the Arduino
  while (true) {
    lcd.clear();
    BB_getPaddle();         // move paddle based on UP/DOWN
    BB_drawwalls();         // draw the bricks on the right side
    BB_placepaddle(BB_paddle_pos);
    BB_handlecollisions();  // handle ball collisions
    BB_placedot((int)BB_xball, (int)BB_yball);

    delay(BB_delaytime);
  }
}

//////////////////////////////
// Implementation of local functions
//////////////////////////////

// Reads the analog pin and returns which button is pressed
static int BB_read_LCD_buttons()
{
  BB_adc_key_in = analogRead(A0);
  // typical thresholds for many LCD keypad shields
  if (BB_adc_key_in > 1000) return BB_btnNONE;
  if (BB_adc_key_in < 50)   return BB_btnRIGHT;
  if (BB_adc_key_in < 195)  return BB_btnUP;
  if (BB_adc_key_in < 380)  return BB_btnDOWN;
  if (BB_adc_key_in < 555)  return BB_btnLEFT;
  if (BB_adc_key_in < 790)  return BB_btnSELECT;
  return BB_btnNONE;
}

// Moves the paddle up/down if user presses UP/DOWN
static void BB_getPaddle()
{
  int lcd_key = BB_read_LCD_buttons();
  if (lcd_key == BB_btnUP) {
    if (BB_paddle_pos > 1) BB_paddle_pos -= 1;
  }
  else if (lcd_key == BB_btnDOWN) {
    if (BB_paddle_pos < 14) BB_paddle_pos += 1;
  }
}

// Builds the wall custom chars for the right side
static void BB_drawwalls()
{
  for (int i = 0; i < 8; i += 4)
  {
    for (int j = 0; j < 4; j++)
    {
      BB_wallFL[j+i] = BB_wallarray[i/2]*16 + BB_wallarray[i/2]*8 + BB_wallarray[i/2+1]*2 + BB_wallarray[i/2+1]*1;
      BB_wallFR[j+i] = BB_wallarray[i/2+4]*16 + BB_wallarray[i/2+4]*8 + BB_wallarray[i/2+5]*2 + BB_wallarray[i/2+5]*1;
      BB_wallBL[j+i] = BB_wallarray[i/2+8]*16 + BB_wallarray[i/2+8]*8 + BB_wallarray[i/2+9]*2 + BB_wallarray[i/2+9]*1;
      BB_wallBR[j+i] = BB_wallarray[i/2+12]*16 + BB_wallarray[i/2+12]*8 + BB_wallarray[i/2+13]*2 + BB_wallarray[i/2+13];
    }
  }
  lcd.createChar(2, BB_wallFL);
  lcd.createChar(3, BB_wallFR);
  lcd.createChar(4, BB_wallBL);
  lcd.createChar(5, BB_wallBR);

  lcd.setCursor(14, 0);
  lcd.write((byte)2);
  lcd.write((byte)4);
  lcd.setCursor(14, 1);
  lcd.write((byte)3);
  lcd.write((byte)5);
}

// Places the ball at x, y in that virtual 80×16 space
static void BB_placedot(int x, int y)
{
  BB_createdot(x % 5, y % 8);
  lcd.setCursor(x/5, y/8);
  lcd.write((byte)0);
}

// Places the paddle at vertical position y
static void BB_placepaddle(int y)
{
  for (int i = 0; i < 8; i++){
    BB_paddle[i] = 0x0;
  }
  if (y % 8 > 0) BB_paddle[y%8 - 1] = 0x10;
  BB_paddle[y%8] = 0x10;
  if (y % 8 < 7) BB_paddle[y%8 + 1] = 0x10;
  lcd.createChar(1, BB_paddle);
  lcd.setCursor(0, y/8);
  lcd.write((byte)1);
}

// Creates a dot pattern for x,y in the 5×8 space
static void BB_createdot(int x, int y)
{
  for (int i = 0; i < 8; i++){
    BB_dot[i] = 0x0;
  }
  if (y > 0) BB_dot[y-1] = (B11000 >> x);
  BB_dot[y] = (B11000 >> x);
  lcd.createChar(0, BB_dot);
}

// The main collision and game logic
static void BB_handlecollisions()
{
  BB_xball += BB_vballx;
  BB_yball += BB_vbally;

  // Collisions in y
  if ((BB_yball > 15) || (BB_yball < 1)) {
    BB_vbally = -BB_vbally;
    // tone(BB_musicpin, 880, BB_eight);
  }
  // If ball near right side, check collisions with bricks
  if (BB_xball > 69) {
    for (int i = 0; i < 16; i++) {
      if (BB_xball > (70 + 2*(i%2) + 5*(i/8))) {
        if ((BB_yball > (2*(i%8))) && (BB_yball < (2*(i%8) + 4))) {
          if (BB_wallarray[i] == 1) {
            // tone(BB_musicpin,1174,BB_eight);
            // delay(BB_eight);
            BB_wallarray[i] = 0;
            BB_vballx = -BB_vballx;
            BB_xball = 70;
          }
        }
      }
    }
  }
  // Right edge
  if (BB_xball > BB_xmax) {
    BB_vballx = -BB_vballx;
    // tone(BB_musicpin,880,BB_eight);
  }

  // Left edge (paddle side)
  if (BB_xball < 1) {
    if (BB_paddle_pos > int(BB_yball) - 2 && BB_paddle_pos < int(BB_yball) + 2) {
      // tone(BB_musicpin,1397,BB_sixteenth);
      // delay(BB_sixteenth);
      // tone(BB_musicpin,1567,BB_eight);
      BB_vballx = -BB_vballx;
      BB_vbally *= random(1, 4);
      BB_vbally /= 2;
      BB_score++;
      BB_delaytime = max(BB_delaytime - 2, 10);
      BB_vballx += 0.1;
      int left = 0;
      for (int i = 0; i < 16; i++){
        left += BB_wallarray[i];
      }
      if (left < 1) {
        // Win
        lcd.clear();
        lcd.print("    You Win!    ");
        // BB_arkanoidsong();
        lcd.print(BB_score);
        delay(15000);
        BB_xmax = 80;
        BB_score = 0;
        BB_delaytime = 60;
        for (int i=0; i<16; i++) {
          BB_wallarray[i] = 1;
        }
      }
    }
    else {
      // Missed the paddle
      // tone(BB_musicpin,349,BB_sixteenth);
      // delay(BB_sixteenth);
      // tone(BB_musicpin,329,BB_eight);
      BB_vballx = -BB_vballx;
      BB_vbally *= random(1,4);
      BB_vbally /= 2;
      BB_score--;
    }
  }
}

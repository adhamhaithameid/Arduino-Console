// SnakeGame.ino (Bit-Packed Version)
//
// Single function: runSnakeGame()
// It includes the "Select level" menu, starts the snake, 
// and runs indefinitely (exit by pressing the Arduino RESET button).
//
// The 16×80 board is now stored in xbit[16][10], using bits for each cell.
//
// Make sure you do NOT define LiquidCrystal lcd(...) here.
// Instead, rely on the 'lcd' declared in your UI.ino

#include <Arduino.h>  // If needed; your UI.ino likely has <LiquidCrystal.h> & a global 'lcd' object.

// ==========================
// Data from original code
// ==========================

// 8 custom snake character patterns
static byte SN_mySnake[8][8] =
{
  { B00000, B00000, B00011, B00110, B01100, B11000, B00000 },
  { B00000, B11000, B11110, B00011, B00001, B00000, B00000 },
  { B00000, B00000, B00000, B00000, B00000, B11111, B01110 },
  { B00000, B00000, B00011, B01111, B11000, B00000, B00000 },
  { B00000, B11100, B11111, B00001, B00000, B00000, B00000 },
  { B00000, B00000, B00000, B11000, B01101, B00111, B00000 },
  { B00000, B00000, B01110, B11011, B11111, B01110, B00000 },
  { B00000, B00000, B00000, B01000, B10000, B01000, B00000 }
};

// 5 possible levels, each is a 2×16 boolean grid
// For example, levelz[1][0][0] is row=0,col=0 of level 1, etc.
static boolean SN_levelz[5][2][16] =
{
  { // Level 0 (all false)
    {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
    {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false}
  },
  { // Level 1
    {true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true},
    {true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true}
  },
  { // Level 2
    {true,false,false,false,true,false,false,false,false,false,false,true,false,false,false,true},
    {true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,true}
  },
  { // Level 3
    {true,false,true,false,false,false,false,false,false,true,false,false,false,true,false,false},
    {false,false,false,false,true,false,false,true,false,false,false,true,false,false,false,true}
  },
  { // Level 4 (an example if you originally had 5 levels)
    // If your code only used up to 3, adapt as you wish
    {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
    {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false}
  }
};

static int SN_levels = 5;  // number of levels (0..4)
static int SN_selectedLevel = 1;

// The board size 16×80, but we store bits in xbit[16][10].
static uint8_t xbit[16][10];  // each row has 80 bits = 10 bytes

// Helper to get or set a cell in xbit
static bool getCell(int row, int col) {
  // col/8 picks the byte; col%8 picks the bit
  uint8_t b = xbit[row][col/8];
  return (b & (1 << (col%8))) != 0;
}
static void setCell(int row, int col, bool val) {
  if (val) {
    xbit[row][col/8] |=  (1 << (col%8));
  } else {
    xbit[row][col/8] &= ~(1 << (col%8));
  }
}

// ==========================
// Snake Game Variables
// ==========================
static unsigned long SN_time, SN_timeNow;
static int  SN_gameSpeed;
static bool SN_skip, SN_gameOver, SN_gameStarted;
static int  SN_olddir;

static int  SN_adc_key_val[5] = {50, 200, 400, 600, 800};
static int  SN_NUM_KEYS = 5;
static int  SN_adc_key_in;
static int  SN_key = -1;
static int  SN_oldkey = -1;

static byte SN_myChar[8];  // used for building custom characters
static bool SN_special;

// The snake is a linked list of parts
struct SN_partdef
{
  int row, column, dir; // dir=0 up,1 down,2 right,3 left
  SN_partdef* next;
};
typedef SN_partdef SN_part;

static SN_part * SN_head = nullptr;
static SN_part * SN_tail = nullptr;

static int SN_i, SN_j, SN_collected;
static long SN_pc, SN_pr; // position of the point

// ============================
// Function Prototypes
// ============================
static int  SN_get_key(unsigned int input);
static void SN_drawMatrix();
static void SN_freeList();
static void SN_gameOverFunction();
static void SN_growSnake();
static void SN_newPoint();
static void SN_moveHead();
static void SN_moveAll();
static void SN_createSnake(int n);
static void SN_startF();

// ============================
// Main function for the game
// ============================
void runSnakeGame()
{
  // "startF()" is basically your old "setup()" routine
  SN_startF();

  // infinite loop
  while(true)
  {
    // if the game hasn't started, we do the "Select Level" logic
    if (!SN_gameOver && !SN_gameStarted)
    {
      SN_adc_key_in = analogRead(0);
      SN_key = SN_get_key(SN_adc_key_in);
      if (SN_key != SN_oldkey)
      {
        delay(50);
        SN_adc_key_in = analogRead(0);
        SN_key = SN_get_key(SN_adc_key_in);
        if (SN_key != SN_oldkey)
        {
          SN_oldkey = SN_key;
          if (SN_key >= 0)
          {
            SN_olddir = SN_head->dir;
            // key=1 means up, key=2 means down, key=4 means select
            if (SN_key == 1 && SN_selectedLevel < SN_levels-1) SN_selectedLevel++;
            if (SN_key == 2 && SN_selectedLevel > 1) SN_selectedLevel--;
            if (SN_key == 4)
            {
              // user pressed SELECT to start
              lcd.clear();
              // your original code does 'selectedLevel--;' for some reason, we keep it
              SN_selectedLevel--;
              SN_newPoint();
              SN_gameStarted = true;
            } else {
              // show the current level
              lcd.setCursor(14, 0);
              lcd.print(SN_selectedLevel);
            }
          }
        }
      }
    }
    // once gameStarted is true, we do normal snake movement
    else if (!SN_gameOver && SN_gameStarted)
    {
      SN_skip = false;
      SN_adc_key_in = analogRead(0);
      SN_key = SN_get_key(SN_adc_key_in);
      if (SN_key != SN_oldkey)
      {
        delay(50);
        SN_adc_key_in = analogRead(0);
        SN_key = SN_get_key(SN_adc_key_in);
        if (SN_key != SN_oldkey)
        {
          SN_oldkey = SN_key;
          if (SN_key >= 0)
          {
            SN_olddir = SN_head->dir;
            // interpret key 0=RIGHT,1=UP,2=DOWN,3=LEFT
            if (SN_key == 0 && SN_head->dir != 3) SN_head->dir = 2;
            if (SN_key == 1 && SN_head->dir != 1) SN_head->dir = 0;
            if (SN_key == 2 && SN_head->dir != 0) SN_head->dir = 1;
            if (SN_key == 3 && SN_head->dir != 2) SN_head->dir = 3;

            if (SN_olddir != SN_head->dir)
            {
              SN_skip = true;
              delay(1000 / SN_gameSpeed);
              SN_moveAll();
              SN_drawMatrix();
            }
          }
        }
      }
      if (!SN_skip)
      {
        SN_timeNow = millis();
        if (SN_timeNow - SN_time > (unsigned long)(1000 / SN_gameSpeed))
        {
          SN_moveAll();
          SN_drawMatrix();
          SN_time = millis();
        }
      }
    }
    // If the game is over, user can press a button to call SN_startF() again
    else if(SN_gameOver)
    {
      SN_adc_key_in = analogRead(0);
      SN_key = SN_get_key(SN_adc_key_in);
      if (SN_key != SN_oldkey)
      {
        delay(50);
        SN_adc_key_in = analogRead(0);
        SN_key = SN_get_key(SN_adc_key_in);
        if (SN_key != SN_oldkey)
        {
          SN_oldkey = SN_key;
          if (SN_key >= 0)
          {
            // start a new game
            SN_startF();
          }
        }
      }
    }

    delay(20); // a small loop delay
  }
}

// ============================
// Implementation
// ============================

// Convert the raw analog read to a key 0..4 or -1
static int SN_get_key(unsigned int input)
{
  for (int k = 0; k < SN_NUM_KEYS; k++)
  {
    if (input < SN_adc_key_val[k])
    {
      return k;
    }
  }
  return -1;
}

// Render the entire 2×16 portion of the LCD
static void SN_drawMatrix()
{
  int cc = 0;
  if (!SN_gameOver)
  {
    // ensure the "point" is set
    setCell(SN_pr, SN_pc, true);
    // For each row 0..1 on the LCD
    for(int r=0; r<2; r++)
    {
      // For each column 0..15
      for(int c=0; c<16; c++)
      {
        SN_special = false;
        // Build a custom char for each of the 8 rows in that cell
        for(int i=0; i<8; i++)
        {
          byte b = B00000;
          // check bits for the 5 pixels in that row
          if (getCell(r*8 + i, c*5+0)) { b |= B10000; SN_special = true; }
          if (getCell(r*8 + i, c*5+1)) { b |= B01000; SN_special = true; }
          if (getCell(r*8 + i, c*5+2)) { b |= B00100; SN_special = true; }
          if (getCell(r*8 + i, c*5+3)) { b |= B00010; SN_special = true; }
          if (getCell(r*8 + i, c*5+4)) { b |= B00001; SN_special = true; }
          SN_myChar[i] = b;
        }
        if (SN_special)
        {
          lcd.createChar(cc, SN_myChar);
          lcd.setCursor(c, r);
          lcd.write(byte(cc));
          cc++;
        }
        else
        {
          lcd.setCursor(c, r);
          // if there's a wall in that cell, print 255
          // else print 254
          if (SN_levelz[SN_selectedLevel][r][c]) lcd.write((byte)255);
          else lcd.write((byte)254);
        }
      }
    }
  }
}

// free the entire snake linked list
static void SN_freeList()
{
  SN_part *p = SN_tail;
  while (p != nullptr)
  {
    SN_part *q = p;
    p = p->next;
    free(q);
  }
  SN_head = SN_tail = nullptr;
}

// Called if we set gameOver = true
static void SN_gameOverFunction()
{
  delay(1000);
  lcd.clear();
  SN_freeList();
  lcd.setCursor(3,0);
  lcd.print("   Game  Over!  ");
  lcd.setCursor(4,1);
  lcd.print("   Score: ");
  lcd.print(SN_collected);
  delay(1000);
}

// Add a segment at the tail
static void SN_growSnake()
{
  SN_part *p = (SN_part*)malloc(sizeof(SN_part));
  p->row = SN_tail->row;
  p->column = SN_tail->column;
  p->dir = SN_tail->dir;
  p->next = SN_tail;
  SN_tail = p;
}

// Place a new "point" at random
static void SN_newPoint()
{
  bool newp = true;
  while(newp)
  {
    SN_pr = random(16);
    SN_pc = random(80);
    newp = false;
    // check for wall
    if (SN_levelz[SN_selectedLevel][SN_pr/8][SN_pc/5]) newp = true;

    // check if inside the snake
    SN_part *p = SN_tail;
    while (p->next != nullptr && !newp)
    {
      if (p->row == SN_pr && p->column == SN_pc) newp = true;
      p = p->next;
    }
  }
  // setCell(SN_pr, SN_pc, true); // We'll do that in drawMatrix
  // If we've collected <13 points and game started, grow the snake
  if (SN_collected < 13 && SN_gameStarted) SN_growSnake();
}

// Move the snake's head 1 step
static void SN_moveHead()
{
  switch(SN_head->dir)
  {
    case 0: SN_head->row--;    break; // up
    case 1: SN_head->row++;    break; // down
    case 2: SN_head->column++; break; // right
    case 3: SN_head->column--; break; // left
    default: break;
  }
  // wrap around
  if (SN_head->column >= 80) SN_head->column = 0;
  if (SN_head->column < 0)   SN_head->column = 79;
  if (SN_head->row >= 16)    SN_head->row = 0;
  if (SN_head->row < 0)      SN_head->row = 15;

  // check wall
  if (SN_levelz[SN_selectedLevel][SN_head->row/8][SN_head->column/5]) SN_gameOver = true;

  // check self collision
  SN_part *p = SN_tail;
  while (p != SN_head && !SN_gameOver)
  {
    if (p->row == SN_head->row && p->column == SN_head->column)
      SN_gameOver = true;
    p = p->next;
  }
  if (SN_gameOver)
    SN_gameOverFunction();
  else
  {
    // place the head
    setCell(SN_head->row, SN_head->column, true);
    // if we are on the point
    if (SN_head->row == SN_pr && SN_head->column == SN_pc)
    {
      SN_collected++;
      if (SN_gameSpeed < 25) SN_gameSpeed += 3;
      SN_newPoint();
    }
  }
}

// Moves the entire snake by removing tail from xbit and shifting
static void SN_moveAll()
{
  // remove tail
  setCell(SN_tail->row, SN_tail->column, false);

  // shift each segment to next
  SN_part *p = SN_tail;
  while (p->next != nullptr)
  {
    p->row = p->next->row;
    p->column = p->next->column;
    p->dir = p->next->dir;
    p = p->next;
  }
  // move head
  SN_moveHead();
}

// Create a snake of size n
static void SN_createSnake(int n)
{
  // Clear entire bit array
  for(int rr=0; rr<16; rr++)
    for(int cc=0; cc<10; cc++)
      xbit[rr][cc] = 0;

  // Build snake from tail to head
  SN_part *p, *q;
  SN_tail = (SN_part*)malloc(sizeof(SN_part));
  SN_tail->row = 7;
  SN_tail->column = 39 + n/2; // center-ish
  SN_tail->dir = 3; // left
  q = SN_tail;
  setCell(SN_tail->row, SN_tail->column, true);
  for (int i=0; i < n-1; i++)
  {
    p = (SN_part*)malloc(sizeof(SN_part));
    p->row = q->row;
    p->column = q->column - 1; // place left
    setCell(p->row, p->column, true);
    p->dir = q->dir;
    q->next = p;
    q = p;
  }
  if (n>1)
  {
    p->next = nullptr;
    SN_head = p;
  } else {
    SN_tail->next = nullptr;
    SN_head = SN_tail;
  }
}

// The function that originally was "setup()"
static void SN_startF()
{
  SN_gameOver = false;
  SN_gameStarted = false;
  SN_selectedLevel = 1;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select level: 1");

  for (int i=0; i<8; i++)
  {
    lcd.createChar(i, SN_mySnake[i]);
    lcd.setCursor(i+4,1);
    lcd.write(byte(i));
  }
  SN_collected = 0;
  SN_gameSpeed = 8;
  SN_createSnake(3);
  SN_time = 0;
}

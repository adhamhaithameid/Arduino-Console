/* Chrome Dino
 * Dino sprite style: T-Rex chunky v2 (cleaner head, bigger foot, spiky tail) — Chrome-like silhouette (4-cell dino)
 * Hardware: Arduino Uno + 16x2 LCD Keypad Shield (A0 buttons)
 * Controls: UP or SELECT to jump; SELECT to start/restart
 * Sprite:
 *   Two-cell dino style (CREA ELECTRONICA look) with alternate feet; cactus obstacle.
 */

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// ---- Buttons via A0 ----
#define BTN_RIGHT  0
#define BTN_UP     1
#define BTN_DOWN   2
#define BTN_LEFT   3
#define BTN_SELECT 4
#define BTN_NONE   5

int read_LCD_buttons() {
  int adc = analogRead(A0);
  if (adc < 50)   return BTN_RIGHT;
  if (adc < 195)  return BTN_UP;
  if (adc < 380)  return BTN_DOWN;
  if (adc < 555)  return BTN_LEFT;
  if (adc < 790)  return BTN_SELECT;
  return BTN_NONE;
}

// Keep your inline style
static inline bool isJumpPressed(){ int b = read_LCD_buttons(); return (b == BTN_UP || b == BTN_SELECT); }

// ---- Custom characters ----
// Index map: 0 HEAD, 1 CHEST_ARM, 2 LEGS_RUN1, 3 TAIL, 4 LEGS_RUN2, 5 LEGS_JUMP, 6 CACTUS

// Head with snout (right), eye hole, jaw and neck
// ---- Single-cell dino frames ----
// ---- Cactus ----
byte CACTUS[8] = {
  B00100,
  B00101,
  B10101,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100
};

// --- Dino style from CREA ELECTRONICA / Max Imagination (two-cell wide) ---
byte DINO_PARADO_PARTE_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00001};
byte DINO_PARADO_PARTE_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B01000};
byte DINO_PIE_DERE_PART_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00001};
byte DINO_PIE_DERE_PART_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B00000};
byte DINO_PIE_IZQU_PART_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00000};
byte DINO_PIE_IZQU_PART_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B01000};

// --- Upper obstacle: bird (animated wings on index 7) ---
byte BIRD_UP[8] = {
  B00000,
  B00001,
  B00011,
  B00111,
  B01111,
  B11111,
  B00000,
  B00000
};

byte BIRD_DOWN[8] = {
  B00000,
  B10000,
  B11000,
  B11100,
  B11110,
  B11111,
  B00000,
  B00000
};

enum State { START, RUN, GAMEOVER };
State state = START;

// Dino placement (two cells wide)
const uint8_t DINO_COL1 = 1;
const uint8_t DINO_COL2 = 2;


// Dino placement (single cell at column DINO_COL)
// HEAD
// CHEST/ARM
// LEGS (anim)
// TAIL
uint8_t dinoRow = 1;         // 0 = top, 1 = bottom
bool footSwap = false;       // toggle run frames

// Obstacle
int8_t cactusCol = 15;
bool cactusActive = false;

// Upper obstacle (bird)
int8_t birdCol = 15;
bool birdActive = false;

// Difficulty/scaling
uint8_t cactusSpawnChance = 2; // out of 10
uint8_t birdSpawnChance   = 0; // out of 10 (starts at 0 until score threshold)


// Timing
unsigned long lastTick = 0;
unsigned long tickInterval = 165;
const unsigned long tickMin = 85;

// Jump
bool jumping = false;
unsigned long jumpStart = 0;
const unsigned long jumpDuration = 520;

// Score
unsigned long score = 0;

// ---- Drawing helpers ----
void drawScore() {
  lcd.setCursor(10, 0);
  char buf[7];
  sprintf(buf, "%6lu", score);
  lcd.print(buf);
}

void showHUD() {
  lcd.setCursor(8, 0); lcd.print("S:");
  drawScore();
}

void clearDinoRow(uint8_t row) {
  lcd.setCursor(DINO_COL1, row);
  lcd.print("  ");
}



void drawDino() {
  // Alternate legs; during jump we can keep alternating for effect
  bool right = footSwap;
  uint8_t leftIdx  = right ? 0 : 2; // left cell bitmap index group
  uint8_t rightIdx = right ? 1 : 3; // right cell bitmap index group
  lcd.setCursor(DINO_COL1, dinoRow); lcd.write(byte(leftIdx));
  lcd.setCursor(DINO_COL2, dinoRow); lcd.write(byte(rightIdx));
}



inline void spawnCactus(){ cactusCol = 15; cactusActive = true; }
inline void eraseCactus(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,1); lcd.print(' ');} }
inline void drawCactusAt(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,1); lcd.write(byte(6)); } }

inline void spawnBird(){ birdCol = 15; birdActive = true; }
inline void eraseBird(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,0); lcd.print(' ');} }
inline void drawBirdAt(int8_t c){
  if (c>=0 && c<16){
    lcd.createChar(7, (footSwap ? BIRD_UP : BIRD_DOWN));
    lcd.setCursor(c,0);
    lcd.write(byte(7));
  }
}

void updateDifficulty() {
  // Tick speed scales with score (down to tickMin)
  unsigned long base = 165;
  unsigned long dec = (score * 2);
  if (dec > (base - tickMin)) dec = (base - tickMin);
  tickInterval = base - dec;

  // Spawn chances scale with score
  if (score < 10) { cactusSpawnChance = 2; birdSpawnChance = 0; }
  else if (score < 20) { cactusSpawnChance = 3; birdSpawnChance = 2; }
  else if (score < 40) { cactusSpawnChance = 4; birdSpawnChance = 3; }
  else if (score < 60) { cactusSpawnChance = 5; birdSpawnChance = 4; }
  else { cactusSpawnChance = 6; birdSpawnChance = 5; } // still out of 10
}
bool checkCollision() {
  if (dinoRow == 1) { // ground: cactus
    if (cactusActive && (cactusCol >= DINO_COL1 && cactusCol <= DINO_COL2)) return true;
  } else { // top: bird
    if (birdActive && (birdCol >= DINO_COL1 && birdCol <= DINO_COL2)) return true;
  }

  return false;
}


void resetGame(bool toStart) {
  lcd.clear();
  score = 0;
  tickInterval = 165;
  footSwap = false;
  jumping = false;
  dinoRow = 1;
  cactusActive = false;
  cactusCol = 15;

  if (toStart) {
    lcd.setCursor(0, 0); lcd.print("  CHROME  DINO  ");
    lcd.setCursor(0, 1); lcd.print(" Press SELECT   ");
    state = START;
  } else {
    showHUD();
    state = RUN;
  }
}

void setup() {
  lcd.begin(16, 2);
    randomSeed(analogRead(A0));
// 0/1 = right foot; 2/3 = left foot; 4/5 = idle; 6 = cactus
  lcd.createChar(0, DINO_PIE_DERE_PART_1);
  lcd.createChar(1, DINO_PIE_DERE_PART_2);
  lcd.createChar(2, DINO_PIE_IZQU_PART_1);
  lcd.createChar(3, DINO_PIE_IZQU_PART_2);
  lcd.createChar(4, DINO_PARADO_PARTE_1);
  lcd.createChar(5, DINO_PARADO_PARTE_2);
  lcd.createChar(6, CACTUS);
  resetGame(true);
}


void loop() {
  switch (state) {
    case START: {
      if (read_LCD_buttons() == BTN_SELECT) {
        lcd.clear();
        showHUD();
        state = RUN;
        lastTick = millis();
      }
    } break;

    case RUN: {
      unsigned long now = millis();

      // Jump edge-trigger
      static bool prevJump = false;
      bool j = isJumpPressed();
      if (j && !prevJump && !jumping) {
        jumping = true;
        jumpStart = now;
        clearDinoRow(1);
        dinoRow = 0;
      }
      prevJump = j;

      if (jumping && (now - jumpStart >= jumpDuration)) {
        jumping = false;
        clearDinoRow(0);
        dinoRow = 1;
      }

      if (now - lastTick >= tickInterval) {
        lastTick = now;
        footSwap = !footSwap;

        // Cactus movement
        updateDifficulty();
        if (!cactusActive) {
          // avoid spawning right on top of a fresh bird near the edge
          bool birdTooClose = (birdActive && birdCol > 11);
          if (!birdTooClose && (random(0, 10) < cactusSpawnChance || score == 0)) spawnCactus();
        } else {
          eraseCactus(cactusCol);
          cactusCol--;
          if (cactusCol < -1) {
            cactusActive = false;
            score++;
            drawScore();
          } else {
            drawCactusAt(cactusCol);
          }
        }

        // Bird movement
        if (!birdActive) {
          // Start birds only after a few points and keep spacing from fresh cactus
          bool cactusTooClose = (cactusActive && cactusCol > 11);
          if (!cactusTooClose && score > 5 && random(0, 10) < birdSpawnChance) spawnBird();
        } else {
          eraseBird(birdCol);
          birdCol--;
          if (birdCol < -1) {
            birdActive = false;
            score++;
            drawScore();
          } else {
            drawBirdAt(birdCol);
          }
        }

        // Draw dino
        drawDino();

        // Collision
        if (checkCollision()) {
          state = GAMEOVER;
          lcd.setCursor(0, 0); lcd.print("   GAME  OVER   ");
          lcd.setCursor(0, 1); lcd.print("Score: "); lcd.print(score);
          lcd.print("  SEL=Again ");
        }
      }
    } break;

    case GAMEOVER: {
      if (read_LCD_buttons() == BTN_SELECT) {
        resetGame(true);
      }
    } break;
  }
}

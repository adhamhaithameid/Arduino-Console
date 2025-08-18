#ifdef EMBED_DINO_IN_UI
  // When embedded in the menu UI:
  // - rename Arduino entry points so UI can call one frame at a time
  // - reuse UI's lcd and button reader
  #define setup dinoSetup
  #define loop  runChromeDino
  class LiquidCrystal;            // forward declaration to avoid include order issues
  extern LiquidCrystal lcd;
  extern int read_LCD_buttons();
#endif

/* Chrome Dino (16x2 LCD Keypad Shield)
 * - Dino pixels: UNCHANGED (CREA ELECTRONICA / Max Imagination two-cell style)
 * - Separate lanes: ground cactus (row 1) and bird (row 0)
 * - Score-based difficulty, clean Game Over
 * - Start/Restart with SELECT; Jump with UP or SELECT
 */

#include <LiquidCrystal.h>

#ifndef EMBED_DINO_IN_UI
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#endif

// ---------- Button mapping (no BTN_* macros to avoid clashes) ----------
#ifndef DINO_BTN_RIGHT
  const uint8_t DINO_BTN_RIGHT  = 0;
  const uint8_t DINO_BTN_UP     = 1;
  const uint8_t DINO_BTN_DOWN   = 2;
  const uint8_t DINO_BTN_LEFT   = 3;
  const uint8_t DINO_BTN_SELECT = 4;
  const uint8_t DINO_BTN_NONE   = 5;
#endif

#ifndef EMBED_DINO_IN_UI
// Local reader only in standalone builds
int read_LCD_buttons() {
  int adc = analogRead(A0);
  if (adc < 50)   return DINO_BTN_RIGHT;
  if (adc < 195)  return DINO_BTN_UP;
  if (adc < 380)  return DINO_BTN_DOWN;
  if (adc < 555)  return DINO_BTN_LEFT;
  if (adc < 790)  return DINO_BTN_SELECT;
  return DINO_BTN_NONE;
}
#endif

static inline bool isJumpPressed(){
  int b = read_LCD_buttons();
  return (b == DINO_BTN_UP || b == DINO_BTN_SELECT);
}

// ---------- Custom characters (PRESERVED DINO PIXELS) ----------
// CACTUS
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

// Dino (two-cell style) — DO NOT CHANGE
byte DINO_PARADO_PARTE_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00001};
byte DINO_PARADO_PARTE_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B01000};
byte DINO_PIE_DERE_PART_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00001};
byte DINO_PIE_DERE_PART_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B00000};
byte DINO_PIE_IZQU_PART_1[8] = {B00000, B00000, B00010, B00010, B00011, B00011, B00001, B00000};
byte DINO_PIE_IZQU_PART_2[8] = {B00111, B00111, B00111, B00100, B11100, B11100, B11000, B01000};

// Bird (animated into slot 7 only)
byte BIRD_UP[8] =   {B00000, B00001, B00011, B00111, B01111, B11111, B00000, B00000};
byte BIRD_DOWN[8] = {B00000, B10000, B11000, B11100, B11110, B11111, B00000, B00000};

// Character slots (lock dino pixels in 0..6; only slot 7 animates)
enum : uint8_t {
  CH_DINO_L0   = 0, // right-foot left cell
  CH_DINO_R0   = 1, // right-foot right cell
  CH_DINO_L1   = 2, // left-foot  left cell
  CH_DINO_R1   = 3, // left-foot  right cell
  CH_DINO_IDLE_L = 4,
  CH_DINO_IDLE_R = 5,
  CH_CACTUS    = 6,
  CH_BIRD      = 7
};

// ---------- Game state ----------
enum State { START, RUN, GAMEOVER };
State state = START;

// Dino placement (two cells wide)
const uint8_t DINO_COL1 = 1;
const uint8_t DINO_COL2 = 2;
uint8_t dinoRow = 1;         // 0 = top, 1 = bottom
bool footSwap = false;       // toggle run frames

// Obstacles
int8_t cactusCol = 15;  bool cactusActive = false;
int8_t birdCol   = 15;  bool birdActive   = false;

// Difficulty/scaling
uint8_t cactusSpawnChance = 2; // out of 10
uint8_t birdSpawnChance   = 0; // out of 10 (begins at 0)

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

// ---------- Helpers ----------
void drawScore() {
  lcd.setCursor(10, 0);
  char buf[7];
  sprintf(buf, "%6lu", score);
  lcd.print(buf);
}

void showHUD() {
  lcd.setCursor(15, 0);
  drawScore();
}

void clearDinoRow(uint8_t row) {
  lcd.setCursor(DINO_COL1, row);
  lcd.print("  ");
}

void drawDino() {
  // Alternate legs; dino pixels fixed to slots 0..5
  bool right = footSwap;
  uint8_t leftIdx  = right ? CH_DINO_L0 : CH_DINO_L1;
  uint8_t rightIdx = right ? CH_DINO_R0 : CH_DINO_R1;
  lcd.setCursor(DINO_COL1, dinoRow); lcd.write(leftIdx);
  lcd.setCursor(DINO_COL2, dinoRow); lcd.write(rightIdx);
}

inline void spawnCactus(){ cactusCol = 15; cactusActive = true; }
inline void eraseCactus(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,1); lcd.print(' ');} }
inline void drawCactusAt(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,1); lcd.write(CH_CACTUS); } }

inline void spawnBird(){ birdCol = 15; birdActive = true; }
inline void eraseBird(int8_t c){ if (c>=0 && c<16){ lcd.setCursor(c,0); lcd.print(' ');} }
inline void drawBirdAt(int8_t c){
  if (c>=0 && c<16){
    lcd.createChar(CH_BIRD, (footSwap ? BIRD_UP : BIRD_DOWN)); // only slot 7 is updated
    lcd.setCursor(c,0);
    lcd.write(CH_BIRD);
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
  else { cactusSpawnChance = 6; birdSpawnChance = 5; } // out of 10
}

bool checkCollision() {
  if (dinoRow == 1) { // ground: cactus
    if (cactusActive && (cactusCol >= DINO_COL1 && cactusCol <= DINO_COL2)) return true;
  } else { // top: bird
    if (birdActive && (birdCol >= DINO_COL1 && birdCol <= DINO_COL2)) return true;
  }
  return false;
}

inline void enterGameOver() {
  state = GAMEOVER;
  jumping = false;
  footSwap = false;
  lcd.setCursor(0, 0); lcd.print("   GAME  OVER   ");
  lcd.setCursor(0, 1); lcd.print("   Score: "); lcd.print(score); lcd.print("    ");
}

void resetGame(bool toStart) {
  lcd.clear();
  score = 0;
  tickInterval = 165;
  footSwap = false;
  jumping = false;
  dinoRow = 1;
  cactusActive = false; cactusCol = 15;
  birdActive   = false; birdCol   = 15;

  
    showHUD();
    state = RUN;
  
}

// ---------- Arduino entry points (renamed when embedded) ----------
void setup() {
  lcd.begin(16, 2);
  randomSeed(analogRead(A0));

  // Load fixed glyphs (dino & cactus). Only bird slot is animated later.
  lcd.createChar(CH_DINO_L0,     DINO_PIE_DERE_PART_1);
  lcd.createChar(CH_DINO_R0,     DINO_PIE_DERE_PART_2);
  lcd.createChar(CH_DINO_L1,     DINO_PIE_IZQU_PART_1);
  lcd.createChar(CH_DINO_R1,     DINO_PIE_IZQU_PART_2);
  lcd.createChar(CH_DINO_IDLE_L, DINO_PARADO_PARTE_1);
  lcd.createChar(CH_DINO_IDLE_R, DINO_PARADO_PARTE_2);
  lcd.createChar(CH_CACTUS,      CACTUS);

  resetGame(true);
}

void loop() {
  switch (state) {
    case START: {
      if (read_LCD_buttons() == DINO_BTN_SELECT) {
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

        // Difficulty
        updateDifficulty();

        // Cactus movement / spawn
        if (!cactusActive) {
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

        // Bird movement / spawn
        if (!birdActive) {
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
          enterGameOver();
          return; // stop processing this frame
        }
      }
    } break;

    case GAMEOVER: {
      if (read_LCD_buttons() == DINO_BTN_SELECT) {
        resetGame(true);
      }
    } break;
  }
}

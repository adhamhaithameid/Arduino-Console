// UI.ino
// Menu UI for LCD Keypad Shield, integrates Chrome Dino as an app.
// Put Chrome_Dino.ino in the SAME sketch folder. It will see EMBED_DINO_IN_UI
// and rename its setup() -> dinoSetup() and loop() -> runChromeDino().

#define EMBED_DINO_IN_UI 1

#include <LiquidCrystal.h>

// App runners provided by their own .ino files
void runBreakTheBricks();
void runDecisionCompass();
void runHelicopter();
void runHowManyDays();
void runSnakeGame();
void runEyeAnimation();
void runDinoFromUI();  // wrapper inside this file

// ======================= LCD & Buttons =======================
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Button definitions & thresholds for common 16x2 LCD Keypad Shield
#define UI_BTN_RIGHT  0
#define UI_BTN_UP     1
#define UI_BTN_DOWN   2
#define UI_BTN_LEFT   3
#define UI_BTN_SELECT 4
#define UI_BTN_NONE   5

int read_LCD_buttons() {
  int adc = analogRead(A0);
  if (adc < 50)   return UI_BTN_RIGHT;
  if (adc < 195)  return UI_BTN_UP;
  if (adc < 380)  return UI_BTN_DOWN;
  if (adc < 555)  return UI_BTN_LEFT;
  if (adc < 790)  return UI_BTN_SELECT;
  return UI_BTN_NONE;
}

// ======================= App Integration =======================
// Chrome Dino will expose these names after macro-renaming in its file:
void dinoSetup();        // original setup() inside Chrome_Dino.ino
void runChromeDino();    // original loop()  inside Chrome_Dino.ino (one iteration)

// ======================= Menu State =======================
enum AppState {
  MENU,
  APP_BRICKS,
  APP_COMPASS,
  APP_HELICOPTER,
  APP_DAYS,
  APP_SNAKE,
  APP_EYE,      // Eye Animation
  APP_DINO      // Chrome Dino
};

const char* menuItems[] = {
  " Break Bricks",
  " Decision Cps",   // shortened for 16-char LCD
  " Helicopter",
  " How Many Days",
  " Snake",
  " Chrome Dino"
};
int numMenuItems = 6;

volatile AppState currentState = MENU;
int currentMenu = 0;

// Idle screensaver timer
unsigned long lastActivityMs = 0;
const unsigned long idleTimeoutMs = 30000; // 30s

// ======================= UI Helpers =======================
void displayMenu() {
  lcd.clear();
  int firstItem = (currentMenu / 2) * 2;

  // Top line
  lcd.setCursor(0, 0);
  lcd.print((firstItem == currentMenu) ? " >" : " ");
  lcd.print(menuItems[firstItem]);

  // Bottom line
  if (firstItem + 1 < numMenuItems) {
    lcd.setCursor(0, 1);
    lcd.print(((firstItem + 1) == currentMenu) ? " >" : " ");
    lcd.print(menuItems[firstItem + 1]);
  }
}

void pickCurrentApp() {
  switch (currentMenu) {
    case 0: currentState = APP_BRICKS;     break;
    case 1: currentState = APP_COMPASS;    break;
    case 2: currentState = APP_HELICOPTER; break;
    case 3: currentState = APP_DAYS;       break;
    case 4: currentState = APP_SNAKE;      break;
    case 6: currentState = APP_DINO;       break;
    default: currentState = MENU;          break;
  }
}

void runDinoFromUI() {
  dinoSetup();  // initialize Dino app
  unsigned long holdStart = 0;
  const unsigned long exitHoldMs = 1000;

  while (true) {
    runChromeDino();  // one iteration of the Dino game

    int btn = read_LCD_buttons();
    
  // Update idle timer on any button activity
  if (btn != UI_BTN_NONE) lastActivityMs = millis();
if (btn == UI_BTN_LEFT) {
      if (holdStart == 0) holdStart = millis();
      if (millis() - holdStart >= exitHoldMs) {
        // Exit signal: clear and return to menu
        lcd.clear();
        lcd.setCursor(0,0); lcd.print("Exiting Dino...");
        delay(400);
        break;
      }
    } else {
      holdStart = 0; // reset hold timer
    }
    delay(10); // small pacing
  }
}

// ======================= Setup & Loop =======================
void setup() {
  lcd.begin(16, 2);
    lastActivityMs = millis();
randomSeed(analogRead(A0));
  displayMenu();
}

void loop() {
  // Menu navigation
  displayMenu();
  int btn = read_LCD_buttons();

  
  // Update idle timer on any button activity
  if (btn != UI_BTN_NONE) lastActivityMs = millis();
if (btn == UI_BTN_UP) {
    if (currentMenu > 0) currentMenu--;
    delay(180);
  } else if (btn == UI_BTN_DOWN) {
    if (currentMenu < numMenuItems - 1) currentMenu++;
    delay(180);
  } else if (currentState == MENU && (millis() - lastActivityMs >= idleTimeoutMs)) {
    // Run screensaver (Eye Animation) until a button press returns
    runEyeAnimation();
    lastActivityMs = millis();
    lcd.clear();
    displayMenu();
} else if (btn == UI_BTN_SELECT) {

    pickCurrentApp();

    // Run the selected app; returns here when the app exits
    if (currentState == APP_BRICKS) {
      runBreakTheBricks();
    } else if (currentState == APP_COMPASS) {
      runDecisionCompass();
    } else if (currentState == APP_HELICOPTER) {
      runHelicopter();
    } else if (currentState == APP_DAYS) {
      runHowManyDays();
    } else if (currentState == APP_SNAKE) {
      runSnakeGame();
    } else if (currentState == APP_DINO) {
      runDinoFromUI();
    }

    // After an app finishes, return to MENU
    currentState = MENU;
    lcd.clear();
    delay(120);
    displayMenu();
  }

  delay(60);
}

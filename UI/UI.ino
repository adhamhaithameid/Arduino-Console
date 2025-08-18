// UI.ino
// A main file with menu navigation on an LCD Keypad Shield
// and a global idle animation after 60s of inactivity.
// We have 6 items:
//   1. Break the Bricks
//   2. Decision Compass
//   3. Hekicopter
//   4. How Many Days
//   5. Snake
//   6. Eye Animation

#include <LiquidCrystal.h>
#define EMBED_DINO_IN_UI 1

// Create the global lcd object for the Keypad Shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Dino embedding (provided by Chrome_Dino.ino when EMBED_DINO_IN_UI=1)
void dinoSetup();
void runChromeDino();

// Button definitions & thresholds
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

// Our 6 menu items
const char* menuItems[] = { 
  " Break Bricks", 
  " Decision Cps",   // shortened for 16-char line
  " Helicopter", 
  " Days Until", 
  " Snake Game",
  " Chrome Dino"
};
const int numMenuItems = 6;  // we have 6 items
int currentMenu = 0;         // which menu item is selected

// We'll define states for each app
enum AppState {
  MENU,
  APP_BRICKS,
  APP_COMPASS,
  APP_HELICOPTER,
  APP_DAYS,
  APP_SNAKE,
  APP_EYE // Eye Animation

  , APP_DINO
};
AppState currentState = MENU;

// Idle timer
unsigned long lastButtonTime = 0;
const unsigned long idleTimeout = 60000UL;  // 60 seconds
bool eyeMoveRight = true;
int eyePosition = 0;
bool isIdleMode = false;

// Forward declarations of the external "app" functions
void runBreakTheBricks();
void runDecisionCompass();
void runHelicopterGame();
void runHowManyDaysUntil();
void runSnakeGame();
void runEyeAnimation();

//-----------------------------------
// The idle animation for the MENU
//-----------------------------------
void showGlobalIdleAnimation() {
  isIdleMode = true;
  while (true) {
    if (read_LCD_buttons() != BTN_NONE) {
      // user pressed a button
      isIdleMode = false;
      lastButtonTime = millis();
      lcd.clear();
      return;  // exit idle mode
    }
    lcd.clear();
    lcd.setCursor(eyePosition, 0);
    lcd.print("o  o");
    lcd.setCursor(eyePosition + 1, 1);
    lcd.print(" ) ");
    delay(800);
    // Move the "eye" left or right
    if (eyeMoveRight) {
      eyePosition++;
      if (eyePosition >= 10) eyeMoveRight = false;
    } else {
      eyePosition--;
      if (eyePosition <= 0) eyeMoveRight = true;
    }
  }
}

//-----------------------------------
// displayMenu()
// Shows two items per 16x2 "page"
//-----------------------------------
void displayMenu() {
  lcd.clear();
  int firstItem = (currentMenu / 2) * 2;
  // top line
  lcd.setCursor(0, 0);
  lcd.print((firstItem == currentMenu) ? " >" : " ");
  lcd.print(menuItems[firstItem]);

  // second line
  if (firstItem + 1 < numMenuItems) {
    lcd.setCursor(0, 1);
    lcd.print(((firstItem + 1) == currentMenu) ? " >" : " ");
    lcd.print(menuItems[firstItem + 1]);
  }
}

//-----------------------------------
// menuNavigation()
// Loops until user presses SELECT,
// adjusting currentMenu with UP/DOWN
//-----------------------------------
void menuNavigation() {
  lastButtonTime = millis();
  while (true) {
    // check idle
    if (millis() - lastButtonTime > idleTimeout) {
      showGlobalIdleAnimation();
    }
    displayMenu();
    int btn = read_LCD_buttons();
    if (btn != BTN_NONE) {
      lastButtonTime = millis();
    }
    if (btn == BTN_UP) {
      if (currentMenu > 0) {
        currentMenu--;
        delay(200);
      }
    }
    else if (btn == BTN_DOWN) {
      if (currentMenu < numMenuItems - 1) {
        currentMenu++;
        delay(200);
      }
    }
    else if (btn == BTN_SELECT) {
      delay(200);
      break;
    }
    delay(100);
  }
  // pick the corresponding app
  switch (currentMenu) {
    case 0: currentState = APP_BRICKS;     break;
    case 1: currentState = APP_COMPASS;    break;
    case 2: currentState = APP_HELICOPTER; break;
    case 3: currentState = APP_DAYS;       break;
    case 4: currentState = APP_SNAKE;      break;
    case 5: currentState = APP_DINO;      break;
    case 6: currentState = APP_EYE;        break;
    default: currentState = MENU;          break;
  }
}


//-----------------------------------
// Dino runner wrapper: init once, then call one-iteration frames
//-----------------------------------
void runDinoFromMenu() {
  dinoSetup();                 // initialize Chrome Dino
  // drain SELECT press used to launch
  while (read_LCD_buttons() == BTN_SELECT) { delay(20); }
  unsigned long leftHoldMs = 0;
  for (;;) {
    runChromeDino();           // one iteration from the Dino sketch
    int b = read_LCD_buttons();
    if (b == BTN_LEFT) {
      if (!leftHoldMs) leftHoldMs = millis();
      if (millis() - leftHoldMs >= 800) { lcd.clear(); break; }
    } else {
      leftHoldMs = 0;
    }
    delay(8);
  }
}

//-----------------------------------
// Arduino Setup
//-----------------------------------
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  currentMenu = 0;
  currentState = MENU;
  lastButtonTime = millis();
}

//-----------------------------------
// Arduino Loop
//-----------------------------------
void loop() {
  if (currentState == MENU) {
    menuNavigation();
  }
  // Launch apps
  if (currentState == APP_BRICKS) {
    runBreakTheBricks();
  }
  else if (currentState == APP_COMPASS) {
    runDecisionCompass();
  }
  else if (currentState == APP_HELICOPTER) {
    runHelicopterGame();
  }
  else if (currentState == APP_DAYS) {
    runHowManyDaysUntil();
  }
  else if (currentState == APP_SNAKE) {
    runSnakeGame();
  }
  else if (currentState == APP_DINO) {
    runDinoFromMenu();
  }
  else if (currentState == APP_EYE) {
    // new item: Eye Animation
    runEyeAnimation();
  }
  // After an app finishes, we set state back to MENU
  // so it shows the menu again on next loop iteration.
  currentState = MENU;
  currentMenu = 0;
  lcd.clear();
  delay(200);
}

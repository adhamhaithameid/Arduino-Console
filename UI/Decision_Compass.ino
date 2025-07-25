// DecisionCompass.ino
// Single function: runDecisionCompass()
// Removes the "eye" idle animation. The user just presses SELECT to see a random response,
// and can scroll long text using LEFT and RIGHT. No exit button (reset to exit).

#include <Arduino.h>  // If needed; your UI.ino likely has #include <LiquidCrystal.h> + a global `lcd` object.

// Prototypes (if you want them at top, else they can remain below):
static void DC_showRandomResponse();
static void DC_updateDisplay();
static void DC_scrollText(int direction);

// Data
static const char* DC_responses[] = {
  "as sure as the sun rises in the east, yes.",
  "the answer you seek is yes.",
  "by the power invested in me, I say yes.",
  "the stars have aligned, yes.",
  "the signs are clear.",
  "indeed, it is possible.",
  "yes, the forces of good are with you.",
  "it would be wise to pursue this course of action.",
  "yes, but be wary of temptation and greed.",
  "ask again when the time is right.",
  "no, the path you seek is not the right one for you.",
  "this is not the way.",
  "it is not meant to be.",
  "perhaps, but there is much you still need to learn.",
  "our hopes have been misplaced.",
  "we may have to leave this decision to fate."
};

static const int DC_responseCount = sizeof(DC_responses) / sizeof(DC_responses[0]);
static String DC_currentResponse = "";
static int DC_scrollIndex = 0;
static int DC_lastResponseIndex = -1;

// If you want to match your UI.ino button thresholds, define them similarly:
static const int DC_SELECT_BUTTON = 723;  
static const int DC_LEFT_BUTTON   = 481;    
static const int DC_RIGHT_BUTTON  = 0;     
static const int DC_UP_BUTTON     = 131;      
static const int DC_DOWN_BUTTON   = 309;

// runDecisionCompass()
// This function runs indefinitely, until the user resets the board.
void runDecisionCompass()
{
  // Initialize random
  randomSeed(analogRead(A1));

  // Clear the shared lcd from UI.ino
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Decision Compass");

  // Main loop: press SELECT to pick random response, press RIGHT/LEFT to scroll
  while (true) {
    int buttonValue = analogRead(A0);

    // If user presses SELECT
    if (buttonValue > (DC_SELECT_BUTTON - 10) && buttonValue < (DC_SELECT_BUTTON + 10)) {
      DC_showRandomResponse();
      delay(500);
    }

    // If user presses RIGHT
    if (buttonValue > (DC_RIGHT_BUTTON - 10) && buttonValue < (DC_RIGHT_BUTTON + 10)) {
      DC_scrollText(1);
      delay(250);
    }

    // If user presses LEFT
    if (buttonValue > (DC_LEFT_BUTTON - 10) && buttonValue < (DC_LEFT_BUTTON + 10)) {
      DC_scrollText(-1);
      delay(250);
    }

    delay(50); // small delay to avoid spamming analog reads
  }
}

// Show a new random response (ensuring it’s different from the last one).
static void DC_showRandomResponse()
{
  lcd.clear();

  int index;
  do {
    index = random(DC_responseCount);
  } while (index == DC_lastResponseIndex);

  DC_lastResponseIndex = index;
  DC_currentResponse = DC_responses[index];
  DC_scrollIndex = 0;

  DC_updateDisplay();
}

// Update the LCD with the current response text (handles scrolling if >32 chars).
static void DC_updateDisplay()
{
  lcd.clear();
  // If the response is <= 32 chars, we can display it on two lines at once
  if (DC_currentResponse.length() <= 32) {
    lcd.setCursor(0, 0);
    lcd.print(DC_currentResponse.substring(0, 16));

    lcd.setCursor(0, 1);
    if (DC_currentResponse.length() > 16) {
      lcd.print(DC_currentResponse.substring(16));
    }
    // Wait a bit to let user read short text
    delay(2000);

  } else {
    // If longer than 32 chars, we do an initial display
    DC_scrollIndex = 0;
    lcd.setCursor(0, 0);
    lcd.print(DC_currentResponse.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(DC_currentResponse.substring(16, 32));
    delay(1500);

    // Then automatically scroll through it
    for (DC_scrollIndex = 1; DC_scrollIndex <= (int)DC_currentResponse.length() - 32; DC_scrollIndex++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(DC_currentResponse.substring(DC_scrollIndex, DC_scrollIndex + 16));
      lcd.setCursor(0, 1);
      lcd.print(DC_currentResponse.substring(DC_scrollIndex + 16, DC_scrollIndex + 32));
      delay(500);
    }
    delay(1500);
  }
}

// Manually scroll the text left or right, if the response is >32 chars
static void DC_scrollText(int direction)
{
  // Only if the response is longer than 32
  if (DC_currentResponse.length() > 32) {
    DC_scrollIndex += direction;
    if (DC_scrollIndex < 0) DC_scrollIndex = 0;
    if (DC_scrollIndex > (int)DC_currentResponse.length() - 32)
      DC_scrollIndex = DC_currentResponse.length() - 32;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(DC_currentResponse.substring(DC_scrollIndex, DC_scrollIndex + 16));
    lcd.setCursor(0, 1);
    lcd.print(DC_currentResponse.substring(DC_scrollIndex + 16, DC_scrollIndex + 32));
  }
}

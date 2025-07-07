#include <LiquidCrystal.h>

// LCD Setup (Standard for LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Button Values
#define SELECT_BUTTON 723  
#define LEFT_BUTTON 481    
#define RIGHT_BUTTON 0     
#define UP_BUTTON 131      
#define DOWN_BUTTON 309    

const char* responses[] = {
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

const int responseCount = sizeof(responses) / sizeof(responses[0]);
String currentResponse = "";
int scrollIndex = 0;
bool isIdle = true;
bool moveRight = true;
int eyePosition = 0;
int lastResponseIndex = -1; // Track last response index

void setup() {
  lcd.begin(16, 2); 
  randomSeed(analogRead(A1)); // Better entropy source than A0
  showIdleAnimation();
}

void loop() {
  int buttonValue = analogRead(A0);

  if (buttonValue > (SELECT_BUTTON - 10) && buttonValue < (SELECT_BUTTON + 10)) {
    showRandomResponse();
    delay(500);
  } 

  if (buttonValue > (RIGHT_BUTTON - 10) && buttonValue < (RIGHT_BUTTON + 10)) {
    scrollText(1);
    delay(250);
  }

  if (buttonValue > (LEFT_BUTTON - 10) && buttonValue < (LEFT_BUTTON + 10)) {
    scrollText(-1);
    delay(250);
  }

  if (isIdle) {
    showIdleAnimation();
  }
}

void showRandomResponse() {
  lcd.clear();
  
  int index;
  do {
    index = random(responseCount); // Pick a new random response
  } while (index == lastResponseIndex); // Ensure it's different from last time

  lastResponseIndex = index; // Store last response index
  currentResponse = responses[index];
  scrollIndex = 0;
  isIdle = false;
  updateDisplay();
}

void updateDisplay() {
  lcd.clear();
  if (currentResponse.length() <= 32) {
    lcd.setCursor(0, 0);
    lcd.print(currentResponse.substring(0, 16));
    lcd.setCursor(0, 1);
    if (currentResponse.length() > 16) {
      lcd.print(currentResponse.substring(16));
    }
    delay(3000);
    isIdle = true;
  } else {
    scrollIndex = 0;
    lcd.setCursor(0, 0);
    lcd.print(currentResponse.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(currentResponse.substring(16, 32));
    delay(1500);
    for (scrollIndex = 1; scrollIndex <= currentResponse.length() - 32; scrollIndex++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(currentResponse.substring(scrollIndex, scrollIndex + 16));
      lcd.setCursor(0, 1);
      lcd.print(currentResponse.substring(scrollIndex + 16, scrollIndex + 32));
      delay(500);
    }
    delay(2000);
    isIdle = true;
  }
}

void scrollText(int direction) {
  if (currentResponse.length() > 32) {
    scrollIndex += direction;
    if (scrollIndex < 0) scrollIndex = 0;
    if (scrollIndex > currentResponse.length() - 32) scrollIndex = currentResponse.length() - 32;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(currentResponse.substring(scrollIndex, scrollIndex + 16));
    lcd.setCursor(0, 1);
    lcd.print(currentResponse.substring(scrollIndex + 16, scrollIndex + 32));
  }
}

void showIdleAnimation() {
  while (isIdle) {
    lcd.clear();
    lcd.setCursor(eyePosition, 0);
    lcd.print("o  o"); 
    lcd.setCursor(eyePosition + 1, 1);
    lcd.print(" ) "); 

    delay(800);

    // **Check button press inside the loop to break immediately**
    int buttonValue = analogRead(A0);
    if (buttonValue > (SELECT_BUTTON - 10) && buttonValue < (SELECT_BUTTON + 10)) {
      isIdle = false; // Stop animation
      lcd.clear();
      lcd.print("Ask a question..");
      break;
    }

    if (moveRight) {
      eyePosition++;
      if (eyePosition >= 10) moveRight = false;
    } else {
      eyePosition--;
      if (eyePosition <= 0) moveRight = true;
    }
  }
}

#include <LiquidCrystal.h>

// LCD Setup
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Button Values
#define UP_BUTTON 131
#define DOWN_BUTTON 309

// Game variables
int heliRow = 0; // Helicopter's row position
int obstacleX[5]; // X positions of obstacles
int obstacleY[5]; // Y positions (0 = top, 1 = bottom)
bool gameOver = false;

// Custom Helicopter Character (5x8 pixels)
byte heliChar[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B01110,
  B00100,
  B01010,
  B00000
};

// Function to reset game
void resetGame() {
  heliRow = 0;
  int pos = 10; // Start placing obstacles from column 10
  for (int i = 0; i < 5; i++) {
    obstacleX[i] = pos; // Set X position
    obstacleY[i] = random(2); // Randomly choose row

    // Ensure at least one row is always clear
    if (i > 0 && obstacleY[i] == obstacleY[i - 1]) {
      obstacleY[i] = !obstacleY[i]; // Flip row to maintain gap
    }

    pos += random(2, 4); // Add spacing (2-3 columns apart)
  }
  gameOver = false;
}

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, heliChar); // Load custom helicopter character
  randomSeed(analogRead(A1)); // Randomize obstacles
  resetGame();
}

void loop() {
  if (gameOver) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(" GAME OVER ");
    lcd.setCursor(2, 1);
    lcd.print("> PRESS UP <");

    while (true) {
      int buttonValue = analogRead(A0);
      if (buttonValue > (UP_BUTTON - 10) && buttonValue < (UP_BUTTON + 10)) {
        resetGame();
        break;
      }
    }
    return;
  }

  // Read button input
  int buttonValue = analogRead(A0);
  if (buttonValue > (UP_BUTTON - 10) && buttonValue < (UP_BUTTON + 10)) {
    heliRow = 0; // Move up
  }
  if (buttonValue > (DOWN_BUTTON - 10) && buttonValue < (DOWN_BUTTON + 10)) {
    heliRow = 1; // Move down
  }

  // Move obstacles left
  for (int i = 0; i < 5; i++) {
    obstacleX[i]--;
    if (obstacleX[i] < 0) { // Reset obstacle after it leaves screen
      obstacleX[i] = obstacleX[(i + 4) % 5] + random(2, 4); // Keep spacing
      obstacleY[i] = random(2);

      // Ensure a gap is always there
      if (i > 0 && obstacleY[i] == obstacleY[i - 1]) {
        obstacleY[i] = !obstacleY[i];
      }
    }
  }

  // Check collision
  for (int i = 0; i < 5; i++) {
    if (obstacleX[i] == 1 && heliRow == obstacleY[i]) {
      gameOver = true;
    }
  }

  // Render game
  lcd.clear();
  lcd.setCursor(1, heliRow);
  lcd.write(byte(0)); // Draw helicopter

  for (int i = 0; i < 5; i++) {
    lcd.setCursor(obstacleX[i], obstacleY[i]);
    lcd.print("|"); // Draw obstacles
  }

  delay(200); // Balanced speed for difficulty
}

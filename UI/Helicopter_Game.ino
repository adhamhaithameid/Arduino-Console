void runHelicopterGame() {
  const int UP_BUTTON = 131;
  const int DOWN_BUTTON = 309;
  const int BTN_SELECT = 790;
  
  int heliRow = 0;
  int obstacleX[5];
  int obstacleY[5];
  bool gameOver = false;
  
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
  
  auto resetGame = [&]() {
    heliRow = 0;
    int pos = 10;
    for (int i = 0; i < 5; i++) {
      obstacleX[i] = pos;
      obstacleY[i] = random(2);
      if (i > 0 && obstacleY[i] == obstacleY[i - 1])
        obstacleY[i] = !obstacleY[i];
      pos += random(2, 4);
    }
    gameOver = false;
  };
  
  lcd.clear();
  lcd.createChar(0, heliChar);
  randomSeed(analogRead(A1));
  resetGame();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Helicopter  Game");
  lcd.setCursor(0, 1);
  lcd.print(" >Press SELECT< ");
  while (read_LCD_buttons() != BTN_SELECT) { delay(50); }
  
  while (true) {
    int btnVal = analogRead(A0);
    if (btnVal > (UP_BUTTON - 10) && btnVal < (UP_BUTTON + 10))
      heliRow = 0;
    if (btnVal > (DOWN_BUTTON - 10) && btnVal < (DOWN_BUTTON + 10))
      heliRow = 1;
    for (int i = 0; i < 5; i++) {
      obstacleX[i]--;
      if (obstacleX[i] < 0) {
        obstacleX[i] = obstacleX[(i + 4) % 5] + random(2, 4);
        obstacleY[i] = random(2);
        if (i > 0 && obstacleY[i] == obstacleY[i - 1])
          obstacleY[i] = !obstacleY[i];
      }
    }
    for (int i = 0; i < 5; i++) {
      if (obstacleX[i] == 1 && heliRow == obstacleY[i])
        gameOver = true;
    }
    lcd.clear();
    lcd.setCursor(1, heliRow);
    lcd.write(byte(0));
    for (int i = 0; i < 5; i++) {
      lcd.setCursor(obstacleX[i], obstacleY[i]);
      lcd.print("|");
    }
    delay(200);
    if (gameOver) {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("   GAME  OVER   ");
      lcd.setCursor(2, 1);
      lcd.print(" >Press SELECT< ");
      while (true) {
        int val = analogRead(A0);
        if (val > (BTN_SELECT - 10) && val < (BTN_SELECT + 10)) {
          resetGame();
          gameOver = false;
          break;
        }
        delay(100);
      }
    }
  }
}


// UI integration wrapper
void runHelicopter() { runHelicopterGame(); }

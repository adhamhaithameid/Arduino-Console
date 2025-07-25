// EyeAnimation.ino
// Single function: runEyeAnimation()
// This shows the "eye" animation indefinitely. 
// If you want to exit on a specific button, see the note below.

#include <Arduino.h>  // If needed; your UI.ino likely has #include <LiquidCrystal.h> + a global 'lcd' object.

// The main function called from your UI:
void runEyeAnimation() 
{
  // We track the eye position and direction
  bool eyeMoveRight = true;
  int eyePosition = 0;

  // Indefinite loop (until user resets the board or you detect a key to break):
  while(true) {
    // Clear the LCD and draw the "eye"
    lcd.clear();
    lcd.setCursor(eyePosition, 0);
    lcd.print("o  o");
    lcd.setCursor(eyePosition + 1, 1);
    lcd.print(" ) ");
    delay(800);

    // If you want a button press to exit, detect it here:
    int buttonValue = analogRead(A0);
    // Example: break on any button:
    if (buttonValue < 790) { // i.e. if SELECT or any recognized threshold
      break;  // leave runEyeAnimation(), returning to UI.ino
    }

    // Move the eye left/right
    if (eyeMoveRight) {
      eyePosition++;
      if (eyePosition >= 10) eyeMoveRight = false;
    } else {
      eyePosition--;
      if (eyePosition <= 0) eyeMoveRight = true;
    }
  }
}

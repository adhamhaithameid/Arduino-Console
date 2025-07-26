# Arduino LCD Mini-Games Collection

A collection of classic games and utilities for the Arduino Uno with a 16x2 LCD Keypad Shield.  
Navigate a menu to play games like Snake, Break the Bricks, Chrome Dino, Helicopter, and more—all on a simple LCD display!

## Features

- **Menu UI**: Select games and utilities from a simple LCD menu ([UI/UI.ino](UI/UI.ino))
- **Snake Game**: Classic snake with multiple levels ([snake_game/snake_game.ino](snake_game/snake_game.ino), [UI/snake_game.ino](UI/snake_game.ino))
- **Break the Bricks**: Arkanoid-style brick breaker ([Break_the_Bricks/Break_the_Bricks.ino](Break_the_Bricks/Break_the_Bricks.ino), [UI/Break_the_Bricks.ino](UI/Break_the_Bricks.ino))
- **Chrome Dino Game**: T-Rex runner clone ([Chrome_Dino_Game_16x2.ino](Chrome_Dino_Game_16x2.ino))
- **Helicopter Game**: Dodge obstacles as a helicopter ([Helicopter_Game/Helicopter_Game.ino](Helicopter_Game/Helicopter_Game.ino), [UI/Helicopter_Game.ino](UI/Helicopter_Game.ino))
- **Decision Compass**: Magic 8-ball style random answer generator ([Decision_Compass/Decision_Compass.ino](Decision_Compass/Decision_Compass.ino), [UI/Decision_Compass.ino](UI/Decision_Compass.ino))
- **How Many Days Until**: Date calculator for days/months/years between two dates ([How_Many_Days_Until/How_Many_Days_Until.ino](How_Many_Days_Until/How_Many_Days_Until.ino), [UI/How_Many_Days_Until.ino](UI/How_Many_Days_Until.ino))
- **Eye Animation**: Fun idle animation ([UI/EyeAnimation.ino](UI/EyeAnimation.ino))

## Hardware Requirements

- Arduino Uno (or compatible)
- 16x2 LCD Keypad Shield (DFRobot or similar)
- (Optional) Passive buzzer for sound effects (some games)

## File Structure

```
Chrome_Dino_Game_16x2.ino
Break_the_Bricks/
    Break_the_Bricks.ino
Decision_Compass/
    Decision_Compass.ino
Helicopter_Game/
    Helicopter_Game.ino
How_Many_Days_Until/
    How_Many_Days_Until.ino
snake_game/
    snake_game.ino
UI/
    Break_the_Bricks.ino
    Decision_Compass.ino
    EyeAnimation.ino
    Helicopter_Game.ino
    How_Many_Days_Until.ino
    snake_game.ino
    UI.ino
```

- The `UI/` folder contains menu-integrated versions of each game/utility.
- The root and other folders contain standalone versions.

## How to Use

1. **Menu Version:**  
   - Open [UI/UI.ino](UI/UI.ino) in the Arduino IDE.
   - Upload to your Arduino Uno with the LCD Keypad Shield attached.
   - Use the shield’s buttons to navigate and select games/utilities.

2. **Standalone Games:**  
   - Open any `.ino` file outside `UI/` (e.g., [Chrome_Dino_Game_16x2.ino](Chrome_Dino_Game_16x2.ino)).
   - Upload to play that game directly.

## Controls

- **UP/DOWN/LEFT/RIGHT/SELECT**: Navigate menus and control games (mapped to analog values in code).
- **RESET**: Return to menu or restart games.

## Credits

- Chrome Dino Game by CREA ELECTRONICA, revised by
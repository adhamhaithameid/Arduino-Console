# Arduino LCD Mini-Games Console (Menu UI + Screensaver)

A collection of classic games and utilities for the Arduino Uno with a 16x2 LCD Keypad Shield.  
Navigate a menu to play games like Snake, Break the Bricks, Chrome Dino, Helicopter, and more—all on a simple LCD display!

## 🧰 Hardware
- **Arduino Uno** (ATmega328P)
- **16×2 LCD Keypad Shield** (DFRobot-style, buttons via **A0** resistor ladder)

## 🛠 Features

- **Menu UI**: Select games and utilities from a simple LCD menu ([UI/UI.ino](UI/UI.ino))
- **Snake Game**: Classic snake with multiple levels ([snake_game/snake_game.ino](snake_game/snake_game.ino), [UI/snake_game.ino](UI/snake_game.ino))
- **Break the Bricks**: Arkanoid-style brick breaker ([Break_the_Bricks/Break_the_Bricks.ino](Break_the_Bricks/Break_the_Bricks.ino), [UI/Break_the_Bricks.ino](UI/Break_the_Bricks.ino))
- **Chrome Dino Game**: T-Rex runner clone ([Chrome_Dino/Chrome_Dino.ino](Chrome_Dino/Chrome_Dino.ino)), [UI/Chrome_Dino.ino](UI/Chrome_Dino.ino))
- **Helicopter Game**: Dodge obstacles as a helicopter ([Helicopter_Game/Helicopter_Game.ino](Helicopter_Game/Helicopter_Game.ino), [UI/Helicopter_Game.ino](UI/Helicopter_Game.ino))
- **Decision Compass**: Magic 8-ball style random answer generator ([Decision_Compass/Decision_Compass.ino](Decision_Compass/Decision_Compass.ino), [UI/Decision_Compass.ino](UI/Decision_Compass.ino))
- **How Many Days Until**: Date calculator for days/months/years between two dates ([How_Many_Days_Until/How_Many_Days_Until.ino](How_Many_Days_Until/How_Many_Days_Until.ino), [UI/How_Many_Days_Until.ino](UI/How_Many_Days_Until.ino))
- **Eye Animation**: Fun idle animation ([UI/EyeAnimation.ino](UI/EyeAnimation.ino))

## 🎮 Controls
LCD Keypad Shield buttons read through **A0**. The UI maps them to `UI_BTN_*` constants; games that need their own constants use a separate namespace (`DINO_BTN_*`). Typical actions:
- **UP/DOWN**: menu navigation
- **RIGHT/LEFT**: Game navigation
- **SELECT**: choose / start / restart
- **RST (RESET)**: commonly used as back/exit in some games or wrappers

## 🏗 Architecture
- **Single LCD instance**: `LiquidCrystal lcd(8,9,4,5,6,7)` lives in `UI.ino`. Games use `extern LiquidCrystal lcd;` when embedded.
- **Button reader**: one `read_LCD_buttons()` in `UI.ino`; games reuse via `extern` when embedded.
- **App runners**: each game exposes `void runXYZ();` which the UI calls (e.g., `runSnakeGame()`).
- **Screensaver**: `EyeAnimation` is called when `(millis() - lastActivityMs) >= 30000` while in `MENU` state.

## 🕹 Games & utilities
- **Chrome Dino**: two-cell T-Rex (CREA style), **cactus** (ground) + **bird** (top), **score-based tick speed**, fixed **Game Over** edge-case, HUD score.  
- **Snake**: body stored as a **linked list**; food spawn, self-collision, incremental speed.  
- **Break-the-Bricks**: paddle + bricks on 16×2; compact physics for character LCD.  
- **Helicopter**: side-scroller with obstacle gaps.  
- **Decision Compass**: pseudo-random choices (like a Magic-8).  
- **How Many Days Until**: date/interval utility on the LCD.  
- **Eye Animation**: used standalone and as the **idle screensaver**.

## 🗂️ File Structure

```
Break_the_Bricks/
    Break_the_Bricks.ino
Chrome_Dino/
    Chrome_Dino.ino
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
    Chrome_Dino.ino
    Decision_Compass.ino
    EyeAnimation.ino
    Helicopter_Game.ino
    How_Many_Days_Until.ino
    snake_game.ino
    UI.ino
```

## 🙌 Credits
- Chrome Dino sprite inspiration: **CREA ELECTRONICA** / **Max Imagination** (two-cell T-Rex look).  
- Thanks to the Arduino community for the classic LCD Keypad Shield patterns and LiquidCrystal library examples.

---

Made with ♥ on an Uno, tiny RAM, and lots of sprites.

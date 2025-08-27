# Arduino LCD Mini‑Games Console (Menu UI + Screensaver) — **V2**
A collection of classic games and utilities for the Arduino Uno with a 16×2 LCD Keypad Shield.  
Navigate a menu to play games like Snake, Break the Bricks, Chrome Dino, Helicopter, and more — all on a simple LCD display!

![version](https://img.shields.io/badge/Version-V2-blue)
![flash](https://img.shields.io/badge/Flash-18,498%20bytes%20(57%25)-brightgreen)
![sram](https://img.shields.io/badge/SRAM-1,653%20bytes%20(80%25)-orange)

---

## 📦 Build Footprint (V2)

- **Flash (program storage):** **18,498 bytes (57%)** of 32,256 bytes  
- **SRAM (global variables):** **1,653 bytes (80%)** of 2,048 bytes → **395 bytes free** for locals/stack

> **V1** previously consumed **~98%** of flash. V2 cuts that dramatically by de‑duplicating code, sharing a single `LiquidCrystal` + button reader, and embedding games under a unified UI.

*(Your numbers may vary slightly by IDE/toolchain version and enabled games.)*

---

## 🧰 Hardware
- **Arduino Uno** (ATmega328P)
- **16×2 LCD Keypad Shield** (DFRobot‑style, buttons via **A0** resistor ladder)

## 🛠 Features (V2)
- **Unified Menu UI** with clean app hand‑off (`runXYZ()` style) and **single** `LiquidCrystal`/button reader.
- **Eye Animation screensaver** after **30 s** idle on the menu (any key returns).
- **Namespaced buttons** to avoid macro clashes (`UI_BTN_*` in UI, `DINO_BTN_*` in Dino).
- **Embed‑friendly games** (no duplicate `setup()/loop()`; use `extern LiquidCrystal lcd;` when embedded).

## 🎮 Controls
LCD Keypad Shield buttons read through **A0**. The UI maps them to `UI_BTN_*` constants; games that need their own constants use a separate namespace (`DINO_BTN_*`). Typical actions:
- **UP/DOWN**: menu navigation
- **RIGHT/LEFT**: game navigation (varies per game)
- **SELECT**: choose / start / restart
- **RESET (RST)**: resets the board (not used for in‑game back)

## 🏗 Architecture
- **Single LCD instance**: `LiquidCrystal lcd(8,9,4,5,6,7)` lives in `UI/UI.ino`. Games use `extern LiquidCrystal lcd;` when embedded.
- **Button reader**: one `read_LCD_buttons()` in `UI/UI.ino`; games reuse via `extern`.
- **App runners**: each game exposes `void runXYZ();` (e.g., `runSnakeGame()`, `runHelicopter()`).
- **Screensaver**: `EyeAnimation` auto‑runs when `(millis() - lastActivityMs) >= 30000` while in `MENU`.
- **Dino embedding**: guarded by `#ifdef EMBED_DINO_IN_UI` → reuse UI LCD/buttons and rename entry points internally.

## 🕹 Games & utilities
- **Chrome Dino**: two‑cell T‑Rex (CREA style), **cactus** (ground) + **bird** (top), **score‑based tick speed**, fixed **Game Over**, HUD score.  
- **Snake**: body stored as a **linked list**; food spawn, self‑collision, incremental speed.  
- **Break‑the‑Bricks**: paddle + bricks on 16×2; compact physics for character LCD.  
- **Helicopter**: side‑scroller with obstacle gaps.  
- **Decision Compass**: pseudo‑random answers (Magic‑8‑ball style).  
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

## 🚀 Quick Start
1. Open **`UI/UI.ino`** in Arduino IDE.  
2. Select **Board: Arduino Uno** and the correct **Port**.  
3. **Upload**. Use **UP/DOWN** to navigate, **SELECT** to launch a game.  
4. Idle on the menu for **30 s** to see the **Eye Animation** screensaver.

## 🎥 Video Demo
- see the video demo of the project [here](https://youtu.be/lyNDa8WgvO4)

## 🧪 Build Tips & Troubleshooting
- **“Multiple libraries found for LiquidCrystal”**: Info message; the IDE picks one. Remove duplicates to silence.
- **Macro clashes (`BTN_SELECT`)**: V2 uses **`UI_BTN_*`** in the UI and **`DINO_BTN_*`** in Dino to avoid conflicts.
- **Duplicate `setup()/loop()`**: Only the UI should define them in menu builds. Standalone uploads should not include UI files.
- **SRAM tight** (~80% used): keep locals small; prefer `static const`/`PROGMEM` for sprites; wrap strings with `F("...")` to move them to flash.

## 🔁 What changed from V1 → V2
- De‑duplicated LCD/button code and removed placeholder stubs → **big flash reduction** (from **~98%** to **57%**).  
- Added **idle screensaver**, fixed **Game Over** edge‑case in Dino, and standardized input handling.  
- Made each game embed‑safe (no own `setup/loop` when running under the UI).

## 🙌 Credits
- Chrome Dino sprite inspiration: **CREA ELECTRONICA** / **Max Imagination** (two‑cell T‑Rex look).  
- Thanks to the Arduino community for the LCD Keypad Shield patterns and LiquidCrystal examples.

---

Made with ♥ on an Uno, tiny RAM, and lots of sprites.

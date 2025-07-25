// HowManyDaysUntil.ino
//
// Single function: runHowManyDaysUntil()
// - Displays an intro asking user to press SELECT,
// - Then transitions to EVENT_DATE → TODAY_DATE → RESULT
//   each time the user presses SELECT.
// - Pressing UP/DOWN changes day/month/year of the currently
//   selected field; LEFT/RIGHT move between fields
// - No in-app exit button (reset the board to exit).
//

#include <Arduino.h>  // If needed; your UI.ino likely has #include <LiquidCrystal.h> + global lcd.

// We'll store all logic in one function, plus some static helpers:

// Button thresholds (match your hardware).
static const int HMD_SELECT_BUTTON = 723;  
static const int HMD_LEFT_BUTTON   = 481;    
static const int HMD_RIGHT_BUTTON  = 0;     
static const int HMD_UP_BUTTON     = 131;      
static const int HMD_DOWN_BUTTON   = 309;    

// Local states for this app
enum HMD_AppState {
  HMD_INTRO,
  HMD_EVENT_DATE,
  HMD_TODAY_DATE,
  HMD_RESULT
};

// Modes for the final result (days, months, years)
enum HMD_ViewMode { HMD_DAYS, HMD_MONTHS, HMD_YEARS };

// We'll keep these as static so each run doesn't conflict
static HMD_AppState hmdState = HMD_INTRO;
static HMD_ViewMode viewMode = HMD_DAYS;

// Starting date variables (the user can adjust them)
static int eventYear = 2025, eventMonth = 3, eventDay = 15;
static int todayYear = 2025, todayMonth = 3, todayDay = 15;
static int selectedField = 0; // 0=day, 1=month, 2=year

// Forward declarations of helper functions:
static int HMD_daysInMonth(int month, int year);
static int HMD_daysBetween(int y1, int m1, int d1, int y2, int m2, int d2);
static void HMD_showIntro();
static void HMD_showEventDate();
static void HMD_showTodayDate();
static void HMD_showResult();
static void HMD_showDate(int day, int month, int year);
static void HMD_handleDateSelection(int &day, int &month, int &year);
static void HMD_handleResultScreen();

// The main entry point for "How Many Days Until" app:
void runHowManyDaysUntil()
{
  // Initialize local state variables
  hmdState = HMD_INTRO;
  viewMode = HMD_DAYS;
  selectedField = 0;

  // Show intro screen
  HMD_showIntro();

  // Indefinite loop, press SELECT to move through states
  while(true) {
    // Read analog from the keypad
    int buttonValue = analogRead(A0);

    if (hmdState == HMD_INTRO) {
      // Wait for SELECT to move to EVENT_DATE
      if (buttonValue > (HMD_SELECT_BUTTON - 10) && buttonValue < (HMD_SELECT_BUTTON + 10)) {
        hmdState = HMD_EVENT_DATE;
        HMD_showEventDate();
        delay(300);
      }
    }
    else if (hmdState == HMD_EVENT_DATE) {
      // The user is picking event date (day, month, year)
      HMD_handleDateSelection(eventDay, eventMonth, eventYear);
    }
    else if (hmdState == HMD_TODAY_DATE) {
      // The user is picking today's date
      HMD_handleDateSelection(todayDay, todayMonth, todayYear);
    }
    else if (hmdState == HMD_RESULT) {
      // The user sees the final result. They can press UP/DOWN to change viewMode.
      HMD_handleResultScreen();
    }

    delay(50);
  }
}

//--------------------------------------------
// Implementation of local helper functions
//--------------------------------------------

// Return the # of days in given month/year
static int HMD_daysInMonth(int month, int year)
{
  if (month == 2) { // February
    // Leap year check
    if ( (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ) return 29;
    return 28;
  }
  int monthLengths[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  return monthLengths[month - 1];
}

// Calculate difference in days between date1 and date2
static int HMD_daysBetween(int y1, int m1, int d1, int y2, int m2, int d2)
{
  long days1 = d1;
  long days2 = d2;
  // add days for months
  for (int i = 0; i < m1 - 1; i++) days1 += HMD_daysInMonth(i+1, y1);
  for (int i = 0; i < m2 - 1; i++) days2 += HMD_daysInMonth(i+1, y2);

  // add days for years
  days1 += (long) y1 * 365 + (y1 / 4) - (y1 / 100) + (y1 / 400);
  days2 += (long) y2 * 365 + (y2 / 4) - (y2 / 100) + (y2 / 400);

  return (int)(days2 - days1);
}

// Show the "Intro" screen
static void HMD_showIntro()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("How Many Days?");
  lcd.setCursor(1, 1);
  lcd.print("> Press Select <");
}

// Show "Event Date" selection screen
static void HMD_showEventDate()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Event Date:");
  HMD_showDate(eventDay, eventMonth, eventYear);
}

// Show "Today's Date" selection screen
static void HMD_showTodayDate()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Today's Date:");
  HMD_showDate(todayDay, todayMonth, todayYear);
}

// Show the final result screen (days, months, years) depending on viewMode
static void HMD_showResult()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  int totalDays = HMD_daysBetween(todayYear, todayMonth, todayDay, eventYear, eventMonth, eventDay);

  if (viewMode == HMD_DAYS) {
    lcd.print("Days Until:");
    lcd.setCursor(0, 1);
    lcd.print(totalDays);
    lcd.print(" days");
  }
  else if (viewMode == HMD_MONTHS) {
    lcd.print("Months Until:");
    lcd.setCursor(0, 1);
    lcd.print(totalDays / 30); // approximate
    lcd.print(" months");
  }
  else if (viewMode == HMD_YEARS) {
    lcd.print("Years Until:");
    lcd.setCursor(0, 1);
    lcd.print(totalDays / 365); // approximate
    lcd.print(" years");
  }
}

// Show date line with indicators on the selected field
static void HMD_showDate(int day, int month, int year)
{
  lcd.setCursor(2, 1);
  // day
  lcd.print((selectedField == 0) ? ">" : " ");
  lcd.print(day);
  lcd.print("/");

  // month
  lcd.print((selectedField == 1) ? ">" : " ");
  lcd.print(month);
  lcd.print("/");

  // year
  lcd.print((selectedField == 2) ? ">" : " ");
  lcd.print(year);
}

// Used while picking date (event or today)
static void HMD_handleDateSelection(int &day, int &month, int &year)
{
  int buttonValue = analogRead(A0);

  // Press UP => increment chosen field
  if (buttonValue > (HMD_UP_BUTTON - 10) && buttonValue < (HMD_UP_BUTTON + 10)) {
    if (selectedField == 0) {
      day++;
      if (day > HMD_daysInMonth(month, year)) day = 1;
    }
    else if (selectedField == 1) {
      month++;
      if (month > 12) month = 1;
    }
    else if (selectedField == 2) {
      year++;
    }
    delay(250);
  }

  // Press DOWN => decrement chosen field
  if (buttonValue > (HMD_DOWN_BUTTON - 10) && buttonValue < (HMD_DOWN_BUTTON + 10)) {
    if (selectedField == 0) {
      day--;
      if (day < 1) day = HMD_daysInMonth(month, year);
    }
    else if (selectedField == 1) {
      month--;
      if (month < 1) month = 12;
    }
    else if (selectedField == 2) {
      year--;
    }
    delay(250);
  }

  // Press LEFT => move selectedField left
  if (buttonValue > (HMD_LEFT_BUTTON - 10) && buttonValue < (HMD_LEFT_BUTTON + 10)) {
    selectedField = (selectedField == 0) ? 2 : selectedField - 1;
    delay(250);
  }

  // Press RIGHT => move selectedField right
  if (buttonValue > (HMD_RIGHT_BUTTON - 10) && buttonValue < (HMD_RIGHT_BUTTON + 10)) {
    selectedField = (selectedField == 2) ? 0 : selectedField + 1;
    delay(250);
  }

  // Press SELECT => move from EVENT_DATE → TODAY_DATE → RESULT
  if (buttonValue > (HMD_SELECT_BUTTON - 10) && buttonValue < (HMD_SELECT_BUTTON + 10)) {
    if (hmdState == HMD_EVENT_DATE) {
      hmdState = HMD_TODAY_DATE;
      HMD_showTodayDate();
    }
    else if (hmdState == HMD_TODAY_DATE) {
      hmdState = HMD_RESULT;
      HMD_showResult();
    }
    delay(300);
  } 
  else {
    // Keep showing the date line with indicator
    HMD_showDate(day, month, year);
  }
}

// Used while in the result screen: pressing UP toggles view mode (days → months → years)
static void HMD_handleResultScreen()
{
  int buttonValue = analogRead(A0);

  // Press UP => cycle forward in viewMode
  if (buttonValue > (HMD_UP_BUTTON - 10) && buttonValue < (HMD_UP_BUTTON + 10)) {
    if (viewMode == HMD_DAYS) viewMode = HMD_MONTHS;
    else if (viewMode == HMD_MONTHS) viewMode = HMD_YEARS;
    HMD_showResult();
    delay(250);
  }

  // Press DOWN => cycle backward in viewMode
  if (buttonValue > (HMD_DOWN_BUTTON - 10) && buttonValue < (HMD_DOWN_BUTTON + 10)) {
    if (viewMode == HMD_YEARS) viewMode = HMD_MONTHS;
    else if (viewMode == HMD_MONTHS) viewMode = HMD_DAYS;
    HMD_showResult();
    delay(250);
  }
}

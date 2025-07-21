#include <LiquidCrystal.h>

// LCD setup (Arduino LCD Keypad Shield)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Button values
#define SELECT_BUTTON 723  
#define LEFT_BUTTON 481    
#define RIGHT_BUTTON 0     
#define UP_BUTTON 131      
#define DOWN_BUTTON 309    

// App states
enum AppState { INTRO, EVENT_DATE, TODAY_DATE, RESULT };
AppState state = INTRO;

// Result view modes
enum ViewMode { DAYS, MONTHS, YEARS };
ViewMode viewMode = DAYS;

// Date variables (starts at 2025/03/15)
int eventYear = 2025, eventMonth = 3, eventDay = 15;
int todayYear = 2025, todayMonth = 3, todayDay = 15;
int selectedField = 0; // 0 = day, 1 = month, 2 = year

// Days in each month (handling leap years)
int daysInMonth(int month, int year) {
    if (month == 2) { // February
        return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    }
    int monthLengths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return monthLengths[month - 1];
}

// Function to calculate days between two dates
int daysBetween(int y1, int m1, int d1, int y2, int m2, int d2) {
    int totalDays = 0;
    
    // Convert both dates to absolute day counts
    long days1 = d1;
    long days2 = d2;
    
    // Count days in years
    for (int i = 0; i < m1 - 1; i++) days1 += daysInMonth(i + 1, y1);
    for (int i = 0; i < m2 - 1; i++) days2 += daysInMonth(i + 1, y2);
    
    // Add days for years
    days1 += y1 * 365 + (y1 / 4) - (y1 / 100) + (y1 / 400);
    days2 += y2 * 365 + (y2 / 4) - (y2 / 100) + (y2 / 400);

    totalDays = days2 - days1; // Difference

    return totalDays;
}

void setup() {
    lcd.begin(16, 2);
    showIntro();
}

void loop() {
    int buttonValue = analogRead(A0);

    if (state == INTRO) {
        if (buttonValue > (SELECT_BUTTON - 10) && buttonValue < (SELECT_BUTTON + 10)) {
            state = EVENT_DATE;
            showEventDate();
            delay(300);
        }
    }
    
    else if (state == EVENT_DATE) {
        handleDateSelection(eventDay, eventMonth, eventYear);
    }

    else if (state == TODAY_DATE) {
        handleDateSelection(todayDay, todayMonth, todayYear);
    }

    else if (state == RESULT) {
        handleResultScreen();
    }
}

// Show intro screen
void showIntro() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("How Many Days?");
    lcd.setCursor(1, 1);
    lcd.print("> Press Select <");
}

// Show event date selection screen
void showEventDate() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Event Date:");
    showDate(eventDay, eventMonth, eventYear);
}

// Show today's date selection screen
void showTodayDate() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Today's Date:");
    showDate(todayDay, todayMonth, todayYear);
}

// Show the result screen with dynamic views
void showResult() {
    lcd.clear();
    lcd.setCursor(0, 0);
    
    int totalDays = daysBetween(todayYear, todayMonth, todayDay, eventYear, eventMonth, eventDay);

    if (viewMode == DAYS) {
        lcd.print("Days Until:");
        lcd.setCursor(0, 1);
        lcd.print(totalDays);
        lcd.print(" days");
    } 
    else if (viewMode == MONTHS) {
        lcd.print("Months Until:");
        lcd.setCursor(0, 1);
        lcd.print(totalDays / 30); // Approximate conversion
        lcd.print(" months");
    } 
    else if (viewMode == YEARS) {
        lcd.print("Years Until:");
        lcd.setCursor(0, 1);
        lcd.print(totalDays / 365); // Approximate conversion
        lcd.print(" years");
    }
}

// Show date with indicator for the selected field
void showDate(int day, int month, int year) {
    lcd.setCursor(2, 1);
    lcd.print((selectedField == 0) ? ">" : " ");
    lcd.print(day);
    lcd.print("/");

    lcd.print((selectedField == 1) ? ">" : " ");
    lcd.print(month);
    lcd.print("/");

    lcd.print((selectedField == 2) ? ">" : " ");
    lcd.print(year);
}

// Handle date selection (works for both event & today date)
void handleDateSelection(int &day, int &month, int &year) {
    int buttonValue = analogRead(A0);

    if (buttonValue > (UP_BUTTON - 10) && buttonValue < (UP_BUTTON + 10)) {
        if (selectedField == 0) {
            day++;
            if (day > daysInMonth(month, year)) day = 1;
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

    if (buttonValue > (DOWN_BUTTON - 10) && buttonValue < (DOWN_BUTTON + 10)) {
        if (selectedField == 0) {
            day--;
            if (day < 1) day = daysInMonth(month, year);
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

    if (buttonValue > (LEFT_BUTTON - 10) && buttonValue < (LEFT_BUTTON + 10)) {
        selectedField = (selectedField == 0) ? 2 : selectedField - 1;
        delay(250);
    }

    if (buttonValue > (RIGHT_BUTTON - 10) && buttonValue < (RIGHT_BUTTON + 10)) {
        selectedField = (selectedField == 2) ? 0 : selectedField + 1;
        delay(250);
    }

    if (buttonValue > (SELECT_BUTTON - 10) && buttonValue < (SELECT_BUTTON + 10)) {
        if (state == EVENT_DATE) {
            state = TODAY_DATE;
            showTodayDate();
        } 
        else if (state == TODAY_DATE) {
            state = RESULT;
            showResult();
        }
        delay(300);
    } else {
        showDate(day, month, year);
    }
}

// Handle result screen view switching
void handleResultScreen() {
    int buttonValue = analogRead(A0);

    if (buttonValue > (UP_BUTTON - 10) && buttonValue < (UP_BUTTON + 10)) {
        if (viewMode == DAYS) viewMode = MONTHS;
        else if (viewMode == MONTHS) viewMode = YEARS;
        showResult();
        delay(250);
    }

    if (buttonValue > (DOWN_BUTTON - 10) && buttonValue < (DOWN_BUTTON + 10)) {
        if (viewMode == YEARS) viewMode = MONTHS;
        else if (viewMode == MONTHS) viewMode = DAYS;
        showResult();
        delay(250);
    }
}

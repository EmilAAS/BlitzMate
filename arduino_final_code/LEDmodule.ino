#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust I2C address if needed

void setupLCD() {
  lcd.init();       // Initialize LCD
  lcd.backlight();  // Turn on backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You: 00:00");
  lcd.setCursor(0, 1);
  lcd.print("Opp: 00:00");
}





void handleSerialInput() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("CLOCK:")) {
      handleClockUpdate(input);
    } 
    else if (input.length() == 4 || input.length() == 5){
      handleMove(input);
    }
  }
}

void handleMove(String input) {
    if (input.length() == 4 || input.length() == 5) {
        Serial.println("Processing move: " + input);
        enableStepper();
        processMove(input);  // Gantry executes move
        disableStepper();

        // Update reed matrix after move
        delay(300);  // Allow reed switches to stabilize
        scanMatrix();
        copyMatrix(boardState, lastBoardState);
        printMatrix();
    } 
    else {
      Serial.println("Invalid move format. Expected 4 or 5 characters like e2e4 or e7e8q");
    }
}

void handleClockUpdate(String input) {
  // Expected format: "CLOCK:<your_ms>,<opp_ms>"
  // Example: "CLOCK:123456,789012"
  input.remove(0, 6);  // Remove "CLOCK:"
  int commaIndex = input.indexOf(',');
  if (commaIndex > 0) {
    // Parse the millisecond values
    unsigned long yourMs = input.substring(0, commaIndex).toInt();
    unsigned long oppMs  = input.substring(commaIndex + 1).toInt();

    // Convert milliseconds to total seconds
    int yourSec = yourMs / 1000;
    int oppSec  = oppMs  / 1000;

    // Convert seconds to "MM:SS" strings
    String yourTime = secondsToTimeString(yourSec);
    String oppTime  = secondsToTimeString(oppSec);

    // Immediately redraw the LCD with those formatted values
    lcd.setCursor(5, 0);  // After "You: "
    lcd.print(yourTime);
    lcd.print("  ");      // Clear any leftover digits

    lcd.setCursor(5, 1);  // After "Opp: "
    lcd.print(oppTime);
    lcd.print("  ");
  } else {
    Serial.println("Invalid CLOCK format");
  }
}

// Helper function to convert total seconds into "MM:SS"
String secondsToTimeString(int totalSeconds) {
  int mins = totalSeconds / 60;
  int secs = totalSeconds % 60;
  char buffer[6];  // Format: "MM:SS"
  sprintf(buffer, "%02d:%02d", mins, secs);
  return String(buffer);
}




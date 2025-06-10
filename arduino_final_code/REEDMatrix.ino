// REEDMatrix.ino

// ---- Pin Configurations ----
const int rowPins[8] = {32, 47, 45, 43, 41, 39, 37, 35};
const int colPins[8] = {33, 31, 29, 27, 25, 23, 17, 16};
#define MOVE_BUTTON_PIN 15

// ---- State Buffers ----
bool boardState[8][8];
bool lastBoardState[8][8];

void setupReedMatrix() {
  for (int r = 0; r < 8; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }

  for (int c = 0; c < 8; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  pinMode(MOVE_BUTTON_PIN, INPUT_PULLUP);  // assumes button is connected to GND

  scanMatrix();
  copyMatrix(boardState, lastBoardState);
}

// ---- Matrix Scanning ----
void scanMatrix() {
  for (int r = 0; r < 8; r++) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(rowPins[i], HIGH);
    }
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < 8; c++) {
      int val = digitalRead(colPins[c]);
      boardState[7 - r][c] = (val == LOW); // Flip row
    }
  }
}


void printMatrix() {
  Serial.println("Board State:");
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      Serial.print(boardState[r][c] ? " X " : " . ");
    }
    Serial.println();
  }
  Serial.println("------------------------");
}

// ---- Move Detection and Serial Output ----
void detectAndSendMove() {
  int fromR = -1, fromC = -1;
  int toR = -1, toC = -1;

  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      bool prev = lastBoardState[r][c];
      bool curr = boardState[r][c];

      if (prev && !curr) {
        fromR = r;
        fromC = c;
      } else if (!prev && curr) {
        toR = r;
        toC = c;
      }
    }
  }

  if (fromR != -1 && toR != -1) {
    String move = squareToAlgebraic(fromR, fromC) + squareToAlgebraic(toR, toC);
    Serial.println(move);
  }
}

// ---- Helper Functions ----
String squareToAlgebraic(int row, int col) {
  char file = 'a' + col;
  char rank = '1' + row; // Use row directly if row 0 = rank 1
  return String(file) + String(rank);
}

void copyMatrix(bool src[8][8], bool dest[8][8]) {
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      dest[r][c] = src[r][c];
    }
  }
}

void checkButton() {
  if (digitalRead(MOVE_BUTTON_PIN) == LOW) {  // Button pressed
    delay(50);  // Debounce delay
    scanMatrix();
    detectAndSendMove();
    copyMatrix(boardState, lastBoardState);

    // Wait for button to be released before continuing
    while (digitalRead(MOVE_BUTTON_PIN) == LOW);
      delay(50);  // Final debounce
  }
}
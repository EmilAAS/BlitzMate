#include <AccelStepper.h>

#define A_ENABLE_PIN 24
#define B_ENABLE_PIN 30
#define ENDSTOP_X 3
#define ENDSTOP_Y 14 

int lastSpeedA = 0;
int lastSpeedB = 0;
long travelSpeed = 12000;

const long squareSize = 2123L;   // Distance in steps between squares 2093
const long xOrigin = 3822L;      // X steps at column 0 ('a') 3825
const long yOrigin = 16380L;     // Y steps at row 7 ('1')16200
const long halfSquare = squareSize / 2;
const long graveyardX = xOrigin + 8 * squareSize;

const long whiteQueenSupplyX = xOrigin - squareSize;
const long whiteQueenSupplyY = yOrigin -  7 * squareSize; // same row as rank 1

const long blackQueenSupplyX = xOrigin - squareSize;
const long blackQueenSupplyY = yOrigin; // same row as rank 8

bool whiteCastleDone = false;
bool blackCastleDone = false;

AccelStepper stepperA(AccelStepper::DRIVER, 26, 28);
AccelStepper stepperB(AccelStepper::DRIVER, 36, 34);

void setupMotors() {
    pinMode(A_ENABLE_PIN, OUTPUT);
    pinMode(B_ENABLE_PIN, OUTPUT);
    digitalWrite(A_ENABLE_PIN, LOW);
    digitalWrite(B_ENABLE_PIN, LOW);
}

void setWorkingSpeed(){
    stepperA.setMaxSpeed(2000);
    stepperA.setAcceleration(2000);

    stepperB.setMaxSpeed(2000);
    stepperB.setAcceleration(2000);
}

void setupEndstops() {
    pinMode(ENDSTOP_X, INPUT_PULLUP);
    pinMode(ENDSTOP_Y, INPUT_PULLUP);
}

void homeAxes() {
    Serial.println("Starting CoreXY homing sequence...");
    delay(1500);
    // Set slow homing speed
    stepperA.setMaxSpeed(12000);
    stepperB.setMaxSpeed(12000);

    // Move towards X endstop (both motors CW)
    while (digitalRead(ENDSTOP_Y) == LOW) {  
        stepperA.setSpeed(-1000);
        stepperB.setSpeed(-1000);
        stepperA.runSpeed();
        stepperB.runSpeed();
    }
    
    Serial.println("Y axis homed!");

    // Stop motors
    stepperA.setSpeed(0);
    stepperB.setSpeed(0);

    delay(500);  // Small pause

    // Move towards Y endstop (A CW, B CCW)
    while (digitalRead(ENDSTOP_X) == LOW) {  
        stepperA.setSpeed(-1000);
        stepperB.setSpeed(1000);
        stepperA.runSpeed();
        stepperB.runSpeed();
    }
    
    Serial.println("X axis homed!");

    // Stop motors
    stepperA.setSpeed(0);
    stepperB.setSpeed(0);

    // Set zero position
    stepperA.setCurrentPosition(0);
    stepperB.setCurrentPosition(0);
    Serial.println("Homing complete.");
}

void enableStepper(){
    digitalWrite(A_ENABLE_PIN, LOW);  // Disable stepper A
    digitalWrite(B_ENABLE_PIN, LOW);  // Disable stepper B
}

void disableStepper(){
    digitalWrite(A_ENABLE_PIN, HIGH);  // Disable stepper A
    digitalWrite(B_ENABLE_PIN, HIGH);  // Disable stepper B
}

void updateStepper() {
    stepperA.runSpeed();
    stepperB.runSpeed();
}

bool finishedStepper() {
    return (stepperA.distanceToGo() == 0) 
        && (stepperB.distanceToGo() == 0);
}

void moveToPosition(long x, long y) {
    // 1) Compute CoreXY targets for each motor
    long targetA = x + y;
    long targetB = y - x;

    stepperA.moveTo(targetA);
    stepperB.moveTo(targetB);

    while (!finishedStepper()) {
        long distA = stepperA.distanceToGo();
        long distB = stepperB.distanceToGo();

        if      (distA >  0) stepperA.setSpeed(+travelSpeed);
        else if (distA <  0) stepperA.setSpeed(-travelSpeed);
        else                  stepperA.setSpeed(0);

        if      (distB >  0) stepperB.setSpeed(+travelSpeed);
        else if (distB <  0) stepperB.setSpeed(-travelSpeed);
        else                  stepperB.setSpeed(0);

        updateStepper();
    }
}

void uciToIndex(const String& square, int& row, int& col) {
  char file = square.charAt(0); // 'a' to 'h'
  char rank = square.charAt(1); // '1' to '8'

  col = file - 'a';             // 'a' = 0, ..., 'h' = 7
  row = rank - '1';             // '8' = 0 (top), '1' = (bottom)
}   

void indexToSteps(int row, int col, long& x, long& y) {

  x = xOrigin + (col * squareSize);
  y = yOrigin - (row * squareSize); // invert Y (row 0 = rank 8 = top)
}

void moveCaptured(long fromX, long fromY) {
    // Step 1: Move 45 degrees into the center of the 4 squares
    long diagX = fromX + halfSquare;
    long diagY = fromY - halfSquare;
    moveToPosition(diagX, diagY);
    delay(100);
    //step 2: move to graveyard zone
    moveToPosition(graveyardX, diagY);
}

void diagonalMove(long &x, long &y, int dirX, int dirY) {
    // Move diagonally by half a square in specified direction
    x += halfSquare * dirX;
    y += halfSquare * dirY;
    moveToPosition(x, y);
}

void handleKnightMove(int fromRow, int fromCol, int toRow, int toCol, long fromX, long fromY) {
    bool isTwoUp = abs(toRow - fromRow) == 2;
    bool isTwoSide = abs(toCol - fromCol) == 2;

    int dirX = (toCol > fromCol) ? 1 : -1;
    int dirY = (toRow > fromRow) ? -1 : 1;

    long currX = fromX;
    long currY = fromY;

    moveToPosition(currX, currY);
    electromagnetOn();
    delay(100);

    if (isTwoUp) {
        // 2 up/down, 1 side
        diagonalMove(currX, currY, dirX, dirY);            // diagonal
        currY += squareSize * dirY;                        // up/down
        moveToPosition(currX, currY);
        diagonalMove(currX, currY, dirX, dirY);            // diagonal
    } else if (isTwoSide) {
        // 2 side, 1 up/down
        diagonalMove(currX, currY, dirX, dirY);            // diagonal
        currX += squareSize * dirX;                        // sideways
        moveToPosition(currX, currY);
        diagonalMove(currX, currY, dirX, dirY);            // diagonal
    }

    moveToPosition(currX, currY); // Ensure exact endpoint
    delay(300);
    electromagnetOff();
    Serial.println("Knight move completed.");
}

void moveRookAroundKing(long fromX, long fromY, long toX, long toY, int dirX, int horizontalSquares) {
    long currX = fromX;
    long currY = fromY;

    // Step 1: diagonal up-right or up-left
    diagonalMove(currX, currY, dirX, -1);

    // Step 2: move horizontally past king
    currX += squareSize * dirX * horizontalSquares;
    moveToPosition(currX, currY);

    // Step 3: diagonal down-right or down-left
    diagonalMove(currX, currY, dirX, 1);

    // Final move to exact square (to ensure alignment)
    moveToPosition(currX, currY);
    delay(300);
    electromagnetOff();
}



void handleCastle(String move, long fromX, long fromY, long toX, long toY) {
    Serial.println("Castling...");

    // Move king first
    moveToPosition(fromX, fromY);
    electromagnetOn();
    delay(100);
    moveToPosition(toX, toY);
    delay(200);
    electromagnetOff();

    // Now handle rook based on castling side
    long rookFromX, rookFromY, rookToX, rookToY;
    int rookRow = (move == "e1g1" || move == "e1c1") ? 0 : 7;

    if (move == "e1g1" || move == "e8g8") {
        // Kingside
        indexToSteps(rookRow, 7, rookFromX, rookFromY); // h1 or h8
        indexToSteps(rookRow, 5, rookToX, rookToY);     // f1 or f8
        moveToPosition(rookFromX, rookFromY);
        electromagnetOn();
        delay(100);
        moveRookAroundKing(rookFromX, rookFromY, rookToX, rookToY, -1, 1);
    } else if (move == "e1c1" || move == "e8c8") {
        // Queenside
        indexToSteps(rookRow, 0, rookFromX, rookFromY); // a1 or a8
        indexToSteps(rookRow, 3, rookToX, rookToY);     // d1 or d8
        moveToPosition(rookFromX, rookFromY);
        electromagnetOn();
        delay(100);
        moveRookAroundKing(rookFromX, rookFromY, rookToX, rookToY, 1, 2);
    }

    Serial.println("Castling complete.");
}

void handlePromotion(long fromX, long fromY, long toX, long toY, bool isWhite) {
    Serial.println("Starting promotion sequence...");

    // Step 1: Remove the pawn from the board
    moveToPosition(fromX, fromY);
    electromagnetOn();
    delay(100);
    moveCaptured(fromX, fromY);
    delay(200);
    electromagnetOff();
    delay(100);    

    // Step 2: Move to queen supply and pick up a queen
    long currX = isWhite ? whiteQueenSupplyX : blackQueenSupplyX;
    long currY = isWhite ? whiteQueenSupplyY : blackQueenSupplyY;

    moveToPosition(currX, currY);
    electromagnetOn();
    delay(100);

    // Step 3: Safe L-shaped path to promotion square
    int dirX = (toX > currX) ? 1 : -1;

    // Diagonal up-right or up-left
    diagonalMove(currX, currY, dirX, -1);

    // Horizontal move
    currX += squareSize * dirX;
    moveToPosition(currX, currY);

    // Diagonal down-right or down-left into promotion square
    diagonalMove(currX, currY, dirX, 1);
    delay(300);
    // Done – we're now on (toX, toY)
    electromagnetOff();
    Serial.println("Promotion to Queen completed.");
}

void processMove(String move) {
  String fromSquare = move.substring(0, 2);
  String toSquare   = move.substring(2, 4);

  int fromRow, fromCol, toRow, toCol;
  long fromX, fromY, toX, toY;

  uciToIndex(fromSquare, fromRow, fromCol);
  uciToIndex(toSquare, toRow, toCol);

  indexToSteps(fromRow, fromCol, fromX, fromY);
  indexToSteps(toRow, toCol, toX, toY);

  bool isCapture = boardState[toRow][toCol];
  bool isKnightMove = (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 1) || (abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 2);
  bool isPromotion = move.length() == 5;

  if (isCapture) {
    Serial.println("captured piece on destination");
    moveToPosition(toX, toY);
    electromagnetOn();
    delay(100);
    moveCaptured(toX, toY);
    electromagnetOff();
    delay(100);
  } 

  if (isKnightMove) {
    moveToPosition(fromX, fromY);
    handleKnightMove(fromRow, fromCol, toRow, toCol, fromX, fromY);
    return;
  }

  bool isWhiteCastle = (move == "e1g1" || move == "e1c1");
  bool isBlackCastle = (move == "e8g8" || move == "e8c8");

  if ((isWhiteCastle || isBlackCastle)) {
    if ((isWhiteCastle && !whiteCastleDone) || (isBlackCastle && !blackCastleDone)) {
      handleCastle(move, fromX, fromY, toX, toY);
      if (isWhiteCastle) whiteCastleDone = true;
      if (isBlackCastle) blackCastleDone = true;
      return;
    }
  }  

  if (isPromotion) {
    bool isWhite = fromRow == 6;
    handlePromotion(fromX, fromY, toX, toY, isWhite);
    return;
  }

  moveToPosition(fromX, fromY);
  electromagnetOn();
  delay(100);

  moveToPosition(toX, toY);
  delay(300);
  electromagnetOff();
  Serial.println("Move completed.");
}

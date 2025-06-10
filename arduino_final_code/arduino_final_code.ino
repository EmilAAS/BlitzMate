void setup() {
    Serial.begin(115200);
    setupLCD();
    setupMotors();
    setupEndstops();
    setupMagnet();
    setupReedMatrix();
    homeAxes();
    delay(1);
    Serial.println("Chessboard ready");

}

void loop() {
  handleSerialInput();
  checkButton();
}



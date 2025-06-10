#define ELECTROMAGNET_PIN 10  // FAN0 on RAMPS 1.4

void electromagnetOn() {
    digitalWrite(ELECTROMAGNET_PIN, HIGH);
    Serial.println("Electromagnet ON");
}

void electromagnetOff() {
    digitalWrite(ELECTROMAGNET_PIN, LOW);
    Serial.println("Electromagnet OFF");
}

void setupMagnet() {
  pinMode(ELECTROMAGNET_PIN, OUTPUT);
  electromagnetOff();  // Ensure it's off on boot
}
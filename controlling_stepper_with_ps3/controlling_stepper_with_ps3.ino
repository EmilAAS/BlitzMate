#include <AccelStepper.h>

// Define stepper motors (4 = DRIVER mode: Step & Dir)
AccelStepper stepperE0(AccelStepper::DRIVER, 26, 28);
AccelStepper stepperE1(AccelStepper::DRIVER, 36, 34);

#define E0_ENABLE_PIN 24
#define E1_ENABLE_PIN 30
#define ELECTROMAGNET_PIN 9  // FAN0 on RAMPS 1.4

int lastSpeed = 0;
int lastSpeed2 = 0;
int electromagnetState = 0; // 0 = OFF, 1 = ON

void setup() {
    Serial.begin(115200);

    // Enable stepper drivers
    pinMode(E0_ENABLE_PIN, OUTPUT);
    pinMode(E1_ENABLE_PIN, OUTPUT);
    digitalWrite(E0_ENABLE_PIN, LOW);
    digitalWrite(E1_ENABLE_PIN, LOW);

    // Set max speed and acceleration
    stepperE0.setMaxSpeed(1000);
    stepperE0.setAcceleration(500);
    stepperE1.setMaxSpeed(1000);
    stepperE1.setAcceleration(500);

    // Set electromagnet pin
    pinMode(ELECTROMAGNET_PIN, OUTPUT);
    digitalWrite(ELECTROMAGNET_PIN, LOW);  // Start with magnet OFF
}

void loop() {
    // Read serial input
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        int firstComma = data.indexOf(',');
        int secondComma = data.indexOf(',', firstComma + 1);

        if (firstComma != -1 && secondComma != -1) {  // Ensure data is valid
            lastSpeed = data.substring(0, firstComma).toInt();
            lastSpeed2 = data.substring(firstComma + 1, secondComma).toInt();
            electromagnetState = data.substring(secondComma + 1).toInt();
        }
    }

    // Map joystick values to stepper speeds
    int speedE0 = map(lastSpeed, -100, 100, -1000, 1000);
    int speedE1 = map(lastSpeed2, -100, 100, -1000, 1000);

    // Set stepper speeds
    stepperE0.setSpeed(speedE0);
    stepperE1.setSpeed(speedE1);

    // Move stepper motors smoothly
    stepperE0.runSpeed();
    stepperE1.runSpeed();

    // Control the electromagnet
    if (electromagnetState == 1) {
        digitalWrite(ELECTROMAGNET_PIN, HIGH);  // Turn ON
    } else {
        digitalWrite(ELECTROMAGNET_PIN, LOW);   // Turn OFF
    }
}


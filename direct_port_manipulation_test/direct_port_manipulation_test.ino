#include <avr/interrupt.h>

// Stepper A (CoreXY Motor A)
#define A_STEP PA4  // Pin 26
#define A_DIR  PA6  // Pin 28
#define A_ENABLE_PIN 24 // Already assigned

// Stepper B (CoreXY Motor B)
#define B_STEP PC1  // Pin 36
#define B_DIR  PC3  // Pin 34
#define B_ENABLE_PIN 30 // Already assigned

volatile bool stepAState = false;
volatile bool stepBState = false;

void setup() {
    // Set STEP and DIR pins as OUTPUTS
    DDRA |= (1 << A_STEP) | (1 << A_DIR);
    DDRC |= (1 << B_STEP) | (1 << B_DIR);

    // Enable both motors (LOW = enabled)
    pinMode(A_ENABLE_PIN, OUTPUT);
    pinMode(B_ENABLE_PIN, OUTPUT);
    digitalWrite(A_ENABLE_PIN, LOW);
    digitalWrite(B_ENABLE_PIN, LOW);

    // Set direction forward (change as needed)
    PORTA |= (1 << A_DIR);
    PORTC |= (1 << B_DIR);

    // Set up Timer1 for precise step pulses
    cli();  // Disable interrupts

    TCCR1A = 0;  // Normal mode
    TCCR1B = (1 << WGM12) | (1 << CS10);  // CTC mode, no prescaler
    OCR1A = 1000;  // Adjust this for speed (1666 Hz for 500 RPM)
    TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare interrupt

    sei();  // Enable interrupts
}

// Timer1 Interrupt - Step pulses at ~1666 Hz (500 RPM for 1.8° stepper)
ISR(TIMER1_COMPA_vect) {
    // Toggle Step Pin for A
    if (stepAState) {
        PORTA &= ~(1 << A_STEP);  // STEP LOW
    } else {
        PORTA |= (1 << A_STEP);   // STEP HIGH
    }
    stepAState = !stepAState;

    // Toggle Step Pin for B
    if (stepBState) {
        PORTC &= ~(1 << B_STEP);  // STEP LOW
    } else {
        PORTC |= (1 << B_STEP);   // STEP HIGH
    }
    stepBState = !stepBState;
}

void loop() {
    // Motors are stepping using Timer1 interrupt
}

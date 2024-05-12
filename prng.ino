#include <stdint.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

uint64_t output = 0;  // Holds the binary output value
byte sample = 0;
byte current_bit = 0;  // Track the current bit position
boolean waiting = false;  // Boolean to determine when ISR has run
int print_count = 0;  // Counter for the number of outputs printed
unsigned long startTime = 0;  // Start time of the program
unsigned long lastOutputTime = 0;  // Time when last output was generated

void setup() {
  Serial.begin(115200);
  wd_setup();  // Setting up the watchdog timer
  startTime = micros();  // Initialize the start time in microseconds
}

void loop() {
  if (waiting && print_count < 1000) {  // Adjusted from 1000 to 30
    unsigned long currentTime = micros();  // Use micros() for current time
    output = rotate(output, 1);
    output ^= (uint64_t)sample << (current_bit % 8);

    current_bit++;
    waiting = false;

    if (current_bit >= 64) {
      printBinary(output);
      output = 0;
      current_bit = 0;
      print_count++;

      unsigned long elapsedTime = currentTime - lastOutputTime;
      Serial.print("Elapsed Time for this output (microseconds): ");
      Serial.println(elapsedTime);
      lastOutputTime = currentTime;

      if (print_count == 1) {  // Setup lastOutputTime on the first print
        lastOutputTime = micros();
      }
      
      if (print_count == 1000) {  // Adjusted from 1000 to 30
        unsigned long totalTime = currentTime - startTime;
        Serial.print("Total Time (microseconds): ");
        Serial.println(totalTime);
        Serial.print("Average Time per Output (microseconds): ");
        Serial.println(totalTime / 1000);
        while(true) {}  // Stop further processing
      }
    }
  }
}

uint64_t rotate(const uint64_t val, int shift) {
  shift &= 63;
  if (shift == 0)
    return val;
  return (val << shift) | (val >> (64 - shift));
}

void wd_setup() {
  cli();
  MCUSR = 0;
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = _BV(WDIE);
  sei();
}

ISR(WDT_vect) {
  sample = TCNT1L;
  waiting = true;
}

void printBinary(uint64_t val) {
  for (int i = 63; i >= 0; i--) {
    Serial.print((int)((val >> i) & 1));
  }
  Serial.println();
}

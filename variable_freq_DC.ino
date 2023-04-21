
// Arduino uno code to sample analog input pin A0 and output PWM to pin 9
// Sampling frequency is variable from 0 to 1024Hz using a knob on pin A1
// Timing strategy is similar to zero-error pic timing by romanblack
// PWM is in fast mode with fixed high frequency and 10 bit range

// Define constants for pins and timer registers
#define ANALOG_IN A0 // analog input pin for sampling
#define ANALOG_OUT 9 // PWM output pin
#define KNOB A1 // analog input pin for knob
#define TCCR1A _SFR_IO8(0x2F) // Timer/Counter1 Control Register A
#define TCCR1B _SFR_IO8(0x30) // Timer/Counter1 Control Register B
#define OCR1A _SFR_IO16(0x2A) // Output Compare Register 1 A
#define OCR1B _SFR_IO16(0x28) // Output Compare Register 1 B
#define ICR1 _SFR_IO16(0x26) // Input Capture Register 1

// Define variables for sampling frequency and period
int freq; // sampling frequency in Hz
unsigned long period; // sampling period in microseconds

void setup() {
  // Set PWM output pin as output
  pinMode(ANALOG_OUT, OUTPUT);

  // Set timer 1 to fast PWM mode with ICR1 as top value
  TCCR1A = (1 << WGM11) | (1 << COM1A1); // clear OC1A on compare match, set at bottom
  TCCR1B = (1 << WGM13) | (1 << WGM12); // fast PWM mode, no prescaler

  // Set ICR1 to 1023 for 10 bit resolution
  ICR1 = 1023;

  // Initialize last_time with period
  static unsigned long last_time; // variable to store the last time of sampling
  last_time = micros() + period; // set last_time to current time plus period
}

void loop() {
  // Read the knob value and map it to sampling frequency range
  int knob = analogRead(KNOB); // read knob value from 0 to 1023
  freq = map(knob, 0, 1023, 0, 1024); // map knob value to frequency from 0 to 1024 Hz

  // Calculate the sampling period in microseconds
  period = 1000000UL / freq; // divide one second by frequency

  // Read the analog input value and map it to PWM duty cycle range
  int analog_in = analogRead(ANALOG_IN); // read analog input value from 0 to 1023
  int pwm = map(analog_in, 0, 1023, 0, ICR1); // map analog input value to PWM duty cycle from 0 to ICR1

  // Set the OCR1A register to the PWM value
  OCR1A = pwm;

  // Delay for the sampling period using zero-error timing algorithm
  unsigned long now = micros(); // get the current time in microseconds
  unsigned long elapsed = now - last_time; // calculate the elapsed time since last sampling
  if (elapsed >= period) { // if the elapsed time is greater than or equal to the period
    last_time += period; // add the period to the last time for zero-error correction
    return; // return from the loop function
  }
}

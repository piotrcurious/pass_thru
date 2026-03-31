
// Arduino uno code for sampling analog input pin A0 and sending PWM signals to pins 9 and 10
// The sampling frequency is variable in the range 0-1024Hz and set by a knob connected to A1 pin
// The timing strategy is similar to zero-error pic timing as used by romanblack
// The PWM mode is fast and fixed at 9-bit resolution

// Define the analog input pins
#define ANALOG_IN A0
#define KNOB_IN A1

// Define the PWM output pins
#define PWM_POS 9
#define PWM_NEG 10

// Define the PWM frequency and resolution
#define PWM_FREQ 31250 // Hz
#define PWM_RES 9 // bits

// Define the maximum sampling frequency and period
#define MAX_FREQ 1024 // Hz
#define MIN_PERIOD 976 // microseconds

void setup() {
  // Set the PWM output pins as outputs
  pinMode(PWM_POS, OUTPUT);
  pinMode(PWM_NEG, OUTPUT);

  // Set the analog input pins as inputs
  pinMode(ANALOG_IN, INPUT);
  pinMode(KNOB_IN, INPUT);

  // Set the PWM mode to fast and fixed frequency with 9-bit resolution
  // Mode 14: Fast PWM with ICR1 as TOP
  TCCR1A = (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1); // Non-inverting mode for both channels
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // Fast PWM, no prescaler
  ICR1 = (1 << PWM_RES) - 1;
}

void loop() {
  // Read the knob value and map it to the sampling frequency range
  int knob_val = analogRead(KNOB_IN);
  int freq = map(knob_val, 0, 1023, 1, MAX_FREQ);

  // Calculate the sampling period from the frequency
  unsigned long sampling_period = 1000000UL / freq;

  // Check if it is time to sample the analog input using a static variable
  static unsigned long last_sample_time = 0;
  unsigned long now = micros();

  if (last_sample_time == 0) { last_sample_time = now; if (last_sample_time == 0) last_sample_time = 1; }

  if (now - last_sample_time >= sampling_period) {
    // Update the timer variable
    last_sample_time += sampling_period;

    // Read the analog input value and map it to the PWM range
    int analog = analogRead(ANALOG_IN);
    int pwm = map(analog, 0, 1023, 0, (1 << PWM_RES) - 1);

    // Split the PWM value into positive and negative parts
    int pwm_pos = pwm > (1 << (PWM_RES - 1)) ? pwm - (1 << (PWM_RES - 1)) : 0;
    int pwm_neg = pwm < (1 << (PWM_RES - 1)) ? (1 << (PWM_RES - 1)) - pwm : 0;

    // Write the PWM values to the output pins
    OCR1A = pwm_pos;
    OCR1B = pwm_neg;
    
    // End of sampling code block
    }
}

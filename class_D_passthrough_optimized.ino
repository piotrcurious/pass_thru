
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
  TCCR1A = (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1); // Fast PWM, non-inverting mode for both channels
  TCCR1B = (1 << WGM12) | (1 << CS10); // Fast PWM, no prescaler
}

void loop() {
  // Read the knob value and map it to the sampling period range
  int knob = analogRead(KNOB_IN);
  int period = map(knob, 0, 1023, MIN_PERIOD, 1000000);

  // Check if it is time to sample the analog input using a static variable
  static unsigned int timer = micros();
  if (micros() - timer >= period) {
    // Update the timer variable
    timer += period;

    // Read the analog input value and map it to the PWM range
    int analog = analogRead(ANALOG_IN);
    int pwm = map(analog, 0, 1023, -(1 << (PWM_RES - 1)), (1 << (PWM_RES - 1)) - 1);

    // Write the PWM values to the output pins using a ternary operator
    analogWrite(PWM_POS, pwm > 0 ? pwm : 0);
    analogWrite(PWM_NEG, pwm < 0 ? -pwm : 0);
    
    // End of sampling code block
    }
}

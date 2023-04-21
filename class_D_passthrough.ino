
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

// Define a variable to store the current sampling period
unsigned int period;

// Define a variable to store the timer overflow count
unsigned int timer;

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

  // Initialize the sampling period and timer variables
  period = MIN_PERIOD;
  timer = micros();
}

void loop() {
  // Read the knob value and map it to the sampling frequency range
  int knob = analogRead(KNOB_IN);
  int freq = map(knob, 0, 1023, 0, MAX_FREQ);

  // Calculate the sampling period from the frequency
  period = constrain(1000000 / freq, MIN_PERIOD, 1000000);

  // Check if it is time to sample the analog input
  if (micros() - timer >= period) {
    // Update the timer variable
    timer += period;

    // Read the analog input value and map it to the PWM range
    int analog = analogRead(ANALOG_IN);
    int pwm = map(analog, 0, 1023, 0, (1 << PWM_RES) - 1);

    // Split the PWM value into positive and negative parts
    int pwm_pos = pwm > (1 << (PWM_RES - 1)) ? pwm - (1 << (PWM_RES - 1)) : 0;
    int pwm_neg = pwm < (1 << (PWM_RES - 1)) ? (1 << (PWM_RES - 1)) - pwm : 0;

    // Write the PWM values to the output pins
    analogWrite(PWM_POS, pwm_pos);
    analogWrite(PWM_NEG, pwm_neg);
    
    // End of sampling code block
    }
}


//Source: Conversation with Bing, 4/22/2023
//(1) Analog Input | Arduino Documentation | Arduino Documentation. https://www.arduino.cc/en/Tutorial/AnalogInput.
//(2) Analog Input | Arduino. https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInput/.
//(3) How To Use Arduino’s Analog and Digital Input/Output (I/O). https://www.allaboutcircuits.com/projects/using-the-arduinos-analog-io/.
//(4) Basics of PWM (Pulse Width Modulation) | Arduino Documentation. https://www.arduino.cc/en/Tutorial/PWM.
//(5) Secrets of Arduino PWM | Arduino Documentation. https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM.
//(6) analogWrite() - Arduino Reference. https://reference.arduino.cc/reference/en/language/functions/analog-io/analogwrite/.

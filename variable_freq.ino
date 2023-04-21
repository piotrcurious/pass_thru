
// Define the analog pins
#define INPUT_PIN A0
#define FREQ_PIN A1

// Define the PWM pins
#define POS_PIN 10
#define NEG_PIN 9

// Define the frequency range
#define MIN_FREQ 20
#define MAX_FREQ 2000

// Define the analog range
#define MIN_ANALOG 0
#define MAX_ANALOG 1023

// Define the zero point
#define ZERO_POINT 512

// Define the PWM resolution
#define PWM_RES 9

// Define the timer prescaler value (1, 8, 64, 256 or 1024)
#define PRESCALER_VALUE 64

// Define the timer prescaler bits (00, 01, 10 or 11)
#if PRESCALER_VALUE == 1
#define PRESCALER_BITS B00
#elif PRESCALER_VALUE == 8
#define PRESCALER_BITS B01
#elif PRESCALER_VALUE == 64
#define PRESCALER_BITS B10
#elif PRESCALER_VALUE == 256
#define PRESCALER_BITS B11
#elif PRESCALER_VALUE == 1024
#define PRESCALER_BITS B100
#else
#error Invalid prescaler value
#endif

// Define the timer overflow value
#define OVERFLOW 255

// Define the sampling frequency variable
volatile int sampling_freq = MIN_FREQ;

// Define the sampling period variable
volatile int sampling_period = 0;

// Define the timer counter variable
volatile int timer_count = 0;

void setup() {
  // Set the PWM pins as outputs
  pinMode(POS_PIN, OUTPUT);
  pinMode(NEG_PIN, OUTPUT);

  // Set the analog pins as inputs
  pinMode(INPUT_PIN, INPUT);
  pinMode(FREQ_PIN, INPUT);

  // Set the PWM mode to fast and resolution to PWM_RES bits
  TCCR1A = (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  ICR1 = (1 << PWM_RES) - 1;

  // Set the timer0 mode to normal and prescaler to PRESCALER_BITS
  TCCR0A = 0;
  TCCR0B = PRESCALER_BITS;

  // Enable the timer0 overflow interrupt
  TIMSK0 = (1 << TOIE0);

  // Calculate the initial sampling period
  sampling_period = F_CPU / (PRESCALER_VALUE * sampling_freq);
}

void loop() {
  // Read the frequency pin and map it to the frequency range
  int freq_value = analogRead(FREQ_PIN);
  int new_freq = map(freq_value, MIN_ANALOG, MAX_ANALOG, MIN_FREQ, MAX_FREQ);

  // Gradually shift the sampling frequency to avoid glitches
  if (new_freq > sampling_freq) {
    sampling_freq++;
    sampling_period = F_CPU / (PRESCALER_VALUE * sampling_freq);
    delay(10);
  }
  
  if (new_freq < sampling_freq) {
    sampling_freq--;
    sampling_period = F_CPU / (PRESCALER_VALUE * sampling_freq);
    delay(10);
  }
}

// Timer0 overflow interrupt service routine
ISR(TIMER0_OVF_vect) {
  
   // Increment the timer counter by OVERFLOW + 1
   timer_count += OVERFLOW + 1;

   // Check if the timer counter has reached or exceeded the sampling period
   if (timer_count >= sampling_period) {

     // Reset the timer counter
     timer_count = 0;

     // Read the input pin and subtract the zero point
     int input_value = analogRead(INPUT_PIN) - ZERO_POINT;

     // Write the input value to both PWM pins with different signs using XOR operation and bit masking
     analogWrite(POS_PIN, input_value ^ ((input_value >> (PWM_RES - 1)) & ((1 << PWM_RES) - 1)));
     analogWrite(NEG_PIN, input_value ^ (~((input_value >> (PWM_RES - 1)) & ((1 << PWM_RES) - 1))));
   }
}


// Define the analog pins
#define INPUT_PIN A0
#define FREQ_PIN A1

// Define the PWM pins
#define POS_PIN 10
#define NEG_PIN 9

// Define the frequency range
#define MIN_FREQ 20
#define MAX_FREQ 2000

// Define the timer0 prescaler values and bits
#define PRESCALER_1 1
#define PRESCALER_8 8
#define PRESCALER_64 64
#define PRESCALER_256 256
#define PRESCALER_1024 1024
#define CS00_BIT _BV(CS00)
#define CS01_BIT _BV(CS01)
#define CS02_BIT _BV(CS02)

// Define the analog input range and zero point
#define ANALOG_RANGE 1024
#define ANALOG_ZERO 512

// Define the PWM resolution and range
#define PWM_RES 9
#define PWM_RANGE 512

// Define a global variable for the sampling frequency
volatile int sampling_freq = MIN_FREQ;

// Define a global variable for the timer0 overflow count
volatile int overflow_count = 0;

// Define a global variable for the timer0 prescaler value and bits
volatile int prescaler_value = PRESCALER_64;
volatile byte prescaler_bits = CS01_BIT;

// Define a function to set the timer0 prescaler based on the sampling frequency
void set_prescaler() {
  // Calculate the number of timer0 ticks per sample
  int ticks_per_sample = F_CPU / (sampling_freq);
  
  // Choose the smallest prescaler that can fit the ticks per sample in one byte
  if (ticks_per_sample <= 256) {
    // Use prescaler /1 and set the bits accordingly
    prescaler_value = PRESCALER_1;
    prescaler_bits = CS00_BIT;
    
    } else if (ticks_per_sample <= 2048) {
    // Use prescaler /8 and set the bits accordingly
    prescaler_value = PRESCALER_8;
    prescaler_bits = CS01_BIT;
    
    } else if (ticks_per_sample <= 16384) {
    // Use prescaler /64 and set the bits accordingly
    prescaler_value = PRESCALER_64;
    prescaler_bits = CS01_BIT | CS00_BIT;
    
    } else if (ticks_per_sample <= 65536) {
    // Use prescaler /256 and set the bits accordingly
    prescaler_value = PRESCALER_256;
    prescaler_bits = CS02_BIT;
    
    } else {
    // Use prescaler /1024 and set the bits accordingly
    prescaler_value = PRESCALER_1024;
    prescaler_bits = CS02_BIT | CS00_BIT;
    
    }
  
}

// Define a function to set the timer0 overflow count based on the sampling frequency and prescaler
void set_overflow_count() {
  
   // Calculate the number of timer0 ticks per sample with the chosen prescaler 
   int ticks_per_sample = F_CPU / (prescaler_value * sampling_freq); 
   
   // Calculate the number of timer0 overflows per sample 
   int overflows_per_sample = ticks_per_sample / 256; 
   
   // Calculate the remaining ticks after the last overflow 
   int remaining_ticks = ticks_per_sample % 256; 
   
   // Set the timer0 overflow count to the complement of the remaining ticks 
   overflow_count = 256 - remaining_ticks; 
   
   // Set the timer0 counter to the overflow count 
   TCNT0 = overflow_count; 
   
}

// Define a function to read the analog input and output the PWM signals
void read_and_output() {
  
   // Read the analog input from pin A0 
   int input_value = analogRead(INPUT_PIN); 
   
   // Subtract the zero point from the input value 
   input_value -= ANALOG_ZERO; 
   
   // Determine the sign of the input value 
   bool sign = input_value >= 0; 
   
   // Output the PWM signals to pins 9 and 10 based on the sign 
   if (sign) { 
      // Output the positive value to pin 10 and zero to pin 9 
      OCR1B = input_value; 
      OCR1A = 0; 
      
      } else { 
      // Output the inverted negative value to pin 9 and zero to pin 10 
      OCR1A = -input_value; 
      OCR1B = 0; 
      
      }
}

// Define a function to read the frequency input and update the sampling frequency

void read_and_update_freq() {
  
   // Read the analog input from pin A1 
   int freq_value = analogRead(FREQ_PIN); 
   
   // Map the input value to the frequency range 
   int new_freq = map(freq_value, 0, ANALOG_RANGE, MIN_FREQ, MAX_FREQ); 
   
   // Gradually shift the sampling frequency to the new value to avoid glitches 
   if (new_freq > sampling_freq) { 
      sampling_freq++; 
      set_prescaler(); 
      set_overflow_count(); 
      delay(1); 
      
      } else if (new_freq < sampling_freq) { 
      sampling_freq--; 
      set_prescaler(); 
      set_overflow_count(); 
      delay(1); 
      
      }
}

// Define an interrupt service routine for timer0 overflow
ISR(TIMER0_OVF_vect) {
  
   // Set the timer0 counter to the overflow count again 
   TCNT0 = overflow_count; 
   
   // Call the function to read and output 
   read_and_output(); 
   
}

void setup() {
  
   // Set pins A0 and A1 as inputs 
   pinMode(INPUT_PIN, INPUT); 
   pinMode(FREQ_PIN, INPUT); 
   
   // Set pins 9 and 10 as outputs 
   pinMode(POS_PIN, OUTPUT); 
   pinMode(NEG_PIN, OUTPUT); 
   
   // Set PWM mode to fast and resolution to 9 bits 
   TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); 
   TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); 
   ICR1 = PWM_RANGE; 
   
   // Set the initial prescaler and overflow count based on the minimum frequency 
   set_prescaler(); 
   set_overflow_count(); 
   
   // Enable timer0 overflow interrupt

   TIMSK0 = _BV(TOIE0); 
   
   // Enable global interrupts 
   sei(); 
   
}

void loop() {
  
   // Call the function to read and update the frequency 
   read_and_update_freq(); 
   
}

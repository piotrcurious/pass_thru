
// This code samples analog input pin A0 and synchronously outputs the result to PWM pin 9
// The sampling frequency is variable in the range 0-1024Hz and set by using a knob connected to A1 pin
// The timing strategy used for sampling is similar to zero-error pic timing as used by romanblack
// PWM is in fast mode and fixed high frequency and 10 bit range

// Define constants for timer and PWM settings
#define TIMER_PRESCALER 64 // Timer prescaler value
#define PWM_FREQUENCY 62500 // PWM frequency in Hz
#define PWM_RESOLUTION 1024 // PWM resolution in bits

// Define variables for sampling frequency and period
volatile uint32_t samplingFrequency; // Sampling frequency in Hz
volatile uint32_t samplingPeriod; // Sampling period in timer ticks

// Define variables for zero-error timing algorithm
volatile uint32_t bres; // Bresenham variable for zero-error timing
volatile uint32_t bres_; // Bresenham constant for zero-error timing

// Define variables for analog input and output values
volatile uint16_t analogInput; // Analog input value from A0
volatile uint16_t analogOutput; // Analog output value for PWM pin 9

// Setup function
void setup() {
  // Set pin modes
  pinMode(A0, INPUT); // Set A0 as input
  pinMode(A1, INPUT); // Set A1 as input
  pinMode(9, OUTPUT); // Set pin 9 as output

  // Set timer1 for fast PWM mode with ICR1 as top value
  TCCR1A = (1 << WGM11) | (1 << COM1A1); // Clear OC1A on compare match, set at bottom
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); // Fast PWM mode, prescaler 64
  ICR1 = F_CPU / (TIMER_PRESCALER * PWM_FREQUENCY) - 1; // Set top value for PWM frequency

  // Set timer2 for normal mode with overflow interrupt enabled
  TCCR2A = 0; // Normal mode
  TCCR2B = (1 << CS22); // Prescaler 64
  TIMSK2 = (1 << TOIE2); // Enable overflow interrupt

  // Initialize sampling frequency and period variables
  samplingFrequency = map(analogRead(A1), 0, 1023, 0, 1024); // Read A1 and map it to sampling frequency range
  samplingPeriod = F_CPU / (TIMER_PRESCALER * samplingFrequency); // Calculate sampling period in timer ticks

  // Initialize bresenham variables for zero-error timing algorithm
  bres = samplingPeriod; // Set initial bresenham variable to sampling period
  bres_ = samplingPeriod; // Set bresenham constant to sampling period

}

// Loop function
void loop() {
  // Nothing to do here, everything is done by interrupts
}

// Timer2 overflow interrupt service routine
ISR(TIMER2_OVF_vect) {
  // Subtract timer overflow value from bresenham variable
  bres -= 256;

  // Check if bresenham variable is less than zero
  if (bres < 0) {
    // Sample analog input from A0 and map it to PWM resolution range
    analogInput = analogRead(A0);
    analogOutput = map(analogInput, 0, 1023, 0, PWM_RESOLUTION - 1);

    // Write analog output value to PWM pin 9 using OCR1A register
    OCR1A = analogOutput;

    // Add bresenham constant to bresenham variable for next cycle
    bres += bres_;
    
    // Update sampling frequency and period variables based on A1 input
    samplingFrequency = map(analogRead(A1), 0, 1023, 0, 1024); // Read A1 and map it to sampling frequency range
    samplingPeriod = F_CPU / (TIMER_PRESCALER * samplingFrequency); // Calculate sampling period in timer ticks
    
    // Update bresenham constant based on new sampling period
    bres_ = samplingPeriod;
    
    }
}

//Source: Conversation with Bing, 4/21/2023
//(1) Read an Analog Input with Arduino | Starting Electronics. https://startingelectronics.org/beginners/arduino-tutorial-for-beginners/read-an-analog-input-with-arduino/.
//(2) Analog Input Pins | Arduino Documentation. https://www.arduino.cc/en/Tutorial/AnalogInputPins.
//(3) Read Analog Voltage | Arduino Documentation. https://www.arduino.cc/en/Tutorial/ReadAnalogVoltage.
//(4) Secrets of Arduino PWM | Arduino Documentation. https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM.
//(5) How to use Arduino PWM Pins - The Engineering Projects. https://www.theengineeringprojects.com/2017/03/use-arduino-pwm-pins.html.
//(6) Basics of PWM (Pulse Width Modulation) | Arduino Documentation. https://www.arduino.cc/en/Tutorial/PWM.
//(7) Zero-error 1 second timing algorithm - Roman Black. http://www.romanblack.com/one_sec.htm.
//(8) High accuracy PIC timing systems - Roman Black. https://www.romanblack.com/onesec/High_Acc_Timing.htm.
//(9) ZEZJ zero error zero jitter period generation | All About Circuits. https://forum.allaboutcircuits.com/threads/zezj-zero-error-zero-jitter-period-generation.30477/.
//(10) ZEZJ zero error zero jitter period algorithm | Electronics Forum .... https://www.electro-tech-online.com/threads/zezj-zero-error-zero-jitter-period-algorithm.100299/.
//(11) video - ffmpeg transcoding H264 interlaced and keep interlacing via .... https://video.stackexchange.com/questions/19788/ffmpeg-transcoding-h264-interlaced-and-keep-interlacing-via-h264-nvenc-codec.

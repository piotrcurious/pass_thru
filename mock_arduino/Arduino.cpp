#include "Arduino.h"
#include <chrono>
#include <thread>

// Memory is used for these registers now

volatile uint8_t TCCR2A = 0;
volatile uint8_t TCCR2B = 0;
volatile uint8_t TIMSK2 = 0;
volatile uint8_t TCNT2_reg = 0;
volatile uint8_t OCR2A = 0;
volatile uint8_t OCR2B = 0;

volatile uint8_t TCCR0A = 0;
volatile uint8_t TCCR0B = 0;
volatile uint8_t TIMSK0 = 0;
volatile uint8_t TCNT0_reg = 0;

uint8_t mock_memory[1024];

uint32_t F_CPU = 16000000;

static uint64_t mock_micros = 0;
static int mock_analog_inputs[10] = {0};

void set_mock_micros(uint64_t us) {
    mock_micros = us;
}

void set_mock_analog_input(uint8_t pin, int value) {
    if (pin < 10) mock_analog_inputs[pin] = value;
}

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
int digitalRead(uint8_t pin) { return 0; }
int analogRead(uint8_t pin) {
    if (pin < 10) return mock_analog_inputs[pin];
    return 0;
}

void analogWrite(uint8_t pin, int val) {
    // For monitoring in simulation
    // printf("analogWrite(pin=%d, val=%d) at %u us\n", pin, val, mock_micros);
}

uint32_t millis(void) { return (uint32_t)(mock_micros / 1000); }
uint32_t micros(void) { return (uint32_t)mock_micros; }

void delay(uint32_t ms) { mock_micros += (uint64_t)ms * 1000; }
void delayMicroseconds(uint32_t us) { mock_micros += us; }

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

void sei() {}
void cli() {}

// For simulation control
extern "C" {
    void sim_init_memory() {
        for (int i=0; i<1024; i++) mock_memory[i] = 0;
    }
    void sim_set_micros(uint64_t us) { set_mock_micros(us); }
    void sim_set_analog(int pin, int val) { set_mock_analog_input(pin, val); }
    uint16_t sim_get_OCR1A() { return OCR1A; }
    uint16_t sim_get_OCR1B() { return OCR1B; }
    uint16_t sim_get_ICR1() { return ICR1; }
    void sim_timer0_ovf() { timer0_ovf_handler(); }
    void sim_timer2_ovf() { timer2_ovf_handler(); }
    void sim_setup() { setup(); }
    void sim_loop() { loop(); }
}

// Default weak implementations of ISRs if not defined in .ino
void timer2_ovf_handler(void) __attribute__((weak));
void timer2_ovf_handler(void) {}
void timer0_ovf_handler(void) __attribute__((weak));
void timer0_ovf_handler(void) {}

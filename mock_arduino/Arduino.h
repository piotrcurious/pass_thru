#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef uint8_t byte;
typedef uint8_t boolean;

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5

#define _BV(bit) (1 << (bit))

// Registers
extern volatile uint8_t TCCR1A;
extern volatile uint8_t TCCR1B;
extern volatile uint16_t ICR1;
extern volatile uint16_t OCR1A;
extern volatile uint16_t OCR1B;

extern volatile uint8_t TCCR2A;
extern volatile uint8_t TCCR2B;
extern volatile uint8_t TIMSK2;
extern volatile uint8_t TCNT2;
extern volatile uint8_t OCR2A;
extern volatile uint8_t OCR2B;

extern volatile uint8_t TCCR0A;
extern volatile uint8_t TCCR0B;
extern volatile uint8_t TIMSK0;
extern volatile uint8_t TCNT0;

#define WGM10 0
#define WGM11 1
#define WGM12 3
#define WGM13 4
#define COM1A1 7
#define COM1B1 5
#define CS10 0
#define CS11 1
#define CS12 2

#define WGM20 0
#define WGM21 1
#define CS20 0
#define CS21 1
#define CS22 2
#define TOIE2 0

#define CS00 0
#define CS01 1
#define CS02 2
#define TOIE0 0

#define B00 0
#define B01 1
#define B10 2
#define B11 3
#define B100 4

#define TCNT0 TCNT0_reg
#define TCNT2 TCNT2_reg
extern volatile uint8_t TCNT0_reg;
extern volatile uint8_t TCNT2_reg;

extern uint8_t mock_memory[1024];
#define _SFR_IO8(addr) (*(volatile uint8_t*)&mock_memory[addr])
#define _SFR_IO16(addr) (*(volatile uint16_t*)&mock_memory[addr])

#undef OCR1A
#undef OCR1B
#undef ICR1
#undef TCCR1A
#undef TCCR1B
#define OCR1A _SFR_IO16(0x2A)
#define OCR1B _SFR_IO16(0x28)
#define ICR1 _SFR_IO16(0x26)
#define TCCR1A _SFR_IO8(0x2F)
#define TCCR1B _SFR_IO8(0x30)

extern uint32_t F_CPU;

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);
uint32_t millis(void);
uint32_t micros(void);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);

long map(long x, long in_min, long in_max, long out_min, long out_max);
long constrain(long x, long a, long b);

void sei();
void cli();

void setup();
void loop();

#define ISR(vector) void vector(void)

#define TIMER2_OVF_vect timer2_ovf_handler
#define TIMER0_OVF_vect timer0_ovf_handler

void timer2_ovf_handler(void);
void timer0_ovf_handler(void);

#endif

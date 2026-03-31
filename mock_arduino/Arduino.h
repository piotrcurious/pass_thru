#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

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
extern uint8_t mock_memory[1024];
#define _SFR_IO8(addr) (*(volatile uint8_t*)&mock_memory[addr])
#define _SFR_IO16(addr) (*(volatile uint16_t*)&mock_memory[addr])

// Timer1
#define OCR1A _SFR_IO16(0x88)
#define OCR1B _SFR_IO16(0x8A)
#define ICR1  _SFR_IO16(0x86)
#define TCCR1A _SFR_IO8(0x80)
#define TCCR1B _SFR_IO8(0x81)
#define TCNT1 _SFR_IO16(0x84)

#define WGM10 0
#define WGM11 1
#define WGM12 3
#define WGM13 4
#define COM1A1 7
#define COM1B1 5
#define CS10 0
#define CS11 1
#define CS12 2

// Timer0
#define TCCR0A _SFR_IO8(0x44)
#define TCCR0B _SFR_IO8(0x45)
#define TCNT0  _SFR_IO8(0x46)
#define OCR0A  _SFR_IO8(0x47)
#define OCR0B  _SFR_IO8(0x48)
#define TIMSK0 _SFR_IO8(0x6E)

#define CS00 0
#define CS01 1
#define CS02 2
#define WGM00 0
#define WGM01 1
#define WGM02 3
#define TOIE0 0

// Timer2
#define TCCR2A _SFR_IO8(0xB0)
#define TCCR2B _SFR_IO8(0xB1)
#define TCNT2  _SFR_IO8(0xB2)
#define OCR2A  _SFR_IO8(0xB3)
#define OCR2B  _SFR_IO8(0xB4)
#define TIMSK2 _SFR_IO8(0x70)

#define CS20 0
#define CS21 1
#define CS22 2
#define WGM20 0
#define WGM21 1
#define WGM22 3
#define TOIE2 0

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
void timer2_ovf_handler(void);
void timer0_ovf_handler(void);

#ifdef __cplusplus
}
#endif

#define ISR(vector) extern "C" void vector(void)

#define TIMER2_OVF_vect timer2_ovf_handler
#define TIMER0_OVF_vect timer0_ovf_handler

#endif

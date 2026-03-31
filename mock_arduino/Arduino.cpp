#include "Arduino.h"
#include <vector>

uint8_t mock_memory[1024];
uint32_t F_CPU = 16000000;

static uint64_t mock_micros = 0;
static int mock_analog_inputs[16] = {0};

static uint64_t timer0_accum = 0;
static uint64_t timer2_accum = 0;

static uint32_t get_prescaler(uint8_t tccrb, bool is_timer2) {
    uint8_t cs = tccrb & 0x07;
    if (is_timer2) {
        switch (cs) {
            case 1: return 1;
            case 2: return 8;
            case 3: return 32;
            case 4: return 64;
            case 5: return 128;
            case 6: return 256;
            case 7: return 1024;
            default: return 0;
        }
    } else {
        switch (cs) {
            case 1: return 1;
            case 2: return 8;
            case 3: return 64;
            case 4: return 256;
            case 5: return 1024;
            default: return 0;
        }
    }
}

void update_timers(uint64_t delta_cycles) {
    if (delta_cycles == 0) return;
    // Timer0
    uint32_t ps0 = get_prescaler(TCCR0B, false);
    if (ps0 > 0) {
        timer0_accum += delta_cycles;
        uint64_t ticks = timer0_accum / ps0;
        timer0_accum %= ps0;
        while (ticks > 0) {
            uint32_t to_ovf = 256 - TCNT0;
            if (ticks >= to_ovf) {
                TCNT0 = 0;
                ticks -= to_ovf;
                if (TIMSK0 & (1 << TOIE0)) timer0_ovf_handler();
            } else {
                TCNT0 += (uint8_t)ticks;
                ticks = 0;
            }
        }
    }

    // Timer2
    uint32_t ps2 = get_prescaler(TCCR2B, true);
    if (ps2 > 0) {
        timer2_accum += delta_cycles;
        uint64_t ticks = timer2_accum / ps2;
        timer2_accum %= ps2;
        while (ticks > 0) {
            uint32_t to_ovf = 256 - TCNT2;
            if (ticks >= to_ovf) {
                TCNT2 = 0;
                ticks -= to_ovf;
                if (TIMSK2 & (1 << TOIE2)) timer2_ovf_handler();
            } else {
                TCNT2 += (uint8_t)ticks;
                ticks = 0;
            }
        }
    }
}

extern "C" {

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
int digitalRead(uint8_t pin) { return 0; }
int analogRead(uint8_t pin) {
    return mock_analog_inputs[pin % 16];
}
void analogWrite(uint8_t pin, int val) {}

uint32_t millis(void) { return (uint32_t)(mock_micros / 1000); }
uint32_t micros(void) { return (uint32_t)mock_micros; }

void delay(uint32_t ms) {
    update_timers((uint64_t)ms * 16000);
    mock_micros += (uint64_t)ms * 1000;
}

void delayMicroseconds(uint32_t us) {
    update_timers((uint64_t)us * 16);
    mock_micros += us;
}

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

void sim_init_memory() {
    for (int i=0; i<1024; i++) mock_memory[i] = 0;
    mock_micros = 0;
    timer0_accum = 0;
    timer2_accum = 0;
    TCNT0 = 0;
    TCNT2 = 0;
}

struct SimResult {
    double t;
    uint16_t ocr1a;
    uint16_t ocr1b;
    uint16_t icr1;
};

int sim_run(uint64_t duration_us, uint64_t dt_us, double freq, int knob_val, SimResult* results, int max_results) {
    sim_init_memory();
    setup();

    int count = 0;
    uint64_t target_t_us = 0;
    while (target_t_us < duration_us && count < max_results) {
        target_t_us += dt_us;

        if (target_t_us > mock_micros) {
            uint64_t catchup = target_t_us - mock_micros;
            update_timers(catchup * 16);
            mock_micros = target_t_us;
        } else {
            target_t_us = mock_micros;
        }

        mock_analog_inputs[0] = (int)(512 + 511 * sin(2 * M_PI * freq * mock_micros / 1e6));
        mock_analog_inputs[1] = knob_val;

        loop();

        results[count].t = (double)mock_micros;
        results[count].ocr1a = OCR1A;
        results[count].ocr1b = OCR1B;
        results[count].icr1 = ICR1;
        count++;
    }
    return count;
}

void timer2_ovf_handler(void) __attribute__((weak));
void timer2_ovf_handler(void) {}
void timer0_ovf_handler(void) __attribute__((weak));
void timer0_ovf_handler(void) {}

}

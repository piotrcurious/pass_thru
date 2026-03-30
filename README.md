# pass_thru
simple analog pass thru for Arduino with selectable sampling freq.

## Implementations
This repository contains several implementations of an analog pass-thru system for Arduino Uno (ATmega328P), ranging from simple single-ended PWM to differential Class D outputs with variable sampling frequencies.

- `simple_pass_thru.ino`: Basic implementation using Timer2 for zero-jitter timing.
- `class_D_passthrough.ino`: Differential output (pins 9 and 10) for driving a Class D stage.
- `class_D_passthrough_optimized.ino`: Optimized version of the Class D implementation with direct register access.
- `class_D_passthrough_optimized3.ino`: Advanced Class D implementation with frequency-dependent prescaler adjustment.
- `variable_freq.ino`: Implements variable sampling frequency (20Hz - 2kHz) with smooth transitions.
- `variable_freq_DC.ino`: Variable frequency implementation optimized for DC or slow-moving signals.

## Performance Analysis
The following graph shows the transfer functions of the different implementations, calculated using a custom mock Arduino environment and Python-based hardware simulation system.

![Transfer Functions](transfer_function.png)

*Note: The gains are normalized to the 10-bit input range. Dips in the response for some implementations (like `simple_pass_thru.ino`) are related to the interaction between the sampling frequency and the simulation's fixed interrupt timing.*

## Simulation Environment
To test the Arduino code without hardware, a mock environment is provided:
- `mock_arduino/`: C++ implementation of Arduino core functions and memory-mapped registers.
- `simulator.py`: Python script that compiles the `.ino` files using `g++` and runs signal analysis using `numpy` and `matplotlib`.

### Running the simulation
1. Install dependencies:
   ```bash
   pip install numpy matplotlib
   ```
2. Run the simulator:
   ```bash
   python3 simulator.py
   ```
   This will generate `transfer_function.png` and output test results to the console.

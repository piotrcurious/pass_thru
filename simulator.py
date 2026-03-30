import ctypes
import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt

class ArduinoSimulator:
    def __init__(self, ino_path):
        self.ino_path = ino_path
        self.lib_path = f"/tmp/{os.path.basename(ino_path)}.so"
        self._compile()
        self.lib = ctypes.CDLL(self.lib_path)

        self.sim_setup = self.lib.sim_setup
        self.sim_loop = self.lib.sim_loop

        self.sim_set_micros = self.lib.sim_set_micros
        self.sim_set_micros.argtypes = [ctypes.c_uint64]
        self.sim_set_analog = self.lib.sim_set_analog
        self.sim_get_OCR1A = self.lib.sim_get_OCR1A
        self.sim_get_OCR1B = self.lib.sim_get_OCR1B
        self.sim_get_ICR1 = self.lib.sim_get_ICR1
        self.sim_init_memory = self.lib.sim_init_memory
        self.sim_timer0_ovf = self.lib.sim_timer0_ovf
        self.sim_timer2_ovf = self.lib.sim_timer2_ovf

    def _compile(self):
        # Create a temporary C++ file from .ino
        with open(self.ino_path, 'r') as f:
            ino_content = f.read()

        cpp_path = f"/tmp/{os.path.basename(self.ino_path)}.cpp"
        with open(cpp_path, 'w') as f:
            f.write('#include "mock_arduino/Arduino.h"\n')
            f.write(ino_content)

        # Compile to shared library
        subprocess.run([
            "g++", "-shared", "-fPIC", "-o", self.lib_path,
            cpp_path, "mock_arduino/Arduino.cpp", "-I."
        ], check=True)

        # Clean up temporary CPP file
        os.remove(cpp_path)

    def __del__(self):
        if hasattr(self, 'lib_path') and os.path.exists(self.lib_path):
            # We can't easily unload the library in Python to delete it on Linux
            # but we can try
            try:
                os.remove(self.lib_path)
            except:
                pass

    def run_simulation(self, duration_us, dt_us=1, input_freq=1000, knob_val=512):
        time_points = np.arange(0, duration_us, dt_us)
        ocr1a_values = []
        ocr1b_values = []
        icr1_values = []

        self.sim_init_memory()
        # Set some reasonable defaults for registers before setup if needed
        self.sim_setup()

        # Determine which timer interrupts are enabled
        # This is a bit simplified, but let's try to call them more frequently
        for t_idx, t in enumerate(time_points):
            t_int = int(t)
            self.sim_set_micros(t_int)

            # Input signal (sine wave on A0)
            input_signal = int(512 + 511 * np.sin(2 * np.pi * input_freq * t / 1e6))
            self.sim_set_analog(0, input_signal) # A0
            self.sim_set_analog(1, knob_val) # A1

            # Simulate timer overflows
            # Timer2 and Timer0 often use 64 prescaler -> 1024us per overflow
            # We call them every 1024 us.
            if t_int % 1024 == 0:
                 self.sim_timer2_ovf()
            if t_int % 256 == 0:
                 self.sim_timer0_ovf()

            # Additional calls for files that might expect more frequent interrupts
            # simple_pass_thru bres -= 256 every interrupt.
            # If we want 512Hz sampling, we need bres to hit 0 every 1/512 s = 1953 us.
            # 1953 / 1024 ~ 2 interrupts per sample.

            # Call loop frequently
            self.sim_loop()

            ocr1a_values.append(self.sim_get_OCR1A())
            ocr1b_values.append(self.sim_get_OCR1B())
            icr1_values.append(self.sim_get_ICR1())

        return time_points, ocr1a_values, ocr1b_values, icr1_values

def run_transfer_function(filename, knob_val=512):
    print(f"Calculating Transfer Function for {filename}...")
    sim = ArduinoSimulator(filename)

    frequencies = np.logspace(1, 3.5, 20) # 10Hz to ~3.16kHz
    gains = []

    for freq in frequencies:
        # Run simulation for enough cycles
        duration = int(max(20000, 2e6 / freq))
        t, o1a, o1b, icr1 = sim.run_simulation(duration, input_freq=freq, knob_val=knob_val)

        # Calculate output signal (differential for Class D, or single ended)
        # We'll use OCR1A - OCR1B as a proxy for the output voltage
        output = np.array(o1a) - np.array(o1b)

        # Measure amplitude (RMS or peak-to-peak)
        # Skip initial transients
        skip = len(output) // 2
        if len(output[skip:]) == 0:
            gains.append(-100)
            continue

        peak_to_peak = np.ptp(output[skip:])
        if peak_to_peak == 0:
            gains.append(-100)
        else:
            gain_db = 20 * np.log10(peak_to_peak / 1024.0) # Normalized to 10-bit input
            gains.append(gain_db)

    return frequencies, gains

def test_file(filename, knob_val=512):
    print(f"Testing {filename}...")
    sim = ArduinoSimulator(filename)
    t, o1a, o1b, icr1 = sim.run_simulation(20000, knob_val=knob_val) # 20ms
    print(f"  ICR1: {icr1[0]}")
    print(f"  OCR1A (first 20): {o1a[:20]}")
    print(f"  OCR1A max: {max(o1a)}")

if __name__ == "__main__":
    files = [
        "simple_pass_thru.ino",
        "class_D_passthrough.ino",
        "class_D_passthrough_optimized.ino",
        "class_D_passthrough_optimized3.ino",
        "variable_freq.ino",
        "variable_freq_DC.ino"
    ]
    plt.figure(figsize=(10, 6))
    for f in files:
        try:
            test_file(f)
            freqs, gains = run_transfer_function(f)
            plt.semilogx(freqs, gains, label=f)
        except Exception as e:
            print(f"Failed to process {f}: {e}")

    plt.grid(True, which="both", ls="-", alpha=0.5)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Gain (dB)")
    plt.title("Transfer Functions of Arduino Pass-Thru Implementations")
    plt.legend()
    plt.ylim([-40, 5])
    plt.savefig("transfer_function.png")
    print("Saved transfer_function.png")

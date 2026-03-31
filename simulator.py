import ctypes
import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt

class SimResult(ctypes.Structure):
    _fields_ = [
        ("t", ctypes.c_double),
        ("ocr1a", ctypes.c_uint16),
        ("ocr1b", ctypes.c_uint16),
        ("icr1", ctypes.c_uint16),
    ]

class ArduinoSimulator:
    def __init__(self, ino_path):
        self.ino_path = ino_path
        self.lib_path = f"/tmp/{os.path.basename(ino_path)}.so"
        self._compile()
        self.lib = ctypes.CDLL(self.lib_path)

        self.sim_run = self.lib.sim_run
        self.sim_run.argtypes = [
            ctypes.c_uint64, # duration_us
            ctypes.c_uint64, # dt_us
            ctypes.c_double, # freq
            ctypes.c_int,    # knob_val
            ctypes.POINTER(SimResult),
            ctypes.c_int     # max_results
        ]
        self.sim_run.restype = ctypes.c_int

    def _compile(self):
        with open(self.ino_path, 'r') as f:
            ino_content = f.read()

        cpp_path = f"/tmp/{os.path.basename(self.ino_path)}.cpp"
        with open(cpp_path, 'w') as f:
            f.write('#include "mock_arduino/Arduino.h"\n')
            f.write(ino_content)

        subprocess.run([
            "g++", "-shared", "-fPIC", "-o", self.lib_path,
            cpp_path, "mock_arduino/Arduino.cpp", "-I."
        ], check=True)
        os.remove(cpp_path)

    def run_simulation(self, duration_us, dt_us=100, input_freq=1000, knob_val=1023):
        max_results = int(duration_us // dt_us) + 1
        results = (SimResult * max_results)()

        count = self.sim_run(duration_us, dt_us, input_freq, knob_val, results, max_results)

        t = np.array([results[i].t for i in range(count)])
        o1a = np.array([results[i].ocr1a for i in range(count)])
        o1b = np.array([results[i].ocr1b for i in range(count)])
        icr1 = np.array([results[i].icr1 for i in range(count)])

        return t, o1a, o1b, icr1

def run_transfer_function(filename, knob_val=1023):
    print(f"Calculating Transfer Function for {filename}...")
    sim = ArduinoSimulator(filename)

    # Sweep up to 3kHz
    frequencies = np.logspace(1, 3.4, 10)
    gains = []

    for freq in frequencies:
        # Run for 4 cycles
        duration = int(max(400000, 8e6 / freq))
        t, o1a, o1b, icr1 = sim.run_simulation(duration, dt_us=500, input_freq=freq, knob_val=1023)

        # Differential for Class D
        output = o1a.astype(float) - o1b.astype(float)

        # Skip initial transients
        skip = len(output) // 2
        sig = output[skip:]
        t_sig = t[skip:]

        if len(sig) < 10:
            gains.append(-100)
            continue

        ref_cos = np.cos(2 * np.pi * freq * t_sig / 1e6)
        ref_sin = np.sin(2 * np.pi * freq * t_sig / 1e6)

        a = np.mean(sig * ref_cos) * 2
        b = np.mean(sig * ref_sin) * 2
        amplitude = np.sqrt(a**2 + b**2)

        input_amplitude = 511.0

        if amplitude <= 1e-3:
            gains.append(-100)
        else:
            gains.append(20 * np.log10(amplitude / input_amplitude))

    return frequencies, gains

if __name__ == "__main__":
    files = [
        "simple_pass_thru.ino",
        "class_D_passthrough_optimized.ino",
        "variable_freq.ino",
    ]
    plt.figure(figsize=(10, 6))
    for f in files:
        try:
            freqs, gains = run_transfer_function(f, knob_val=1023)
            plt.semilogx(freqs, gains, label=f, marker='o', markersize=4)
        except Exception as e:
            print(f"Failed to process {f}: {e}")

    plt.grid(True, which="both", ls="-", alpha=0.5)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Gain (dB)")
    plt.title("Arduino Pass-Thru Frequency Response (Sampling @ ~1kHz)")
    plt.legend()
    plt.ylim([-40, 5])
    plt.savefig("transfer_function.png")
    print("Saved transfer_function.png")

import serial
import matplotlib.pyplot as plt
import numpy as np

PORT       = '/dev/tty.usbmodem14201'      # Change to your port (e.g. '/dev/ttyACM0' on Linux/Mac)
BAUD       = 115200
N_SAMPLES  = 400         # Number of samples to collect (~5s at 80Hz)

# IIR filter parameters (must match Pico code)
IIR_A = 0.85
IIR_B = 1.0 - IIR_A

print(f"Opening {PORT} at {BAUD} baud...")
ser = serial.Serial(PORT, BAUD, timeout=10)

# Send number of samples
ser.write(f"{N_SAMPLES}\n".encode())
print(f"Requested {N_SAMPLES} samples, waiting...")

# Read data back
t_ms   = []
raw    = []
filt   = []

for _ in range(N_SAMPLES):
    line = ser.readline().decode().strip()
    if not line:
        continue
    parts = line.split(',')
    if len(parts) != 3:
        print(f"Skipping malformed line: {line}")
        continue
    t_ms.append(float(parts[0]))
    raw.append(float(parts[1]))
    filt.append(float(parts[2]))

ser.close()
print(f"Received {len(raw)} samples.")

# Convert time to seconds, starting at 0
t = np.array(t_ms) / 1000.0
t = t - t[0]
raw  = np.array(raw)
filt = np.array(filt)

Fs = len(t) / (t[-1] - t[0])
print(f"Measured sample rate: {Fs:.1f} Hz  (Nyquist: {Fs/2:.1f} Hz)")

def compute_fft(signal, fs):
    n   = len(signal)
    Y   = np.fft.fft(signal) / n
    frq = np.fft.fftfreq(n, d=1.0/fs)
    # one-sided
    half = int(n / 2)
    return frq[:half], np.abs(Y[:half])

frq_raw,  mag_raw  = compute_fft(raw,  Fs)
frq_filt, mag_filt = compute_fft(filt, Fs)

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7))
fig.suptitle(f'HX711 Force Sensor  —  {N_SAMPLES} samples @ ~{Fs:.0f} Hz\n'
             f'IIR filter: A={IIR_A}, B={IIR_B:.2f}')

# Time domain
ax1.plot(t, raw,  'k',  linewidth=0.7, label='Raw')
ax1.plot(t, filt, 'r',  linewidth=1.2, label=f'IIR filtered (A={IIR_A})')
ax1.set_xlabel('Time [s]')
ax1.set_ylabel('ADC counts')
ax1.legend()
ax1.grid(True, alpha=0.3)

# Frequency domain (log-log)
ax2.loglog(frq_raw[1:],  mag_raw[1:],  'k',  linewidth=0.7, label='Raw FFT')
ax2.loglog(frq_filt[1:], mag_filt[1:], 'r',  linewidth=1.2, label=f'Filtered FFT')
ax2.axvline(x=Fs/2, color='b', linestyle='--', linewidth=0.8, label=f'Nyquist ({Fs/2:.0f} Hz)')
ax2.set_xlabel('Frequency [Hz]')
ax2.set_ylabel('|Y(f)|')
ax2.legend()
ax2.grid(True, which='both', alpha=0.3)

plt.tight_layout()
plt.savefig('HW14_force_sensor.png', dpi=150)
print("Plot saved as HW14_force_sensor.png")
plt.show()
import serial
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Circle

# ---------------- Config ----------------
PORT = '/dev/tty.usbmodem1101'   # update to your port
BAUD = 115200

# ---------------- Serial setup ----------------
ser = serial.Serial(PORT, BAUD, timeout=1)

# Auto-calibrate force baseline from first samples
print("Calibrating baseline (don't touch the sensor)...")
baseline_samples = []
while len(baseline_samples) < 50:
    line = ser.readline().decode().strip()
    parts = line.split(',')
    if len(parts) == 4:
        baseline_samples.append(float(parts[3]))  # filtered force
force_baseline = np.mean(baseline_samples)
print(f"Baseline force: {force_baseline:.0f}")

# Scale: how much force change maps to "full" visual response.
# Tune this if the color/size saturates too fast or too slow.
FORCE_SCALE = 200000.0

# ---------------- Plot setup ----------------
fig, ax = plt.subplots(figsize=(6, 6))
ax.set_xlim(-1.5, 1.5)
ax.set_ylim(-1.5, 1.5)
ax.set_aspect('equal')
ax.axis('off')
ax.set_title('HW17 - Force & Position', fontsize=14)

# The dial circle (size + color = force)
circle = Circle((0, 0), 0.5, color='green', alpha=0.6)
ax.add_patch(circle)

# The angle indicator line
line_indicator, = ax.plot([0, 0], [0, 1], 'k-', linewidth=4)

# Text readout
text = ax.text(0, -1.3, '', ha='center', fontsize=11)

# ---------------- Update function ----------------
def update(frame):
    # Read the most recent line available
    raw_line = None
    while ser.in_waiting:
        raw_line = ser.readline().decode().strip()

    if raw_line is None:
        return circle, line_indicator, text

    parts = raw_line.split(',')
    if len(parts) != 4:
        return circle, line_indicator, text

    try:
        angle_raw = int(parts[1])
        force = float(parts[3])
    except ValueError:
        return circle, line_indicator, text

    # --- Angle -> rotation of indicator line ---
    angle_deg = angle_raw * 360.0 / 4096.0
    angle_rad = np.radians(angle_deg)
    line_indicator.set_data([0, np.sin(angle_rad)], [0, np.cos(angle_rad)])

    # --- Force -> color + size of circle ---
    force_delta = abs(force - force_baseline)
    intensity = min(force_delta / FORCE_SCALE, 1.0)  # 0 to 1

    # Color: green (low) -> red (high)
    circle.set_color((intensity, 1.0 - intensity, 0.0))
    # Size: grow with force
    circle.set_radius(0.4 + 0.4 * intensity)

    text.set_text(f'Angle: {angle_deg:.0f}    Force: {force - force_baseline:+.0f}')

    return circle, line_indicator, text

# ---------------- Run ----------------
ani = animation.FuncAnimation(fig, update, interval=30, blit=True)
plt.show()

ser.close()
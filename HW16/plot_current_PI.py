import serial
import numpy as np
import matplotlib.pyplot as plt
from time import sleep

PORT = '/dev/tty.usbmodem102'  # update if needed
BAUD = 115200
N = 400

ser = serial.Serial(PORT, BAUD, timeout=10)
sleep(2)
ser.write(b'a')

indices = []
desired = []
actual = []

for _ in range(N):
    line = ser.readline().decode().strip()
    print(repr(line))  # debug
    parts = line.split()
    if len(parts) == 3:
        indices.append(int(parts[0]))
        desired.append(float(parts[1]))
        actual.append(float(parts[2]))

ser.close()

t = np.array(indices)
des = np.array(desired)
act = np.array(actual)

plt.figure(figsize=(10, 4))
plt.plot(t, des, label='Desired Current (mA)', linewidth=2)
plt.plot(t, act, label='Actual Current (mA)', alpha=0.8)
plt.xlabel('Sample')
plt.ylabel('Current (mA)')
plt.title('PI Current Controller')
plt.legend()
plt.tight_layout()
plt.savefig('pi_controller.png', dpi=150)
plt.show()
import machine
import time

i2c = machine.I2C(0, sda=machine.Pin(4), scl=machine.Pin(5), freq=400000)
MPU = 0x68

i2c.writeto_mem(MPU, 0x6B, bytes([0]))
button = machine.Pin(15, machine.Pin.IN, machine.Pin.PULL_UP)

def read_raw_accel(reg):
    data = i2c.readfrom_mem(MPU, reg, 2)
    val = (data[0] << 8) | data[1]
    if val > 32767:
        val -= 65536
    return val

DEBOUNCE_MS = 50
last_btn_time = 0
btn_state = 1

while True:
    ax = read_raw_accel(0x3B)
    ay = read_raw_accel(0x3D)
    raw = 0 if button.value() == 0 else 1
    now = time.ticks_ms()
    if raw != btn_state and time.ticks_diff(now, last_btn_time) > DEBOUNCE_MS:
        btn_state = raw
        last_btn_time = now
    print(f"{ax},{ay},{btn_state}")
    time.sleep_ms(20)
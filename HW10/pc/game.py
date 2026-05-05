import pgzrun
import random
import serial
import threading

# Serial setup
ser = serial.Serial('/dev/tty.usbmodem101', 115200, timeout=1)
print('Opening port: ' + str(ser.name))

# Shared state from Pico — updated by background thread
pico_ay = 0
pico_btn = 1

def read_serial():
    global pico_ay, pico_btn
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            parts = line.split(',')
            if len(parts) == 3:
                pico_ay = int(parts[1])
                pico_btn = int(parts[2])
        except:
            pass

threading.Thread(target=read_serial, daemon=True).start()

# Window
WIDTH = 800
HEIGHT = 600
TITLE = "Space Invaders"

# Ship
SHIP_Y = 540
SHIP_SPEED = 6
ship_x = WIDTH // 2

# Bullets
bullets = []
BULLET_SPEED = 8

# Enemies
ENEMY_ROWS = 3
ENEMY_COLS = 8
ENEMY_SPACING_X = 80
ENEMY_SPACING_Y = 60
ENEMY_START_X = 100
ENEMY_START_Y = 80
ENEMY_SPEED = 1.5
ENEMY_DROP = 20

enemies = []
for row in range(ENEMY_ROWS):
    for col in range(ENEMY_COLS):
        enemies.append({
            'x': ENEMY_START_X + col * ENEMY_SPACING_X,
            'y': ENEMY_START_Y + row * ENEMY_SPACING_Y,
        })

enemy_dir = 1  # 1 = right, -1 = left

# Game state
game_over = False
win = False

# IMU mapping constants
AY_MIN = -9000
AY_MAX = 9000
ALPHA = 0.7  # low-pass filter coefficient

filtered_ay = 0.0

def read_pico():
    pass  # threading handles serial reads in background

def get_ship_x():
    """Map filtered ay from IMU to ship x position."""
    global ship_x, filtered_ay
    filtered_ay = ALPHA * pico_ay + (1 - ALPHA) * filtered_ay
    t = (filtered_ay - AY_MIN) / (AY_MAX - AY_MIN)
    ship_x = int(max(20, min(WIDTH - 20, t * WIDTH)))
    return ship_x

shoot_cooldown = 0
prev_btn = 1

def check_shoot():
    global shoot_cooldown
    if shoot_cooldown > 0:
        shoot_cooldown -= 1
    if pico_btn == 0 and shoot_cooldown == 0:
        bullets.append({'x': ship_x, 'y': SHIP_Y - 20})
        shoot_cooldown = 15

def update():
    global enemy_dir, game_over, win, ship_x

    if game_over or win:
        return

    # Read from Pico
    read_pico()

    # Update ship from input
    get_ship_x()
    check_shoot()

    # Move bullets
    for b in bullets[:]:
        b['y'] -= BULLET_SPEED
        if b['y'] < 0:
            bullets.remove(b)

    # Move enemies
    move_down = False
    for e in enemies:
        e['x'] += ENEMY_SPEED * enemy_dir
    # Check if any enemy hits wall
    if any(e['x'] > WIDTH - 30 for e in enemies) or any(e['x'] < 30 for e in enemies):
        enemy_dir *= -1
        move_down = True
    if move_down:
        for e in enemies:
            e['y'] += ENEMY_DROP

    # Bullet-enemy collision
    for b in bullets[:]:
        for e in enemies[:]:
            if abs(b['x'] - e['x']) < 25 and abs(b['y'] - e['y']) < 20:
                if b in bullets:
                    bullets.remove(b)
                enemies.remove(e)
                break

    # Enemy reaches bottom
    if any(e['y'] > SHIP_Y - 30 for e in enemies):
        game_over = True

    # Win condition
    if len(enemies) == 0:
        win = True

def draw():
    screen.fill((0, 0, 0))

    if game_over:
        screen.draw.text("GAME OVER", center=(WIDTH//2, HEIGHT//2), fontsize=60, color="red")
        screen.draw.text("Press R to restart", center=(WIDTH//2, HEIGHT//2 + 60), fontsize=30, color="white")
        return

    if win:
        screen.draw.text("YOU WIN!", center=(WIDTH//2, HEIGHT//2), fontsize=60, color="green")
        screen.draw.text("Press R to restart", center=(WIDTH//2, HEIGHT//2 + 60), fontsize=30, color="white")
        return

    # Draw ship
    screen.draw.filled_circle((ship_x, SHIP_Y), 15, (0, 200, 255))
    screen.draw.filled_rect(Rect(ship_x - 25, SHIP_Y - 5, 50, 10), (0, 200, 255))

    # Draw bullets
    for b in bullets:
        screen.draw.filled_rect(Rect(b['x'] - 3, b['y'] - 10, 6, 14), (255, 255, 0))

    # Draw enemies
    for e in enemies:
        screen.draw.filled_rect(Rect(e['x'] - 20, e['y'] - 15, 40, 30), (255, 60, 60))
        # Simple "face"
        screen.draw.filled_circle((e['x'] - 7, e['y'] - 3), 4, (0, 0, 0))
        screen.draw.filled_circle((e['x'] + 7, e['y'] - 3), 4, (0, 0, 0))

    # HUD
    screen.draw.text(f"Enemies: {len(enemies)}", (10, 10), fontsize=24, color="white")
    screen.draw.text("Tilt: move  |  Button: shoot", (10, HEIGHT - 30), fontsize=20, color="gray")

def on_key_down(key):
    global game_over, win, enemies, bullets, ship_x, enemy_dir
    if key == keys.R:
        # Reset
        game_over = False
        win = False
        ship_x = WIDTH // 2
        bullets.clear()
        enemies.clear()
        enemy_dir = 1
        for row in range(ENEMY_ROWS):
            for col in range(ENEMY_COLS):
                enemies.append({
                    'x': ENEMY_START_X + col * ENEMY_SPACING_X,
                    'y': ENEMY_START_Y + row * ENEMY_SPACING_Y,
                })

pgzrun.go()
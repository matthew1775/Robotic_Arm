import math

# --- KONFIGURACJA SIECI ---
BROKER_ADDRESS = "192.168.1.1"   
BROKER_PORT = 1883
TOPIC_CMD      = "Robotic_arm_cmd"
TOPIC_FEEDBACK = "Robotic_arm_enc"

# --- LIMITY STOPNI SWOBODY (Min, Max) w stopniach ---
AXIS_LIMITS = [
    (-90.0, 90.0),    # 1. Obrotnica
    (-60.0, 60.0),    # 2. Bark
    (-10.0, 110.0),   # 3. Łokieć
    (-10.0, 90.0),    # 4. Nadgarstek
    (-360.0, 360.0),  # 5. Oś 5 (Przegub)
    (-20.0, 20.0),    # 6. Wysuw
    (0.0, 360.0),     # 7. Rotacja końcówki (modulo 360)
    (0.0, 60.0)       # 8. Szczęki
]

# --- PRĘDKOŚCI RUCHU (Stopnie na cykl dla Osi 1-8) ---
AXIS_SPEEDS = [
    1.5,    # 1. Obrotnica
    1.0,    # 2. Bark
    1.0,    # 3. Łokieć
    1.5,    # 4. Nadgarstek
    3.0,    # 5. Oś 5
    0.5,    # 6. Wysuw
    3.0,    # 7. Rotacja Końcówki
    2.0     # 8. Szczęki
]

# --- PARAMETRY SYMULACJI (Wygląd na ekranie) ---
LINK_LENGTHS = [0, 112, 112, 84, 28] 
JOYSTICK_DEADZONE = 0.15

# --- KOLORY GUI ---
BG_COLOR = "#1e1e1e"
FG_COLOR = "#ffffff"
BTN_RESET_COLOR = "#cc3333"
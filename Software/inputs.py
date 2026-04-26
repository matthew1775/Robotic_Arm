import pygame
import config

class InputManager:
    def __init__(self):
        pygame.init()
        pygame.joystick.init()
        self.joysticks = []
        self.scan_joysticks()

    def scan_joysticks(self):
        self.joysticks = []
        pygame.joystick.quit()
        pygame.joystick.init()
        count = pygame.joystick.get_count()
        self.joysticks = [pygame.joystick.Joystick(i) for i in range(count)]
        for joy in self.joysticks:
            joy.init()
        return self.joysticks

    def handle_keyboard(self, event_type, key_code):
        pass
            
    def update(self, app_state):
        pygame.event.pump()
        if not self.joysticks:
            return

        joy = self.joysticks[0]
        deltas = [0.0] * 8
        
        # Funkcje pomocnicze
        def get_btn(idx):
            return joy.get_button(idx) if idx < joy.get_numbuttons() else 0
            
        def get_ax(idx):
            if idx < joy.get_numaxes():
                val = joy.get_axis(idx)
                return val if abs(val) > config.JOYSTICK_DEADZONE else 0.0
            return 0.0
            
        def get_hat_y(): 
            return joy.get_hat(0)[1] if joy.get_numhats() > 0 else 0

        # --- MAPOWANIE Z PADA DO OS 1-8 ---
        # 1. OBROTNICA (Triggery)
        trig_left = joy.get_axis(4) if joy.get_numaxes() > 4 else -1.0
        trig_right = joy.get_axis(5) if joy.get_numaxes() > 5 else -1.0
        if trig_left > -0.5: deltas[0] -= 1.0
        if trig_right > -0.5: deltas[0] += 1.0

        # 2. BARK (Lewa gałka Y)
        deltas[1] = -get_ax(1)

        # 3. ŁOKIEĆ (Prawa gałka Y)
        deltas[2] = -get_ax(3)

        # 4. NADGARSTEK (Krzyżak)
        hat_val = get_hat_y()
        if hat_val == -1: deltas[3] -= 1.0
        elif hat_val == 1: deltas[3] += 1.0

        # 5. OŚ 5 (LB / RB)
        if get_btn(4): deltas[4] -= 1.0 
        if get_btn(5): deltas[4] += 1.0 

        # 6. WYSUW (L3 / R3)
        if get_btn(8): deltas[5] -= 1.0
        if get_btn(9): deltas[5] += 1.0
        
        # 7. ROTACJA KOŃCÓWKI (X / B)
        if get_btn(2): deltas[6] -= 1.0
        if get_btn(1): deltas[6] += 1.0

        # 8. SZCZĘKI (A / Y)
        if get_btn(0): deltas[7] += 1.0 # Otwieranie
        if get_btn(3): deltas[7] -= 1.0 # Zamykanie

        # --- APLIKACJA ZMIAN Z LIMITAMI ---
        for i in range(8):
            if deltas[i] != 0.0:
                new_val = app_state.target_joints_deg[i] + (deltas[i] * config.AXIS_SPEEDS[i])
                
                # Zabezpieczenie limitów osi
                l_min, l_max = config.AXIS_LIMITS[i]
                if i == 6: # Rotacja końcówki (zawijanie 360 stopni)
                    new_val %= 360.0
                else:
                    new_val = max(l_min, min(l_max, new_val))
                    
                app_state.target_joints_deg[i] = new_val
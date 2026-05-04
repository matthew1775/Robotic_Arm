import tkinter as tk
from tkinter import scrolledtext
import threading
import time
import subprocess
import platform
import math
import socket
import concurrent.futures
import config

class DashboardGUI:
    def __init__(self, root, app_state, input_manager, mqtt_manager):
        self.root = root
        self.state = app_state
        self.input_manager = input_manager
        self.mqtt_manager = mqtt_manager
        self._last_rendered_arm_axes = None
        self._last_rendered_tip_state = None
        self.setup_ui()
        self._start_network_monitor()
    
    def _check_connection(self, host, port=None):
        """Zoptymalizowana funkcja sprawdzania dostępności w sieci"""
        if port:
            # Bardzo szybkie sprawdzenie za pomocą Socket (nie obciąża procesora)
            try:
                with socket.create_connection((host, port), timeout=0.5):
                    return True
            except OSError:
                return False
        else:
            # Klasyczny ping tylko dla urządzeń bez otwartych portów
            try:
                param = '-n' if platform.system().lower() == 'windows' else '-c'
                timeout_param = '-w' if platform.system().lower() == 'windows' else '-W'
                cmd = ['ping', param, '1', timeout_param, '500' if platform.system().lower() == 'windows' else '1', host]
                startupinfo = None
                if platform.system().lower() == 'windows':
                    startupinfo = subprocess.STARTUPINFO()
                    startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
                return subprocess.call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, startupinfo=startupinfo) == 0
            except Exception: return False

    def _start_network_monitor(self):
        def monitor_loop():
            with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
                while True:
                    # Równoległe sprawdzenie połączeń z natychmiastową aktualizacją stanu
                    futures_to_attr = {
                        executor.submit(self._check_connection, "192.168.1.1", 1883): "ping_broker_ok",
                        executor.submit(self._check_connection, "192.168.1.102"): "ping_router_ok",
                        executor.submit(self._check_connection, "192.168.1.101"): "ping_ground_ok"
                    }

                    for future in concurrent.futures.as_completed(futures_to_attr):
                        attr_name = futures_to_attr[future]
                        try:
                            result = future.result()
                            setattr(self.state, attr_name, result)
                        except Exception:
                            setattr(self.state, attr_name, False)

                    time.sleep(1.5) # Zwiększono odstęp miedzy sprawdzeniami z 1s na 1.5s
        threading.Thread(target=monitor_loop, daemon=True).start()
    
    def setup_ui(self):
        self.root.configure(bg=config.BG_COLOR)
        self.top_frame = tk.Frame(self.root, bg=config.BG_COLOR)
        self.top_frame.pack(side="top", fill="x", padx=10, pady=5)
        self.main_frame = tk.Frame(self.root, bg=config.BG_COLOR)
        self.main_frame.pack(fill="both", expand=True, padx=10, pady=5)

        self._build_top_panel()
        self._build_joints_grid()
        self._build_simulation_and_console()

    def _build_top_panel(self):
        joy_frame = tk.Frame(self.top_frame, bg=config.BG_COLOR)
        joy_frame.pack(side="left")
        tk.Button(joy_frame, text="⟳ RESET JOYSTICK", command=self.refresh_joysticks, bg=config.BTN_RESET_COLOR, fg="white", font=("Arial", 10, "bold")).pack(side="left", padx=5)
        self.lbl_joy = tk.Label(joy_frame, text="Joystick: Brak", bg=config.BG_COLOR, fg="yellow", font=("Arial", 10))
        self.lbl_joy.pack(side="left", padx=5)

        net_frame = tk.Frame(self.top_frame, bg=config.BG_COLOR)
        net_frame.pack(side="right")
        self.lbl_mqtt_status = tk.Label(net_frame, text="MQTT: --", bg=config.BG_COLOR, fg=config.FG_COLOR, font=("Arial", 10))
        self.lbl_mqtt_status.pack(side="left", padx=10)

        self.led_canvas = tk.Canvas(net_frame, width=200, height=25, bg=config.BG_COLOR, highlightthickness=0)
        self.led_canvas.pack(side="left", padx=5)
        self.led_ground = self.led_canvas.create_oval(5, 5, 20, 20, fill="grey", outline="white")
        self.led_canvas.create_text(25, 12, text="Gnd", fill="white", anchor="w", font=("Arial", 8))
        self.led_router = self.led_canvas.create_oval(65, 5, 80, 20, fill="grey", outline="white")
        self.led_canvas.create_text(85, 12, text="Router", fill="white", anchor="w", font=("Arial", 8))
        self.led_broker = self.led_canvas.create_oval(130, 5, 145, 20, fill="grey", outline="white")
        self.led_canvas.create_text(150, 12, text="MQTT", fill="white", anchor="w", font=("Arial", 8))

    def _build_joints_grid(self):
        self.grid_frame = tk.Frame(self.main_frame, bg=config.BG_COLOR)
        self.grid_frame.pack(side="top", fill="x")
        self.joint_widgets = []
        names = ["1. Obrotnica", "2. Bark", "3. Łokieć", "4. Nadgarstek", "5. Oś 5 (LB/RB)", "6. Wysuw", "7. Rotacja Końc.", "8. Szczęki"]
        for i in range(8):
            frame = tk.LabelFrame(self.grid_frame, text=names[i], bg=config.BG_COLOR, fg="white", font=("Arial", 10, "bold"))
            frame.grid(row=i//4, column=i%4, padx=10, pady=5, sticky="nsew")
            self.grid_frame.grid_columnconfigure(i%4, weight=1)
            
            cvs = tk.Canvas(frame, width=120, height=120, bg="#2b2b2b", highlightthickness=1, highlightbackground="#555")
            cvs.pack(pady=5)
            
            # NAPRAWA: Rysujemy tło okręgu TYLKO RAZ podczas tworzenia kafelka, a nie 20 razy na sekundę
            cvs.create_oval(10, 10, 110, 110, outline="#444", width=4)
            
            lbl = tk.Label(frame, text="T: 0.0°\nA: 0.0°", bg=config.BG_COLOR, fg="white", font=("Consolas", 11))
            lbl.pack(pady=2)
            self.joint_widgets.append((cvs, lbl))

    def _build_simulation_and_console(self):
        bottom_frame = tk.Frame(self.main_frame, bg=config.BG_COLOR)
        bottom_frame.pack(side="bottom", fill="both", expand=True, pady=10)
        
        self.arm_frame = tk.LabelFrame(bottom_frame, text="Symulacja Kinematyki Ramienia", bg=config.BG_COLOR, fg="white")
        self.arm_frame.pack(side="left", fill="both", expand=True, padx=(0, 5))
        self.canvas_arm = tk.Canvas(self.arm_frame, bg="#2b2b2b", highlightthickness=0)
        self.canvas_arm.pack(fill="both", expand=True, padx=5, pady=5)

        self.tip_frame = tk.LabelFrame(bottom_frame, text="Narzędzie", bg=config.BG_COLOR, fg="white")
        self.tip_frame.pack(side="left", fill="y", padx=5)
        self.canvas_tip = tk.Canvas(self.tip_frame, bg="#222", width=250, height=250, highlightthickness=0)
        self.canvas_tip.pack(anchor="center", padx=5, pady=5)

        self.console = scrolledtext.ScrolledText(bottom_frame, bg="#222", fg="#00ff00", width=40, font=("Consolas", 9))
        self.console.pack(side="right", fill="y", padx=(5, 0))

    def refresh_joysticks(self):
        joysticks = self.input_manager.scan_joysticks()
        if not joysticks: self.lbl_joy.config(text="Joystick: Brak", fg="yellow")
        else: self.lbl_joy.config(text=f"Joystick: {joysticks[0].get_name()[:20]}", fg="green")

    def update_interface(self):
        # Aktualizacja statusów LED
        self.lbl_mqtt_status.config(text=self.state.mqtt_status_text)
        self.led_canvas.itemconfig(self.led_ground, fill="#00ff00" if self.state.ping_ground_ok else "#444")
        self.led_canvas.itemconfig(self.led_router, fill="#00ff00" if self.state.ping_router_ok else "#444")
        self.led_canvas.itemconfig(self.led_broker, fill="#00ff00" if self.state.ping_broker_ok else "#444")

        # Rysowanie 8 kwadratów
        for i in range(8):
            cvs, lbl = self.joint_widgets[i]
            target_deg = self.state.target_joints_deg[i]
            actual_deg = self.state.actual_joints_deg[i]
            
            lbl.config(text=f"T: {target_deg:.1f}°\nA: {actual_deg:.1f}°")
            lbl.config(fg="#00ff00" if abs(target_deg - actual_deg) < 2.0 else "orange")
            
            # NAPRAWA: Zmieniamy tylko zawartość taga "arc" (sam niebieski łuk)
            l_min, l_max = config.AXIS_LIMITS[i]
            rng = (l_max - l_min) if l_max != l_min else 360
            angle_arc = ((actual_deg - l_min) / rng) * 360 if rng != 0 else 0
            if not cvs.find_withtag("arc"):
                cvs.create_arc(10, 10, 110, 110, start=90, extent=-angle_arc, style="arc", outline="#00A2FF", width=10, tags="arc")
            else:
                cvs.itemconfig("arc", extent=-angle_arc)

        # Rysowanie symulacji 2D ramienia (tylko gdy zmienily sie pozycje docelowe)
        current_arm_axes = tuple(self.state.target_joints_deg[0:6])
        if current_arm_axes != self._last_rendered_arm_axes:
            self.draw_arm(self.state.target_joints_deg)
            self._last_rendered_arm_axes = current_arm_axes

        current_tip_state = (self.state.target_joints_deg[6], self.state.target_joints_deg[7])
        if current_tip_state != self._last_rendered_tip_state:
            self.draw_scissor_tip(self.state.target_joints_deg[6], self.state.target_joints_deg[7])
            self._last_rendered_tip_state = current_tip_state

        # Wypisywanie logów
        while self.state.logs:
            self.console.insert(tk.END, "> " + self.state.logs.pop(0) + "\n")
            self.console.see(tk.END)

    def draw_arm(self, axes_deg):
        c = self.canvas_arm
        c.delete("all") # Oczyszczanie przed rysowaniem
        w, h = c.winfo_width(), c.winfo_height()
        if w < 10: return
        cx, cy = w // 2, h - 30 
        
        base_rot_val = axes_deg[0]
        c.create_rectangle(cx-30, cy, cx+30, cy+15, fill="#333", outline="#555", width=2)
        c.create_rectangle(cx-20, cy-5, cx+20, cy, fill="#444", outline="#555") 
        
        arrow_act = "#ff00ff"
        arrow_inact = "#555"
        col_L = arrow_act if base_rot_val < -1.0 else arrow_inact
        col_R = arrow_act if base_rot_val > 1.0 else arrow_inact

        c.create_polygon([(cx-32, cy+18), (cx-42, cy+25), (cx-28, cy+28)], fill=col_L, outline=col_L)
        c.create_polygon([(cx+32, cy+18), (cx+42, cy+25), (cx+28, cy+28)], fill=col_R, outline=col_R)
        c.create_text(cx, cy+35, text=f"Obrót: {base_rot_val:.0f}°", fill="#888", font=("Arial", 8))

        L = config.LINK_LENGTHS
        scale_draw = 0.7 
        
        q1 = math.radians(axes_deg[1]) - math.pi/2 
        q2 = math.radians(axes_deg[2])
        q3 = math.radians(axes_deg[3])
        
        x0, y0 = cx, cy-5
        x1 = x0 + (L[1]*scale_draw) * math.cos(q1)
        y1 = y0 + (L[1]*scale_draw) * math.sin(q1)
        x2 = x1 + (L[2]*scale_draw) * math.cos(q1 + q2)
        y2 = y1 + (L[2]*scale_draw) * math.sin(q1 + q2)
        x3 = x2 + (L[3]*scale_draw) * math.cos(q1 + q2 + q3)
        y3 = y2 + (L[3]*scale_draw) * math.sin(q1 + q2 + q3)
        
        ext = axes_deg[5] * 4.2
        x4 = x3 + ((L[4] + ext)*scale_draw) * math.cos(q1 + q2 + q3)
        y4 = y3 + ((L[4] + ext)*scale_draw) * math.sin(q1 + q2 + q3)

        c.create_line(x0, y0, x1, y1, width=6, fill="#888", capstyle="round")
        c.create_line(x1, y1, x2, y2, width=6, fill="#888", capstyle="round")
        c.create_line(x2, y2, x3, y3, width=6, fill="#888", capstyle="round")
        c.create_line(x3, y3, x4, y4, width=4, fill="#ff6600", capstyle="round")
        
        for px, py in [(x1,y1), (x2,y2), (x3,y3)]:
            c.create_oval(px-3, py-3, px+3, py+3, fill="#00ffff", outline="")

    def draw_scissor_tip(self, rotation_deg, jaw_angle):
        c = self.canvas_tip
        c.delete("all")
        cx, cy = 125, 125 
        visual_rot_rad = math.radians(-90)
        jaw_rad = math.radians(jaw_angle)
        blade_len = 60 
        blade_width = 12
        
        def rotate_pt(x, y, angle):
            return x * math.cos(angle) - y * math.sin(angle) + cx, x * math.sin(angle) + y * math.cos(angle) + cy

        c.create_text(cx, cy+70, text=f"Rotacja: {rotation_deg:.0f}°", fill="#888", font=("Arial", 10, "bold"))
        c.create_oval(cx-20, cy-20, cx+20, cy+20, outline="#555", width=2)
        c.create_line(cx, cy-20, cx, cy+20, fill="#333", width=2)

        angle1 = visual_rot_rad - (jaw_rad / 2.0)
        b1_pts = [(0, -3), (blade_len, -blade_width), (blade_len, 0), (0, 3)]
        c.create_polygon([rotate_pt(px, py, angle1) for px, py in b1_pts], fill="#ccc", outline="black")

        angle2 = visual_rot_rad + (jaw_rad / 2.0)
        b2_pts = [(0, 3), (blade_len, blade_width), (blade_len, 0), (0, -3)]
        c.create_polygon([rotate_pt(px, py, angle2) for px, py in b2_pts], fill="#ccc", outline="black")
        c.create_oval(cx-4, cy-4, cx+4, cy+4, fill="#ff6600")
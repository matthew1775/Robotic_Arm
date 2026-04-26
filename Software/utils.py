class AppState:
    """Klasa przechowująca współdzielony stan aplikacji dla 8 osi (w STOPNIACH)"""
    def __init__(self):
        # Osie: [0]Obrotnica, [1]Bark, [2]Łokieć, [3]Nadgarstek, [4]Przegub, [5]Wysuw, [6]Rotacja, [7]Szczęki
        self.target_joints_deg = [0.0, 0.0, 50.0, 40.0, 0.0, 0.0, 0.0, 0.0]
        self.actual_joints_deg = [0.0] * 8
        self.target_speeds = [50.0] * 8 
        
        self.mqtt_connected = False
        self.mqtt_status_text = "MQTT: Rozłączono"
        self.logs = [] 
        
        self.ping_broker_ok = False   
        self.ping_router_ok = False   
        self.ping_ground_ok = False   

    def log(self, message):
        self.logs.append(message)
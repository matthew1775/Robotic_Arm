import paho.mqtt.client as mqtt
import json
import math
import config

class MqttManager:
    def __init__(self, app_state):
        self.client = None
        self.state = app_state

    def connect(self):
        self.state.log("--- MQTT Initialization ---")
        try:
            self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
            self.client.on_connect = self._on_connect
            self.client.on_message = self._on_message
            self.client.connect(config.BROKER_ADDRESS, config.BROKER_PORT)
            self.client.loop_start()
        except Exception as e:
            self.state.log(f"Critical connection error: {e}")

    def _on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.state.mqtt_connected = True
            self.state.mqtt_status_text = "MQTT: CONNECTED"
            client.subscribe(config.TOPIC_FEEDBACK)

    def _on_message(self, client, userdata, msg):
        try:
            payload_wrapper = json.loads(msg.payload.decode())
            if payload_wrapper.get("eventType") == "Robotic_arm_enc":
                p = payload_wrapper.get("payload", {})
                
                # Konwersja odebranych RADIANÓW na STOPNIE
                for i in range(8):
                    key = f"enc_joint{i+1}_rad"
                    rad_val = p.get(key, 0.0)
                    self.state.actual_joints_deg[i] = math.degrees(rad_val)

        except Exception as e:
            self.state.log(f"MQTT receive error: {e}")

    def send_drive_command(self):
        if self.client and self.state.mqtt_connected:
            payload_data = {}
            
            # Wysłanie 8 głównych przegubów: Konwersja STOPNI -> RADIANY
            for i in range(8):
                rad_val = math.radians(self.state.target_joints_deg[i])
                payload_data[f"joint{i+1}_rad"] = round(rad_val, 4)
                payload_data[f"joint{i+1}_speed"] = round(self.state.target_speeds[i], 1)
                
            for i in range(5):
                payload_data[f"joint_mini_{i+1}_rad"] = 0.0
                payload_data[f"joint_mini_{i+1}_speed"] = 0.0

            msg = {
                "eventType": "Robotic_arm_cmd",
                "payload": payload_data,
                "cmd": {"fan": False}
            }
            try:
                self.client.publish(config.TOPIC_CMD, json.dumps(msg))
            except Exception as e:
                self.state.log(f"Data send error: {e}")
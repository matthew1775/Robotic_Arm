192.168.1.1 - Serwer mqtt
Robotic_Arm: 192.168.1.53 - ip adress esp32
Mqtt name from esp to main computer: Robotic_arm_enc
Mqtt name from main computer to esp computer: Robotic_arm_cmd

{
  "eventType": "Robotic_arm_cmd",
  "payload": {
    /* --- Główne przeguby ramienia (Pozycje: 0.0 do 3.0 rad) --- */
    "joint1_rad": 0.0,      // Pozycja kątowa głównego przegubu 1 [0-3 rad]
    "joint2_rad": 0.0,      // Pozycja kątowa głównego przegubu 2 [0-3 rad]
    "joint3_rad": 0.0,      // Pozycja kątowa głównego przegubu 3 [0-3 rad]
    "joint4_rad": 0.0,      // Pozycja kątowa głównego przegubu 4 [0-3 rad]
    "joint5_rad": 0.0,      // Pozycja kątowa głównego przegubu 5 [0-3 rad]
    "joint6_rad": 0.0,      // Pozycja kątowa głównego przegubu 6 [0-3 rad]
    "joint7_rad": 0.0,      // Pozycja kątowa głównego przegubu 7 [0-3 rad]
    "joint8_rad": 0.0,      // Pozycja kątowa głównego przegubu 8 [0-3 rad]

    /* --- Przeguby pomocnicze MINI (Pozycje: 0.0 do 3.0 rad) --- */
    "joint_mini_1_rad": 0.0, // Pozycja kątowa przegubu mini 1 [0-3 rad]
    "joint_mini_2_rad": 0.0, // Pozycja kątowa przegubu mini 2 [0-3 rad]
    "joint_mini_3_rad": 0.0, // Pozycja kątowa przegubu mini 3 [0-3 rad]
    "joint_mini_4_rad": 0.0, // Pozycja kątowa przegubu mini 4 [0-3 rad]
    "joint_mini_5_rad": 0.0, // Pozycja kątowa przegubu mini 5 [0-3 rad]

    /* --- Prędkości głównych przegubów (Prędkość: 0 do 100) --- */
    "joint1_speed": 0.0,    // Prędkość obrotowa głównego przegubu 1 [0-100]
    "joint2_speed": 0.0,    // Prędkość obrotowa głównego przegubu 2 [0-100]
    "joint3_speed": 0.0,    // Prędkość obrotowa głównego przegubu 3 [0-100]
    "joint4_speed": 0.0,    // Prędkość obrotowa głównego przegubu 4 [0-100]
    "joint5_speed": 0.0,    // Prędkość obrotowa głównego przegubu 5 [0-100]
    "joint6_speed": 0.0,    // Prędkość obrotowa głównego przegubu 6 [0-100]
    "joint7_speed": 0.0,    // Prędkość obrotowa głównego przegubu 7 [0-100]
    "joint8_speed": 0.0,    // Prędkość obrotowa głównego przegubu 8 [0-100]

    /* --- Prędkości przegubów pomocniczych MINI (Prędkość: 0 do 100) --- */
    "joint_mini_1_speed": 0.0, // Prędkość obrotowa przegubu mini 1 [0-100]
    "joint_mini_2_speed": 0.0, // Prędkość obrotowa przegubu mini 2 [0-100]
    "joint_mini_3_speed": 0.0, // Prędkość obrotowa przegubu mini 3 [0-100]
    "joint_mini_4_speed": 0.0, // Prędkość obrotowa przegubu mini 4 [0-100]
    "joint_mini_5_speed": 0.0  // Prędkość obrotowa przegubu mini 5 [0-100]
  }
}
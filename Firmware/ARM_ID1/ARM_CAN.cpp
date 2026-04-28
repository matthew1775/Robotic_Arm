#include "ARM_CAN.h"

// ==========================================
// Ustawienia sprzętowe i protokołu
// ==========================================
constexpr long CAN_BAUDRATE     = 500E3; // 500 kbps
constexpr uint32_t MY_NODE_ID   = 0x01;  // ID Głównego ESP32

constexpr uint32_t JOINT_2_ID   = 0x02;  // Docelowe ID dla Joint 2
constexpr uint32_t JOINT_3_ID   = 0x03;  // Docelowe ID dla Joint 3 (Dodane do karuzeli)
constexpr uint32_t JOINT_4_ID   = 0x04;  // Telemetria z ID 4
constexpr uint32_t JOINT_4_CMD_ID = 0x14; // Komendy ruchu dla ID 4
constexpr uint32_t MINI_SERVO_ID = 10;   // Docelowe ID 10 dla Mini Serw

ST3025_Data servos_id4[4];  

void initCAN() {
    CAN.setPins(Pins::RX_CAN_PIN, Pins::TX_CAN_PIN);
    CAN.begin(CAN_BAUDRATE);
}

void handleCAN() {
    // ==========================================
    // 1. ODBIÓR DANYCH Z CAN (Błyskawiczny nasłuch)
    // ZMIANA: Używamy 'while' zamiast 'if'. Dzięki temu jeśli przyjdą 
    // np. 2 ramki na raz, procesor ściągnie obie zanim ruszy dalej.
    // ==========================================
    while (int packetSize = CAN.parsePacket()) {
        uint32_t receivedId = CAN.packetId();
        
        // --- Odbiór enkodera z Joint 2 (ID 2) ---
        if (receivedId == JOINT_2_ID && packetSize >= 4) {
            float enc_pos = 0.0f;
            uint8_t buffer[4];
            for (int i = 0; i < 4; i++) buffer[i] = CAN.read();
            memcpy(&enc_pos, &buffer[0], 4);
            enc_joints[1] = enc_pos; 
            can_status[0] = true; 
        }
        // --- Odbiór enkodera z Joint 3 (ID 3) ---
        else if (receivedId == JOINT_3_ID && packetSize >= 4) {
            float enc_pos = 0.0f;
            uint8_t buffer[4];
            for (int i = 0; i < 4; i++) buffer[i] = CAN.read();
            memcpy(&enc_pos, &buffer[0], 4);
            enc_joints[2] = enc_pos; 
            can_status[1] = true; 
        }
        // --- Odbiór enkoderów z Mini Serw (ID 10) ---
        else if (receivedId == MINI_SERVO_ID && packetSize >= 5) {
            uint8_t servo_idx = CAN.read(); 
            if (servo_idx < 5) {
                float enc_pos = 0.0f;
                uint8_t buffer[4];
                for (int i = 0; i < 4; i++) buffer[i] = CAN.read();
                memcpy(&enc_pos, buffer, 4);
                enc_mini_joints[servo_idx] = enc_pos; 
            }
        }
        // --- Odbiór telemetrii z ID 4 (Ping-Pong z ST3025) ---
        else if (receivedId == JOINT_4_ID && packetSize == 5) { 
            uint8_t servo_idx = CAN.read(); 
            if (servo_idx < 4) {
                can_status[2] = true; 
                servos_id4[servo_idx].is_connected = true; 
                
                uint8_t buf[4];
                for (int i = 0; i < 4; i++) buf[i] = CAN.read(); 
                memcpy(&servos_id4[servo_idx].position, &buf[0], 2);
                memcpy(&servos_id4[servo_idx].speed,    &buf[2], 2);
            }
        }
    }

    // ==========================================
    // 2. NADAWANIE SEKWENCYJNE (KARUZELA MASTER-SLAVE)
    // Procesor ID 1 co 3 milisekundy strzela kolejną ramką
    // do kolejnego serwa, upewniając się, że kabel CAN jest pusty.
    // ==========================================
    static unsigned long last_poll_time = 0;
    static uint8_t poll_step = 0;

    if (millis() - last_poll_time >= 3) {
        last_poll_time = millis();

        switch(poll_step) {
            case 0: { 
                // Krok 0: Wyślij komendę do Joint 2
                CAN.beginPacket(JOINT_2_ID);
                float j2_pos = target_joints[1];   
                float j2_speed = target_speeds[1]; 
                CAN.write((const uint8_t*)&j2_pos, 4);
                CAN.write((const uint8_t*)&j2_speed, 4);
                CAN.endPacket();
                break;
            }
            case 1: { 
                // Krok 1: Wyślij komendę do Joint 3
                CAN.beginPacket(JOINT_3_ID);
                float j3_pos = target_joints[2];   
                float j3_speed = target_speeds[2]; 
                CAN.write((const uint8_t*)&j3_pos, 4);
                CAN.write((const uint8_t*)&j3_speed, 4);
                CAN.endPacket();
                break;
            }
            case 2:
            case 3:
            case 4:
            case 5:
            case 6: { 
                // Kroki 2 do 6: Wyślij komendy do 5 Mini Serw 
                uint8_t mini_idx = poll_step - 2; 
                CAN.beginPacket(MINI_SERVO_ID);
                float mini_pos = target_mini_joints[mini_idx];
                CAN.write(mini_idx); 
                CAN.write((const uint8_t*)&mini_pos, 4); 
                CAN.endPacket();
                break;
            }
            case 7:
            case 8:
            case 9:
            case 10: { 
                // Kroki 7 do 10: Wyślij komendy do Serw ID 4 
                uint8_t id4_idx = poll_step - 7; 
                CAN.beginPacket(JOINT_4_CMD_ID); 
                
                // Odpowiednie mapowanie osi (joint4 -> idx 3, joint7 -> idx 6)
                float radians = target_joints[id4_idx + 3]; 
                int16_t target_pos_steps = (int16_t)(radians * (180.0 / PI) * (4096.0 / 360.0)); 
                int16_t speed = (int16_t)target_speeds[id4_idx + 3];
                
                CAN.write(id4_idx); 
                CAN.write((const uint8_t*)&target_pos_steps, 2); 
                CAN.write((const uint8_t*)&speed, 2);            
                CAN.endPacket();
                break;
            }
        }

        // Przejście do kolejnego węzła
        poll_step++;
        if (poll_step > 10) {
            poll_step = 0; // Reset karuzeli na koniec cyklu
        }
    }
}
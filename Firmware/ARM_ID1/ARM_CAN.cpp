#include "ARM_CAN.h"

// Ustawienia sprzętowe i protokołu
constexpr long CAN_BAUDRATE     = 500E3; // 500 kbps
constexpr uint32_t MY_NODE_ID   = 0x01;  // Nasze ID_1
constexpr uint32_t JOINT_2_ID   = 0x02;  // Docelowe ID dla Joint 2
constexpr uint32_t MINI_SERVO_ID = 10;   // Docelowe ID 10 (0x0A) dla Mini Serw

// Zmienna do wysyłania cyklicznego
unsigned long lastCanSend = 0;
const unsigned long CAN_SEND_INTERVAL = 50; // Wysyłanie co 50ms (20 Hz)

void initCAN() {
    Serial.println("[CAN] Inicjalizacja Węzła CAN ID_1 ...");
    
    CAN.setPins(Pins::RX_CAN_PIN, Pins::TX_CAN_PIN);
    
    if (!CAN.begin(CAN_BAUDRATE)) {
        Serial.println("[CAN] BŁĄD: Inicjalizacja CAN nie powiodła się!");
    } else {
        Serial.println("[CAN] Uruchomiony pomyślnie na 500kbps.");
    }
}

void handleCAN() {
    // ==========================================
    // 1. ODBIÓR DANYCH Z CAN (Nasłuchiwanie)
    // ==========================================
    int packetSize = CAN.parsePacket();
    if (packetSize) {
        uint32_t receivedId = CAN.packetId();
        
        // --- Odbiór enkodera z Joint 2 (ID 2) ---
        // Oczekujemy minimum 4 bajtów z pozycją float
        if (receivedId == JOINT_2_ID && packetSize >= 4) {
            float enc_pos = 0.0f;
            uint8_t buffer[8] = {0};
            for (int i = 0; i < packetSize && i < 8; i++) {
                buffer[i] = CAN.read();
            }
            
            // Konwersja bajtów na float
            memcpy(&enc_pos, &buffer[0], 4);
            enc_joints[1] = enc_pos; // Zapis do tablicy enc_joints (indeks 1 to Joint 2)
            
            // Aktualizacja statusu CAN dla ID 2 (indeks 0 w can_status)
            can_status[0] = true; 
        }
        
        // --- Odbiór enkoderów z Mini Serw (ID 10) ---
        // Oczekujemy: [1 bajt indeks (0-4)] + [4 bajty pozycja float]
        else if (receivedId == MINI_SERVO_ID && packetSize >= 5) {
            uint8_t servo_idx = CAN.read(); // Odczyt pierwszego bajtu (indeksu)
            
            if (servo_idx < 5) {
                float enc_pos = 0.0f;
                uint8_t buffer[4] = {0};
                for (int i = 0; i < 4; i++) buffer[i] = CAN.read();
                
                memcpy(&enc_pos, buffer, 4);
                enc_mini_joints[servo_idx] = enc_pos; // Aktualizacja konkretnego mini serwa
            }
        }
    }

    // ==========================================
    // 2. CYKLICZNE WYSYŁANIE DANYCH (Co 50ms)
    // ==========================================
    unsigned long currentMillis = millis();
    if (currentMillis - lastCanSend >= CAN_SEND_INTERVAL) {
        lastCanSend = currentMillis;

        // --- Wysyłanie do Joint 2 (ID 2) ---
        // Budowa ramki 8-bajtowej: [4 bajty cel (rad)] + [4 bajty prędkość]
        CAN.beginPacket(JOINT_2_ID);
        float j2_pos = target_joints[1];   // target_joints[1] to Joint 2
        float j2_speed = target_speeds[1]; // target_speeds[1] to Joint 2 speed
        CAN.write((const uint8_t*)&j2_pos, 4);
        CAN.write((const uint8_t*)&j2_speed, 4);
        CAN.endPacket();

        // --- Wysyłanie do Mini Serw (ID 10) ---
        // Wysyłamy 5 małych ramek, każda dla innego serwa mini
        for (uint8_t i = 0; i < 5; i++) {
            CAN.beginPacket(MINI_SERVO_ID);
            float mini_pos = target_mini_joints[i];
            
            CAN.write(i); // Pierwszy bajt: do którego serwa to trafia (0-4)
            CAN.write((const uint8_t*)&mini_pos, 4); // Pozycja z MQTT
            CAN.endPacket();
        }
    }
}
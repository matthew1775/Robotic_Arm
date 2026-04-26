#include "ARM_CAN.h"

// Ustawienia sprzętowe i protokołu
constexpr long CAN_BAUDRATE     = 500E3; // 500 kbps
constexpr uint32_t MY_NODE_ID   = 0x01;  // Nasze ID_1
constexpr uint32_t JOINT_2_ID   = 0x02;  // Docelowe ID dla Joint 2
constexpr uint32_t MINI_SERVO_ID = 10;   // Docelowe ID 10 (0x0A) dla Mini Serw


constexpr uint32_t JOINT_4_ID = 0x04; // Docelowe ID dla Węzła 4
constexpr uint32_t JOINT_4_CMD_ID = 0x14; 
ST3025_Data servos_id4[4];             // Inicjalizacja globalnej tablicy na dane serw


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
        // --- Odbiór telemetrii z ID 4 (Serwa ST3025) ---
        else if (receivedId == JOINT_4_ID && packetSize == 8) {
            uint8_t servo_idx = CAN.read(); // Bajt 0: indeks serwa (0-3)
            uint8_t frame_part = CAN.read(); // Bajt 1: część ramki (0=A, 1=B)
            
            if (servo_idx < 4) {
                // can_status indeks 2 odpowiada za ID 4 (bo 0->ID2, 1->ID3, 2->ID4)
                can_status[2] = true; 

                // Odczytanie pozostałych 6 bajtów do bufora tymczasowego
                uint8_t buf[6];
                for (int i = 0; i < 6; i++) {
                    buf[i] = CAN.read();
                }
                
                // Dekodowanie na podstawie części ramki (0 lub 1)
                if (frame_part == 0) { // Część A (Połączenie, Move, Pozycja, Prędkość)
                    servos_id4[servo_idx].is_connected = buf[0];
                    servos_id4[servo_idx].is_moving    = buf[1];
                    memcpy(&servos_id4[servo_idx].position, &buf[2], 2);
                    memcpy(&servos_id4[servo_idx].speed,    &buf[4], 2);
                } 
                else if (frame_part == 1) { // Część B (Load, Current, Voltage, Temperatura)
                    memcpy(&servos_id4[servo_idx].load,    &buf[0], 2);
                    memcpy(&servos_id4[servo_idx].current, &buf[2], 2);
                    servos_id4[servo_idx].voltage     = buf[4];
                    servos_id4[servo_idx].temperature = buf[5];
                }
            }
        }

        /*// Przykładowa funkcja na ESP32 (Węzeł ID 4) ładująca dane i wysyłająca je na CAN
        void sendST3025Telemetry(uint8_t servoIndex, bool connected, bool move, int16_t pos, int16_t speed, int16_t load, int16_t current, uint8_t volt, uint8_t temp) {
            
            // Budowanie Ramki A
            CAN.beginPacket(0x04);
            CAN.write(servoIndex);   // Bajt 0: Numer serwa
            CAN.write(0);            // Bajt 1: Typ = Ramka A
            CAN.write(connected);    // Bajt 2
            CAN.write(move);         // Bajt 3
            CAN.write((const uint8_t*)&pos, 2);   // Bajt 4-5
            CAN.write((const uint8_t*)&speed, 2); // Bajt 6-7
            CAN.endPacket();
            // Krótka przerwa zabezpieczająca przed przepełnieniem buforów CAN
            delay(1); 
            // Budowanie Ramki B
            CAN.beginPacket(0x04);
            CAN.write(servoIndex);   // Bajt 0: Numer serwa
            CAN.write(1);            // Bajt 1: Typ = Ramka B
            CAN.write((const uint8_t*)&load, 2);    // Bajt 2-3
            CAN.write((const uint8_t*)&current, 2); // Bajt 4-5
            CAN.write(volt);         // Bajt 6
            CAN.write(temp);         // Bajt 7
            CAN.endPacket();
        }
        */
        
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
        // --- Wysyłanie do Serw ST3025 (Węzeł ID 4) ---
        // Zakładamy, że 4 serwa z ID4 odpowiadają przegubom joint3, joint4, joint5, joint6 (indeksy 2, 3, 4, 5 w target_joints)
        for (uint8_t i = 0; i < 4; i++) {
            CAN.beginPacket(JOINT_4_CMD_ID);
            
            // Konwersja radianów z MQTT na jednostki serwa ST3025. 
            // UWAGA: Musisz dostosować mnożnik do rozdzielczości swojego serwa (często to 4096 kroków na 360 stopni)
            // Przykład: target_joints podaje radiany
            float radians = target_joints[i + 3]; // Przesunięcie indeksu, np. zaczynamy od joint3
            int16_t target_pos_steps = (int16_t)(radians * (180.0 / PI) * (4096.0 / 360.0)); 
            
            int16_t speed = (int16_t)target_speeds[i + 3];
            
            CAN.write(i); // Bajt 0: Do którego z 4 serw to trafia (0-3)
            CAN.write((const uint8_t*)&target_pos_steps, 2); // Bajt 1-2: Pozycja
            CAN.write((const uint8_t*)&speed, 2);            // Bajt 3-4: Prędkość
            CAN.endPacket();
        }
    }
    
}
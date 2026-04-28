#include <Arduino.h>
#include <CAN.h>
#include <SCServo.h>

SMS_STS st; 

// ==========================================
// KONFIGURACJA PINÓW I ID
// ==========================================
#define RX_CAN_PIN   3
#define TX_CAN_PIN   1
#define RX_SERVO_PIN 16
#define TX_SERVO_PIN 17

#define COMMAND_ID   0x14  
#define TELEMETRY_ID 0x04  

uint8_t servo_ids[4] = {1, 2, 3, 4}; 

// Zmienne 'volatile' (Pamięć buforowa między siecią a silnikami)
// ESP32 jest układem 32-bitowym, więc operacje na int16 są absolutnie
// atomowe i nie potrzebują żadnych dławiących Mutexów!
volatile int16_t target_pos[4]  = {2048, 2048, 2048, 2048};
volatile uint16_t target_spd[4] = {1500, 1500, 1500, 1500}; 

volatile int16_t cached_pos[4]   = {0, 0, 0, 0};
volatile int16_t cached_speed[4] = {0, 0, 0, 0};

void TaskServo(void *pvParameters);
void TaskCan(void *pvParameters);

void setup() {
    Serial2.begin(1000000, SERIAL_8N1, RX_SERVO_PIN, TX_SERVO_PIN);
    st.pSerial = &Serial2;
    delay(500);

    CAN.setPins(RX_CAN_PIN, TX_CAN_PIN);
    while (!CAN.begin(500E3)) { delay(100); }

    // Rdzeń 0 - Obsługa UART serw (Izolowany od zakłóceń)
    xTaskCreatePinnedToCore(TaskServo, "Servo", 4096, NULL, 1, NULL, 0);
    // Rdzeń 1 - Obsługa CAN (Odbiera i nadaje ile sił w procesorze)
    xTaskCreatePinnedToCore(TaskCan, "Can", 4096, NULL, 2, NULL, 1);
}

void loop() {
    vTaskDelete(NULL); // Zabijamy loop, wszystko robi FreeRTOS
}

// ==========================================
// WĄTEK 1: SERWA (Rdzeń 0) - HARD REAL-TIME
// ==========================================
void TaskServo(void *pvParameters) {
    // 1. KLUCZ DO PŁYNNOŚCI: Sprzętowy profil krzywej ruchu (Wartości 20-60)
    uint8_t acc[4] = {40, 40, 40, 40}; 
    int16_t last_sent_pos[4] = {-1, -1, -1, -1};
    uint8_t read_idx = 0;

    // Pobranie pierwszej pozycji (zapobiega strzałowi silnika przy starcie)
    for(int i=0; i<4; i++) {
        int16_t p = st.ReadPos(servo_ids[i]);
        if(p != -1) {
            target_pos[i] = p;
            last_sent_pos[i] = p;
        }
    }

    // 2. SZTYWNY METRONOM (Zegar 25 Hz / co 40 ms)
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 40 / portTICK_PERIOD_MS; 

    for(;;) { 
        // Ta funkcja gwarantuje uśpienie na dokładnie 40ms, bez dryftu czasowego!
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        int16_t p[4]; uint16_t s[4];
        bool move_needed = false;

        // Błyskawiczny odczyt celów zapisanych przez wątek sieciowy (CAN)
        for(int i=0; i<4; i++) {
            p[i] = target_pos[i];
            s[i] = target_spd[i];
            
            // 3. DEAD-BAND (Filtracja zakłóceń). 
            // Jeśli zmiana wynosi mniej niż 3 kroki (~0.2 stopnia), ignoruj - to szum pada
            if (abs(p[i] - last_sent_pos[i]) > 3) {
                move_needed = true;
            }
        }

        if (move_needed) {
            // Serwo dostaje nową trajektorię ze stałą, równą częstotliwością
            // Ma dokładnie 40ms na wykonanie obrotu do momentu kolejnej poprawki
            st.SyncWritePosEx(servo_ids, 4, p, s, acc);
            
            // Zapamiętanie, gdzie wysłaliśmy ramię
            for(int i=0; i<4; i++) last_sent_pos[i] = p[i];
        } 
        else {
            // Gdy ramię nie dostało nowych współrzędnych (lub się zatrzymało), 
            // mamy czas wolny na sprawdzenie telemetrii jednego z serw
            uint8_t s_id = servo_ids[read_idx];
            int16_t cp = st.ReadPos(s_id);
            if (cp != -1) {
                cached_pos[read_idx] = cp;
                cached_speed[read_idx] = st.ReadSpeed(s_id);
            }
            read_idx++;
            if (read_idx >= 4) read_idx = 0; 
        }
    }
}

// ==========================================
// WĄTEK 2: BEZKOLIZYJNA SIEĆ (Rdzeń 1)
// ==========================================
void TaskCan(void *pvParameters) {
    unsigned long last_tx = 0;
    uint8_t tx_idx = 0;

    for(;;) {
        // 1. ODBIÓR Z CAN 
        // Odbiera wszystko jak leci i natychmiast wrzuca do ramu.
        int packetSize = CAN.parsePacket();
        if (packetSize) {
            if (CAN.packetId() == COMMAND_ID && packetSize >= 5) {
                uint8_t s_idx = CAN.read(); 
                
                if (s_idx < 4) {
                    uint8_t buf[4];
                    for (int i = 0; i < 4; i++) buf[i] = CAN.read(); 
                    
                    int16_t pos, speed;
                    memcpy(&pos, &buf[0], 2);
                    memcpy(&speed, &buf[2], 2);

                    if (pos < 0) pos = 0;
                    if (pos > 4095) pos = 4095;

                    int16_t actual_speed = map(speed, 0, 100, 0, 2400); 
                    if (actual_speed <= 0) actual_speed = 1500; 

                    // Wrzut bezpośrednio do tablicy (atomowo, nadpisuje stare wartości)
                    target_pos[s_idx] = pos;
                    target_spd[s_idx] = actual_speed;
                }
            }
        }

        // 2. WYSYŁANIE TELEMETRII (Co 15 ms)
        if (millis() - last_tx >= 15) {
            last_tx = millis();
            
            CAN.beginPacket(TELEMETRY_ID);
            CAN.write(tx_idx);                             
            CAN.write((const uint8_t*)&cached_pos[tx_idx], 2);   
            CAN.write((const uint8_t*)&cached_speed[tx_idx], 2); 
            CAN.endPacket();

            tx_idx++;
            if (tx_idx >= 4) tx_idx = 0;
        }

        vTaskDelay(1 / portTICK_PERIOD_MS); 
    }
}
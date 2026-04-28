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

// Współdzielone bufory danych (Atomowe dla ESP32)
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

    // Rdzeń 0 - Izolowana obsługa UART serw
    xTaskCreatePinnedToCore(TaskServo, "Servo", 4096, NULL, 1, NULL, 0);
    // Rdzeń 1 - Komunikacja CAN (Ping-Pong)
    xTaskCreatePinnedToCore(TaskCan, "Can", 4096, NULL, 2, NULL, 1);
}

void loop() {
    vTaskDelete(NULL); 
}

// ==========================================
// WĄTEK 1: SERWA (Rdzeń 0)
// ==========================================
void TaskServo(void *pvParameters) {
    uint8_t acc[4] = {40, 40, 40, 40}; // Profil krzywej sprzętowej
    int16_t last_sent_pos[4] = {-1, -1, -1, -1};
    uint8_t read_idx = 0;

    // Pobranie pierwszej pozycji
    for(int i=0; i<4; i++) {
        int16_t p = st.ReadPos(servo_ids[i]);
        if(p != -1) {
            target_pos[i] = p;
            last_sent_pos[i] = p;
            cached_pos[i] = p;
        }
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // Cykl 50 Hz

    for(;;) { 
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        int16_t p[4]; uint16_t s[4];
        bool move_needed = false;

        for(int i=0; i<4; i++) {
            p[i] = target_pos[i];
            s[i] = target_spd[i];
            
            // Strefa martwa (ignorowanie szumu z drążka < 3 kroki)
            if (abs(p[i] - last_sent_pos[i]) > 3) {
                move_needed = true;
            }
        }

        if (move_needed) {
            st.SyncWritePosEx(servo_ids, 4, p, s, acc);
            for(int i=0; i<4; i++) last_sent_pos[i] = p[i];
        } 
        else {
            // Czytanie statusów w tle i zapisywanie do cache, gdy nic nie jedzie
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
// WĄTEK 2: CAN (Rdzeń 1) - SYSTEM "PING-PONG"
// ==========================================
void TaskCan(void *pvParameters) {
    for(;;) {
        // Nasłuchiwanie na paczki od ID 1
        int packetSize = CAN.parsePacket();
        
        if (packetSize && CAN.packetId() == COMMAND_ID && packetSize >= 5) {
            uint8_t s_idx = CAN.read(); 
            
            if (s_idx < 4) {
                // 1. ODCZYT KOMENDY (RX)
                uint8_t buf[4];
                for (int i = 0; i < 4; i++) buf[i] = CAN.read(); 
                
                int16_t pos, speed;
                memcpy(&pos, &buf[0], 2);
                memcpy(&speed, &buf[2], 2);

                if (pos < 0) pos = 0;
                if (pos > 4095) pos = 4095;

                int16_t actual_speed = map(speed, 0, 100, 0, 2400); 
                if (actual_speed <= 0) actual_speed = 1500; 

                // Zapisanie nowego celu dla drugiego rdzenia
                target_pos[s_idx] = pos;
                target_spd[s_idx] = actual_speed;

                // ==========================================
                // 2. NATYCHMIASTOWA ODPOWIEDŹ TELEMETRYCZNA (TX)
                // ==========================================
                // Skoro ID 1 właśnie do nas "zagadało" o serwie s_idx,
                // natychmiast odsyłamy mu z cache status tego samego serwa!
                
                int16_t c_pos = cached_pos[s_idx];
                int16_t c_spd = cached_speed[s_idx];

                CAN.beginPacket(TELEMETRY_ID);
                CAN.write(s_idx);                             
                CAN.write((const uint8_t*)&c_pos, 2);   
                CAN.write((const uint8_t*)&c_spd, 2); 
                CAN.endPacket();
            }
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS); 
    }
}
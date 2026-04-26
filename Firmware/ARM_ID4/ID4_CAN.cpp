#include "ID4_CAN.h"
#include <CAN.h>
#include <SCServo.h>
#include "Pins.h"
#include "Config.h"

SMS_STS st; // Obiekt do sterowania serwami STS
unsigned long lastCanRxTime = 0;
unsigned long lastTelemetryTime = 0;

void initCanAndServos() {
    Serial.println("[ID4_CAN] Inicjalizacja magistrali serw...");
    // Inicjalizacja sprzętowego UART dla serw na Serial1
    Serial1.begin(1000000, SERIAL_8N1, Pins::RX_SERVO_PIN, Pins::TX_SERVO_PIN);
    st.pSerial = &Serial1;
    delay(500);

    Serial.println("[ID4_CAN] Inicjalizacja CAN...");
    CAN.setPins(Pins::RX_CAN_PIN, Pins::TX_CAN_PIN);
    if (!CAN.begin(CAN_BAUDRATE)) {
        Serial.println("[ID4_CAN] BŁĄD: Inicjalizacja CAN nie powiodła się!");
        // Zatrzymanie systemu w przypadku braku CAN
        while (1) { delay(1000); } 
    }
    Serial.println("[ID4_CAN] CAN uruchomiony.");
}

void handleCanAndServos() {
    // ==========================================
    // 1. ODBIÓR KOMEND Z MAGISTRALI CAN
    // ==========================================
    int packetSize = CAN.parsePacket();
    if (packetSize) {
        uint32_t receivedId = CAN.packetId();
        
        // Oczekujemy: [Bajt 0: Index Serwa] + [2 Bajty: Pozycja] + [2 Bajty: Prędkość]
        if (receivedId == COMMAND_ID && packetSize >= 5) {
            uint8_t s_idx = CAN.read();
            
            if (s_idx < 4) {
                int16_t target_pos = 0;
                int16_t target_speed = 0;
                
                uint8_t buf[4];
                for (int i = 0; i < 4; i++) buf[i] = CAN.read();
                
                memcpy(&target_pos, &buf[0], 2);
                memcpy(&target_speed, &buf[2], 2);

                // Zapisz pozycję i prędkość do serwa (domyślne przyspieszenie 50)
                st.WritePosEx(SERVO_IDS[s_idx], target_pos, target_speed, 50);
                
                lastCanRxTime = millis(); // Zapisz czas odebrania komendy
            }
        }
    }

    // ==========================================
    // 2. WYSYŁANIE TELEMETRII (Feedback dla ID 1) Co 50ms
    // ==========================================
    if (millis() - lastTelemetryTime >= 50) {
        lastTelemetryTime = millis();

        for (int i = 0; i < 4; i++) {
            uint8_t s_id = SERVO_IDS[i];
            
            // Odczyt z biblioteki SCServo
            bool connected   = (st.Ping(s_id) != -1);
            int  moveStatus  = st.ReadMove(s_id);
            bool move        = (moveStatus == 1);
            int16_t pos      = st.ReadPos(s_id);
            int16_t speed    = st.ReadSpeed(s_id);
            int16_t load     = st.ReadLoad(s_id);
            int16_t current  = st.ReadCurrent(s_id);
            uint8_t voltage  = st.ReadVoltage(s_id);
            uint8_t temper   = st.ReadTemper(s_id);

            // WYSYŁANIE RAMKI "A"
            CAN.beginPacket(TELEMETRY_ID);
            CAN.write(i);                             // Bajt 0: Indeks serwa (0-3)
            CAN.write(0);                             // Bajt 1: Typ = Ramka A (0)
            CAN.write(connected);                     // Bajt 2
            CAN.write(move);                          // Bajt 3
            CAN.write((const uint8_t*)&pos, 2);       // Bajt 4-5
            CAN.write((const uint8_t*)&speed, 2);     // Bajt 6-7
            CAN.endPacket();

            delay(2); 

            // WYSYŁANIE RAMKI "B"
            CAN.beginPacket(TELEMETRY_ID);
            CAN.write(i);                             // Bajt 0: Indeks serwa (0-3)
            CAN.write(1);                             // Bajt 1: Typ = Ramka B (1)
            CAN.write((const uint8_t*)&load, 2);      // Bajt 2-3
            CAN.write((const uint8_t*)&current, 2);   // Bajt 4-5
            CAN.write(voltage);                       // Bajt 6
            CAN.write(temper);                        // Bajt 7
            CAN.endPacket();

            delay(2); 
        }
    }
}

// Sprawdza, czy od ostatniej komendy minęło mniej niż 500 ms
bool isReceivingCommands() {
    return (millis() - lastCanRxTime < 500);
}
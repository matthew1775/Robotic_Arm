#include <Arduino.h>
#include <Adafruit_NeoPixel.h> // <--- Biblioteka NeoPixel
#include "Config.h"
#include "pins.h"
#include "Network.h"
#include "ARM_CAN.h"

// --- Konfiguracja NeoPixel ---
#define NUMPIXELS 1 // Liczba diod (zmień jeśli masz pasek)
Adafruit_NeoPixel pixels(NUMPIXELS, Pins::NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastMqttCmdTime = 0;

// Zmienna do śledzenia poprzedniego stanu MQTT (żeby nie odświeżać diody ciągle)
bool lastMqttState = false; 
bool isFirstLoop = true; // Zmienna upewniająca się, że odświeżymy LED po restarcie

void setup() {
  Serial.begin(115200);
  Pins::init_pins();
  
  // 1. Inicjalizacja NeoPixel i ustawienie na FIOLETOWY
  pixels.begin();
  pixels.setBrightness(50); // Ustawienie jasności (0-255)
  pixels.setPixelColor(0, pixels.Color(128, 0, 128)); // Fioletowy
  pixels.show();
  Serial.println("[SYSTEM] Start konfiguracji...");
  
  // 2. Inicjalizacja modułów
  initNetwork();
  initCAN();

  // 3. Ustawienie koloru ŻÓŁTEGO po zakończeniu inicjalizacji
  pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Żółty
  pixels.show();
  Serial.println("[SYSTEM] Konfiguracja zakończona!");
  
  // Krótkie opóźnienie, abyś zdążył zauważyć żółty kolor (zanim pętla zmieni go na czerwony z powodu braku MQTT)
  delay(1000); 
}

void loop() {
  handleNetwork(); // Obsługa MQTT (odbieranie/łączenie)
  handleCAN();     // Nasłuch i cykliczne wysyłanie na CAN

  // ==========================================
  // OBSŁUGA STATUSU NEOPIXEL
  // ==========================================
  bool currentMqttState = isMqttConnected();
  
  // Zmień kolor LED TYLKO przy zmianie stanu (lub w pierwszej pętli)
  if (currentMqttState != lastMqttState || isFirstLoop) {
      lastMqttState = currentMqttState;
      isFirstLoop = false;
      
      if (currentMqttState) {
          pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // ZIELONY - MQTT Połączone
          Serial.println("[LED] Status: MQTT Połączone (Zielony)");
      } else {
          pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // CZERWONY - Brak MQTT
          Serial.println("[LED] Status: MQTT Rozłączone (Czerwony)");
      }
      pixels.show(); // Wyślij dane do diody
  }

  // ==========================================
  // WYSYŁANIE TELEMETRII CO 200ms
  // ==========================================
  static unsigned long lastPub = 0;
  if (millis() - lastPub > 200) {
    lastPub = millis();
    
    // Odczyt z pinów jako przykład telemetrii
    sensA = analogRead(Pins::SENS_A_PIN) * (2.0 / 4095.0);
    
    sendFeedbackMessage(); // Wysyła dane na MQTT (jeśli jest podłączone)
  }
}
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Pins.h"
#include "Config.h"
#include "ID4_CAN.h"

// Inicjalizacja diody NeoPixel
Adafruit_NeoPixel pixels(1, Pins::NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    Serial.println("[SYSTEM] Uruchamianie węzła ID 4...");

    // 1. Konfiguracja i zapalenie NeoPixel na czerwono (Stan początkowy)
    pixels.begin();
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();

    // 2. Inicjalizacja modułów CAN i Serw
    initCanAndServos();

    Serial.println("[SYSTEM] Konfiguracja zakończona.");
}

void loop() {
    // 1. Obsługa zadań cyklicznych serw i magistrali CAN
    handleCanAndServos();

    // 2. Obsługa statusu LED na podstawie aktywności CAN
    static bool wasActive = false;
    bool isActive = isReceivingCommands();

    // Zmieniamy kolor tylko, jeśli stan się zmienił (zabezpiecza przed migotaniem)
    if (isActive != wasActive) {
        wasActive = isActive;
        if (isActive) {
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Zielony (odbiera dane)
        } else {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Czerwony (brak komend)
        }
        pixels.show();
    }
}
#ifndef ID4_CAN_H
#define ID4_CAN_H

#include <Arduino.h>

// Deklaracje funkcji
void initCanAndServos();
void handleCanAndServos();

// Funkcja zwracająca status połączenia dla głównego pliku (do obsługi LED)
bool isReceivingCommands();

#endif
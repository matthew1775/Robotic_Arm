#include "MotorControl.h"

// Zmienne statyczne dla pamięci regulatora PID
static float integral = 0.0;
static float previous_error = 0.0;
static unsigned long last_time = 0;

// Nastawy PID 
constexpr float Kp = 15.0; // Wzmocnienie proporcjonalne
constexpr float Ki = 0.5;  // Wzmocnienie całkujące
constexpr float Kd = 2.0;  // Wzmocnienie różniczkujące

void handleLocalMotor() {
    unsigned long current_time = millis();
    float dt = (current_time - last_time) / 1000.0; // Różnica czasu w sekundach
    
    // Zabezpieczenie przed dzieleniem przez 0 na samym początku
    if (dt <= 0.0) return; 
    last_time = current_time;

    // 1. Odczyt i mapowanie (jak poprzednio)
    int potValue = analogRead(Pins::POT_1_PIN);
    potValue = constrain(potValue, POT1_MIN_ADC, POT1_MAX_ADC);
    float currentAngle = (float)(potValue - POT1_MIN_ADC) * POT1_MAX_ANGLE / (POT1_MAX_ADC - POT1_MIN_ADC);
    
    float targetAngle = target_joints[0] * (180.0 / PI);
    targetAngle = constrain(targetAngle, 0.0, POT1_MAX_ANGLE);
    
    // 2. Obliczenie błędu (uchybu)
    float error = targetAngle - currentAngle;
    
    // 3. OBLICZENIA PID
    // Część całkująca (z zabezpieczeniem Anti-Windup - zatrzymujemy narastanie w granicach)
    integral += (error * dt);
    integral = constrain(integral, -100.0, 100.0); // Zabezpieczenie przed "nakręcaniem się"
    
    // Część różniczkująca
    float derivative = (error - previous_error) / dt;
    previous_error = error;
    
    // Równanie PID (Wyjście to żądany PWM z kierunkiem na podstawie znaku)
    float pid_output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    
    // 4. Integracja z limitami prędkości z MQTT
    // target_speeds jest z zakresu 0-100, konwertujemy na maksymalny limit PWM (0-1023)
    int max_allowed_pwm = map(target_speeds[0], 0, 100, 0, 1023);
    max_allowed_pwm = constrain(max_allowed_pwm, 0, 1023);

    // 5. Wysterowanie silnika
    if (abs(error) > ANGLE_TOLERANCE) {
        bool moveForward = (pid_output > 0);
        if (MOTOR1_DIRECTION == 1) moveForward = !moveForward;
        
        // Absolutna wartość mocy do przekazania na piny PWM (ograniczona zadaną prędkością z MQTT)
        int final_pwm = min((int)abs(pid_output), max_allowed_pwm);
        
        if (moveForward) {
            digitalWrite(Pins::IN_POS_PIN, HIGH);
            digitalWrite(Pins::IN_NEG_PIN, LOW);
        } else {
            digitalWrite(Pins::IN_POS_PIN, LOW);
            digitalWrite(Pins::IN_NEG_PIN, HIGH);
        }
        analogWrite(Pins::ENABLE_PIN, final_pwm);
    } else {
        // Jesteśmy w celu - resetujemy całkę i wyłączamy silnik
        integral = 0; 
        digitalWrite(Pins::IN_POS_PIN, LOW);
        digitalWrite(Pins::IN_NEG_PIN, LOW);
        analogWrite(Pins::ENABLE_PIN, 0);
    }
}
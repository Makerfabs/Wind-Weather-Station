
#include "LowPower.h"

#define BUZZER_PIN 12
#define VCC_PIN 2
#define VDD_PIN 7
#define LED_PIN 11

void setup()
{
    SerialUSB.begin(115200);
    pinMode(VDD_PIN, OUTPUT);
    pinMode(VCC_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(VDD_PIN, LOW);
    delay(500);
    digitalWrite(VDD_PIN, HIGH);

    digitalWrite(VCC_PIN, LOW);
    delay(500);
    digitalWrite(VCC_PIN, HIGH);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);

    digitalWrite(VDD_PIN, LOW);
    delay(100);
    digitalWrite(VCC_PIN, LOW);
    delay(100);

    SerialUSB.println("CPU Sleep");
    delay(1000);

    // Detach USB interface
    //USBDevice.detach();
    // Enter standby mode
    //LowPower.standby();
    // Attach USB interface
}

void loop()
{
    SerialUSB.println("loop");
    delay(1000);
}
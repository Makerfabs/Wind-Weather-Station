#include <stdio.h>
#include <string.h>

//Description: input AT commands via serial monitor to learn how to use A9/A9G module
//version:v1.0
//Author:Charlin
//web: http://www.makerfabs.com
//

#define DEBUG true
#define PWR_KEY 9
#define RST_KEY 6
#define LOW_PWR_KEY 5

#define VCC_PIN 2
#define VDD_PIN 7
#define BUZZER_PIN 12
#define SD_CS 4
#define pinInterrupt A0
#define PM_READ_PIN A1
#define PM_LED_PIN 10
#define DHTPIN 13

bool ModuleState = false;

void setup()
{
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(VDD_PIN, OUTPUT);
    pinMode(VCC_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PM_LED_PIN, OUTPUT);
    pinMode(pinInterrupt, INPUT_PULLUP);
    pinMode(PWR_KEY, OUTPUT);
    pinMode(RST_KEY, OUTPUT);
    pinMode(LOW_PWR_KEY, OUTPUT);

    digitalWrite(VDD_PIN, LOW);
    delay(500);
    digitalWrite(VDD_PIN, HIGH);

    digitalWrite(VCC_PIN, LOW);
    delay(500);
    digitalWrite(VCC_PIN, HIGH);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(RST_KEY, LOW);
    digitalWrite(LOW_PWR_KEY, HIGH);
    digitalWrite(PWR_KEY, HIGH);
    
    digitalWrite(PWR_KEY, LOW);
    delay(3000);
    digitalWrite(PWR_KEY, HIGH);
    delay(10000);

    ModuleState = moduleStateCheck();
    if (ModuleState == false) //if it's off, turn on it.
    {
        digitalWrite(PWR_KEY, LOW);
        delay(3000);
        digitalWrite(PWR_KEY, HIGH);
        delay(10000);
        SerialUSB.println("Now turnning the A9/A9G on.");
    }

    sendData("AT+CCID", 3000, DEBUG);
    //sendData("AT+CREG?", 3000, DEBUG);
    //sendData("AT+CGATT=1", 1000, DEBUG);
    //sendData("AT+CGACT=1,1", 1000, DEBUG);
    //sendData("AT+CGDCONT=1,\"IP\",\"CMNET\"", 1000, DEBUG);

    //sendData("AT+CIPSTART=\"TCP\",\"www.mirocast.com\",80", 2000, DEBUG);
    SerialUSB.println("Maduino A9/A9G Test Begin!");
}

void loop()
{
    while (Serial1.available() > 0)
    {
        SerialUSB.write(Serial1.read());
        yield();
    }
    while (SerialUSB.available() > 0)
    {
        Serial1.write(SerialUSB.read());
        yield();
    }
}

bool moduleStateCheck()
{
    int i = 0;
    bool moduleState = false;
    for (i = 0; i < 5; i++)
    {
        String msg = String("");
        msg = sendData("AT", 1000, DEBUG);
        if (msg.indexOf("OK") >= 0)
        {
            SerialUSB.println("A9/A9G Module had turned on.");
            moduleState = true;
            return moduleState;
        }
        delay(1000);
    }
    return moduleState;
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial1.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }
    if (debug)
    {
        SerialUSB.print(response);
    }
    return response;
}
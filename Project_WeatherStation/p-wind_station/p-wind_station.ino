#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#include "sensor.h"
#include "ArduinoLowPower.h"

#define DEBUG true
#define VCC_PIN 2 //3.3V
#define VDD_PIN 7 //5V
#define PWR_KEY 9 //A9G
#define BUZZER_PIN 12

#define pinInterrupt A0 // the number of the Wind Speed sensor pin

#define buttonPin 3 // the number of the pushbutton pin

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Sensor_value s_v = {0, 0, 0, 0, 0};

int Count = 0;
int runtime = 0;
int before_sleep = 0;
int sensortime = 0;
int show_index = 0;
int reset_count = 0;

int pm25_value = 0;

bool ModuleState = false;

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 1000; // the debounce time; increase if the output flickers

void setup()
{
    SerialUSB.begin(115200);
    SerialUSB.println("---------------Start----------------------");

    Serial1.begin(115200);

    pin_init();
    SerialUSB.println("Pin init over");

    delay(10000);

    // //PM2.5
    // for (int i = 0; i < 10; i++)
    // {
    //     pm25_value = pm25();
    //     delay(3000);
    // }

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    SerialUSB.println("SSD1306 found");

    sensor_init();

    logo_show();

    wind_count_start();
    get_weather();
    wind_count_end();

    while (0)
        ;

    at_commands_init();
    at_commands_send();
    before_sleep = millis();
}

// typedef struct
// {
//     int humidity;
//     int temperature;
//     int speed;
//     int direct;
//     int pressure_value;
// } Sensor_value;

void loop()
{
    SerialUSB.println("loop");
    while (Serial1.available() > 0)
    {
        SerialUSB.write(Serial1.read());
        yield();
    }
    delay(3000);
    if (millis() - before_sleep >= 1000L)
    { // Execute every TBD ms
        before_sleep = millis();
        SerialUSB.println("Prepare to sleep");
        pin_closeall();
        LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
        LowPower.sleep(60000);
    }
}
void pin_init()
{
    pinMode(VDD_PIN, OUTPUT);
    pinMode(VCC_PIN, OUTPUT);
    pinMode(PWR_KEY, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PM_LED_PIN, OUTPUT);

    pinMode(pinInterrupt, INPUT_PULLUP); //set as input pin

    digitalWrite(VDD_PIN, LOW);
    delay(500);
    digitalWrite(VDD_PIN, HIGH);

    digitalWrite(VCC_PIN, LOW);
    delay(500);
    digitalWrite(VCC_PIN, HIGH);

    //restart a9g
    digitalWrite(PWR_KEY, LOW);
    delay(3000);
    digitalWrite(PWR_KEY, HIGH);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);

    delay(1000);
}

void pin_closeall()
{
    digitalWrite(VDD_PIN, LOW);
    digitalWrite(VCC_PIN, LOW);
    digitalWrite(PWR_KEY, LOW);

    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);

    SerialUSB.println("Close all power");

    delay(1000);
}

void wind_count_start()
{
    attachInterrupt(digitalPinToInterrupt(pinInterrupt), onChange, FALLING);
}

void wind_count_end()
{
    detachInterrupt(digitalPinToInterrupt(pinInterrupt));
}

void logo_show()
{
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2); // Draw 2X-scale text
    display.setCursor(10, 0);
    display.println(F("Makerfabs"));
    display.setTextSize(1);
    display.setCursor(10, 16);
    display.println(F("Weather Station"));
    display.display(); // Show initial text
    delay(100);
}

void sensor_show()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);

    switch (show_index)
    {
    case 0:

        display.setCursor(0, 0);
        display.print("Temp:");
        display.print(s_v.temperature / 10.0);
        display.println(" C");

        display.setCursor(0, 16);
        display.print("Humi:");
        display.print(s_v.humidity);
        display.println(" %");
        break;

    case 1:
        display.setCursor(0, 0);
        display.print("Dir:");
        display.print(s_v.direct);
        display.println("");

        display.setCursor(0, 16);
        display.print("Speed:");
        display.print(s_v.speed / 10.0);
        display.println(" m/s");
        break;

    case 2:
        display.setCursor(0, 0);
        display.println("Pressure:");

        display.setCursor(0, 16);
        display.print(s_v.pressure_value);
        display.println(" Pa");
        break;
    }

    display.display(); // Show initial text
    show_index++;
    show_index %= 3;
}

void onChange()
{
    if (digitalRead(pinInterrupt) == LOW)
    {
        SerialUSB.println("onChange");
        Count++;
    }
}

void wind_speed()
{
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        lastDebounceTime = millis();
        //0.1m/s
        s_v.speed = (int)(Count * 8.75 * 0.01 * 10);
        // SerialUSB.print(s_v.speed);
        Count = 0;
        // SerialUSB.println("m/s");
    }
}

//cost at least 30 seconds
void get_weather()
{
    long get_weather_time = millis();
    while (1)
    {
        if ((millis() - get_weather_time) > 30000)
        {
            SerialUSB.println("Already get weather");
            return;
        }

        wind_speed();

        if ((millis() - sensortime) > 4000)
        {
            SerialUSB.println("Read sensor");
            sensor_read(&s_v);
            sensortime = millis();
        }

        if ((millis() - runtime) > 2000)
        {
            sensor_show();
            runtime = millis();
        }
    }
}

void at_commands_init()
{
    SerialUSB.println("Start send at init");
    while (Serial1.available() > 0)
    {
        SerialUSB.write(Serial1.read());
        yield();
    }
    sendData("AT", 3000, DEBUG);
    sendData("AT+CCID", 3000, DEBUG);
    sendData("AT+CREG?", 3000, DEBUG);
    sendData("AT+CGATT=1", 3000, DEBUG);

    sendData("AT+CGDCONT=1,\"IP\",\"CMNET\"", 3000, DEBUG);
    sendData("AT+CGACT=1,1", 3000, DEBUG);
    sendData("AT+MQTTCONN=\"test.mosquitto.org\",1883,\"mqttx_0931852d35\",120,0", 3000, DEBUG);

    /* For test
    sendData("AT+HTTPGET=\"https://api.thingspeak.com/update?api_key=5F9US450RPYGDBWZ&field1=1&field2=2&field3=3&field4=4&field5=5\"", 3000, DEBUG);
    sendData("AT+MQTTCONN=\"test.mosquitto.org\",1883,\"mqttx_0931852d35\",120,0", 3000, DEBUG);
    String topic_T = "AT+MQTTPUB=\"/public/TEST/makerfabs-T\",\"" + (String)1234567890 + "\",0,0,0";
    sendData(topic_T, 3000, DEBUG);
    */

    SerialUSB.println("At init over");
}

void at_commands_send()
{
    SerialUSB.println("Start send at-command");
    // h t s d p

    String mqtt_str = (String) "h" + s_v.humidity + "t" + s_v.temperature +
                      "s" + s_v.speed + "d" + s_v.direct + "p" + s_v.pressure_value + "q" + s_v.pm25;
    String topic_T = "AT+MQTTPUB=\"/public/TEST/makerfabs-T\",\"" + mqtt_str + "\",0,0,0";
    sendData(topic_T, 10000, DEBUG);

    String temperature_str = (String) "" + s_v.temperature / 10 + "." + s_v.temperature % 10;
    String windspeed_str = (String) "" + s_v.speed / 10 + "." + s_v.speed % 10;

    //NO WIND DIR
    s_v.direct = 0;

    String thing_speak_str = (String) "&field1=" + s_v.humidity + "&field2=" + temperature_str +
                             "&field3=" + windspeed_str + "&field4=" + s_v.direct +
                             "&field5=" + s_v.pressure_value + "&field6=" + s_v.pm25;
    //sendData("AT+HTTPGET=\"https://api.thingspeak.com/update?api_key=5F9US450RPYGDBWZ&field1=1&field2=2&field3=3&field4=4&field5=5\"", 3000, DEBUG);
    sendData("AT+HTTPGET=\"https://api.thingspeak.com/update?api_key=5F9US450RPYGDBWZ" + thing_speak_str + "\"", 10000, DEBUG);

    SerialUSB.println("End send at-command");
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

void dummy()
{
    // This function will be called once on device wakeup
    // You can do some little operations here (like changing variables which will be used in the loop)
    // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
    reset_count++;
    if (reset_count > 30)
        NVIC_SystemReset();
}
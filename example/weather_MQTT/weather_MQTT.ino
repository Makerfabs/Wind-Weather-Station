#include <stdio.h>
#include <string.h>

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SD.h>



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


//#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

bool ModuleState = false;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BMP280_I2C_ADDRESS  0x76
#define BMP280_DEVICE_ID    0x58
Adafruit_BMP280 bmp; // I2C

int humidity_value = 0;
int temperature_value = 0;
int speed_value = 0;
int pressure_value = 0;
int show_index = 0;

int Count = 0;
int runtime = 0;
int sensortime = 0;
int dhttime = 0;
int Mqtttime = 0;
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 1000; // the debounce time; increase if the output flickers



void setup()
{
    SerialUSB.begin(115200);
    Serial1.begin(115200);

    pinMode(VDD_PIN, OUTPUT);
    pinMode(VCC_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PM_LED_PIN, OUTPUT);
    pinMode(pinInterrupt, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pinInterrupt), onChange, FALLING);

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
    sendData("AT+CREG?", 3000, DEBUG);
    sendData("AT+CGATT=1", 1000, DEBUG);
    
    sendData("AT+CGDCONT=1,\"IP\",\"CMNET\"", 1000, DEBUG);
    sendData("AT+CGACT=1,1", 1000, DEBUG);
    sendData("AT+MQTTCONN=\"test.mosquitto.org\",1883,\"mqttx_0931852d35\",120,0", 1000, DEBUG);
    String topic_W = "AT+MQTTPUB=\"/public/TEST/makerfabs-W\",\""+(String)30+"\",0,0,0";
    String topic_T = "AT+MQTTPUB=\"/public/TEST/makerfabs-T\",\""+(String)73.5+"\",0,0,0";
    String topic_H = "AT+MQTTPUB=\"/public/TEST/makerfabs-H\",\""+(String)46.8+"\",0,0,0";
    //String topic_P = "AT+MQTTPUB=\"/public/TEST/makerfabs-P\",\""+(String)pressure_value+"\",0,0,0";
    //String topic_H = "AT+MQTTPUB=\"/public/TEST/makerfabs-H\",\""+(String)h+"\",0,0,0";

    SerialUSB.println(topic_T);
    sendData(topic_T, 1000, DEBUG);
    SerialUSB.println(topic_H);

    i2c_dev_init();
  logo_show();

  dht.begin();

}

void loop()
{
    wind_speed();
      
    if ((millis() - sensortime) > 1000)
    {   
      SerialUSB.println("Read bmp");
      bmp_read();
      //i2c_scan();
      sensortime = millis();
    }
      
    //DHT is slow
    if ((millis() - dhttime) > 4000)
    {
      SerialUSB.println("Read dht");
      dht_read();
      dhttime = millis();
    }
      
    if ((millis() - runtime) > 2000)
    {
      sensor_show();
      runtime = millis();
    }  
    
    if ((millis() - Mqtttime) > 16000)
    {
      String topic_P = "AT+MQTTPUB=\"/public/TEST/makerfabs-P\",\""+(String)pressure_value+"\",0,0,0";
      String topic_W = "AT+MQTTPUB=\"/public/TEST/makerfabs-W\",\""+(String)speed_value+"\",0,0,0";
      String topic_T = "AT+MQTTPUB=\"/public/TEST/makerfabs-T\",\""+(String)temperature_value+"\",0,0,0";
      String topic_H = "AT+MQTTPUB=\"/public/TEST/makerfabs-H\",\""+(String)humidity_value+"\",0,0,0";
      sendData(topic_T, 1000, DEBUG);
      SerialUSB.println("send T");
      sendData(topic_H, 1000, DEBUG);
      sendData(topic_W, 1000, DEBUG);
      sendData(topic_P, 1000, DEBUG);
      Mqtttime = millis();
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


void i2c_scan()
{
  Wire.begin();
  byte error, address;
  int nDevices;

  static int s = 0;
  SerialUSB.print(s++);
  SerialUSB.println(". Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      SerialUSB.print("I2C device found at address 0x");
      if (address < 16)
        SerialUSB.print("0");
      SerialUSB.print(address, HEX);
      SerialUSB.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      SerialUSB.print("Unknown error at address 0x");
      if (address < 16)
        SerialUSB.print("0");
      SerialUSB.println(address, HEX);
    }
  }
  if (nDevices == 0)
    SerialUSB.println("No I2C devices found\n");
  else
  {
    SerialUSB.print(">>>> Found total ");
    ;
    SerialUSB.print(nDevices);
    SerialUSB.println(" devices\n");
  }
}

void dht_read()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    SerialUSB.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  SerialUSB.print(F("Humidity: "));
  SerialUSB.print(h);
  SerialUSB.print(F("%  Temperature: "));
  SerialUSB.print(t);
  SerialUSB.print(F("°C "));
  SerialUSB.print(f);
  SerialUSB.print(F("°F  Heat index: "));
  SerialUSB.print(hic);
  SerialUSB.print(F("°C "));
  SerialUSB.print(hif);
  SerialUSB.println(F("°F"));

  humidity_value = (int)h;
  temperature_value = (int)t;
}

void bmp_read()
{
  SerialUSB.print(F("Temperature = "));
  SerialUSB.print(bmp.readTemperature());
  SerialUSB.println(" *C");

  SerialUSB.print(F("Pressure = "));
  SerialUSB.print(pressure_value = bmp.readPressure());
  SerialUSB.println(" Pa");

  SerialUSB.print(F("Approx altitude = "));
  SerialUSB.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
  SerialUSB.println(" m");

  SerialUSB.println();
}

void i2c_dev_init()
{

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  SerialUSB.println("SSD1306 found");

  if (!bmp.begin(BMP280_I2C_ADDRESS, BMP280_DEVICE_ID))
  {
    SerialUSB.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1)
      ;
  }
  SerialUSB.println("BMP280 found");

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  SerialUSB.println("BMP280 init over");
  //i2c_scan();
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

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x01);
  delay(4000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
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
    display.print(temperature_value);
    display.println(" C");

    display.setCursor(0, 16);
    display.print("Humi:");
    display.print(humidity_value);
    display.println(" %");
    break;

  case 1:
    display.setCursor(0, 0);
    display.print("Humi:");
    display.print(humidity_value);
    display.println(" %");

    display.setCursor(0, 16);
    display.print("Wind:");
    display.print(speed_value);
    display.println(" m/s");
    break;

  case 2:
    display.setCursor(0, 0);
    display.print("PM2.5:");

    display.setCursor(0, 16);
    display.print(pm25());
    display.println(" mg/m3");
    break;

  case 3:
    display.setCursor(0, 0);
    display.println("Pressure:");

    display.setCursor(0, 16);
    display.print(pressure_value);
    display.println(" Pa");
    break;
  }

  display.display(); // Show initial text
  show_index++;
  show_index %= 4;
}


void wind_speed()
{
  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    lastDebounceTime = millis();
    speed_value = Count * 8.75 * 0.01;
    SerialUSB.print(speed_value);
    Count = 0;
    SerialUSB.println("m/s");
  }
}

int pm25()
{
  int samplingTime = 280;
  int deltaTime = 40;
  int sleepTime = 9680;

  float voMeasured = 0;
  float calcVoltage = 0;
  float dustDensity = 0;
  digitalWrite(PM_LED_PIN, HIGH); // power on the LED
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(PM_READ_PIN); // read the dust value   
  voMeasured = voMeasured*2.0/3.0;//5V to 3.3V
  delayMicroseconds(deltaTime);
  digitalWrite(PM_LED_PIN, LOW); // turn the LED off
  delayMicroseconds(sleepTime);

  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);

  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 170 * calcVoltage - 0.1;

  //SerialUSB.println(dustDensity); // unit: ug/m3
  if(dustDensity < 5)
      dustDensity = 0;
  return (int)dustDensity;
}

void onChange()
{
  if (digitalRead(pinInterrupt) == LOW)
    Count++;
  //      Serial.println("Key Down");
  //   else
  //      Serial.println("Key UP");
}

#include "sensor.h"

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

void sensor_init()
{
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

    dht.begin();
    SerialUSB.println("DHT11 init over");
}

void sensor_read(Sensor_value *s_v)
{
    s_v->humidity = dht_humi();
    s_v->temperature = dht_temp();
    s_v->pressure_value = bmp_pa();
    s_v->direct = wind_dir();
    //s_v->pm25 = pm25();
}

int bmp_pa()
{
    return bmp.readPressure();
}

int dht_temp()
{
    float t = dht.readTemperature();
    return (int)(t * 10.0);
}

int dht_humi()
{
    float h = dht.readHumidity();
    return (int)h;
}

int wind_dir()
{
    int adc_value = analogRead(A1);
    SerialUSB.print("Direct:");
    SerialUSB.println(adc_value);
    return (adc_value + 70) / 140;
}

float pm25()
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
    SerialUSB.print("PM AD0=");
    SerialUSB.println(voMeasured);       //
    voMeasured = voMeasured * 2.0 / 3.0; //5V to 3.3V   average is a better way
    delayMicroseconds(deltaTime);
    digitalWrite(PM_LED_PIN, LOW); // turn the LED off
    delayMicroseconds(sleepTime);

    // if (voMeasured < 10)
    //     return 0;
    // 0 - 5V mapped to 0 - 1023 integer values
    // recover voltage
    calcVoltage = voMeasured * (5.0 / 1024.0);

    // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
    // Chris Nafis (c) 2012
    dustDensity = 170 * calcVoltage - 0.1;

    SerialUSB.print(dustDensity); // unit: ug/m3
    SerialUSB.println("ug/m3");
    return dustDensity;
}
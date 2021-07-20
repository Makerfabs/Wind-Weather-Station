#include <DHT.h>
#include <Adafruit_BMP280.h>

#define BMP280_I2C_ADDRESS 0x76
#define BMP280_DEVICE_ID 0x58

#define DHTPIN 13 // the number of the DHT11 pin
#define DHTTYPE DHT11

#define DIRECT_PIN A1

#define PM_READ_PIN A1
#define PM_LED_PIN 10

typedef struct
{
    int humidity;
    int temperature;
    int speed;
    int direct;
    int pressure_value;
    float pm25;
} Sensor_value;

void sensor_init();
void sensor_read(Sensor_value *s_v);
void dht_init();
int dht_temp();
int dht_humi();

int bmp_pa();
int wind_dir();
float pm25();
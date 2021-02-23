# Wind Weather Station

```c++
/*
Version:		V1.0
Author:			Vincent
Create Date:	2021/2/23
Note:
	
*/
```
[toc]

# Makerfabs

[Makerfabs home page](https://www.makerfabs.com/)

[Makerfabs Wiki](https://makerfabs.com/wiki/index.php?title=Main_Page)

# Wind Weather Station

## Intruduce

Weather station based on Maduino A9G.With onboard DHT11 temperature and humidity sensor, BMP280 air pressure sensor and SSD1306LCD screen.It can be connected with anemometer and PM2.5 air quality sensor.

## Feature

- Base on Maduino A9G
- SSD1306 128*32 LCD Screen
- DHT-11 humidity and temperature sensor
- BMP280 high-precision and low-power digital barometer
- High precision anemometer
- GP2Y1014AU Air Quality Sensor For PM2.5

### Front:

![front](md_pic/front.jpg)

### Back:
![back](md_pic/back.jpg)



# Example

## Compiler Options

![complier](md_pic/complier.jpg)

Choice Arduino Zero (Native USB Port).

## weather_show_default

Factory default program, collect temperature, humidity, air pressure, wind speed, PM2.5 index, the sensor data display on SSD1306.

- Connect anemometer or air quality sensor (or both) to weather station.
- Connect to battery or usb cable.
- LCD screen will show sensor value.




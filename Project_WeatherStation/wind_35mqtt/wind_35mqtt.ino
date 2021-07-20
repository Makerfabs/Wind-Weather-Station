#include <WiFi.h>
#include <PubSubClient.h>
#include "alert.h"    // Out of range alert icon
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#define TFT_GREY 0x2104 // Dark grey 16 bit colour

typedef struct
{
  int humidity;
  int temperature;
  int speed;
  int direct;
  int pressure_value;
} Sensor_value;

Sensor_value s_v = {0, 0, 0, 0, 0};
String dir_str[8] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

//WIFI
const char *ssid = "Makerfabs";
const char *password = "20160704";
const char *mqtt_server = "test.mosquitto.org"; //你的服务器地址

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height
WiFiClient espClient;
PubSubClient client(espClient);

uint32_t runTime = -99999; // time for next update
int value = 0;
int d = 0; // Variable used for the sinewave test waveform
boolean range_error = 0;

String msg = "h0t0s0d0p0";

void setup(void)
{
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Makerfabs Weather Station Display", 10, 10, 4);
  //tft.drawString("Wind Speed", 350, 10, 2);

  setup_wifi();
  client.setServer(mqtt_server, 1883); //MQTT默认端口是1883
  client.setCallback(callback);
}

void loop()
{
  mqtt_loop();
  if (millis() - runTime >= 5000L)
  { // Execute every TBD ms
    runTime = millis();

    // Set the the position, gap between meters, and inner radius of the meters
    int xpos = 20, ypos = 40, gap = 20, radius = 56;

    xpos = gap + ringMeter(s_v.humidity, 0, 100, xpos, ypos, radius, "%", GREEN2RED);           // Draw analogue meter
    xpos = gap + ringMeter(s_v.temperature, -100, 500, xpos, ypos, radius, "0.1 degC", BLUE2RED); // Draw analogue meter

    xpos = 20, ypos = 200;

    xpos = gap + ringMeter(s_v.speed, 0, 10, xpos, ypos, radius, "m/s", BLUE2BLUE);               // Draw analogue meter
    xpos = gap + ringMeter(s_v.pressure_value, 8000, 12000, xpos, ypos, radius, "pa", BLUE2BLUE); // Draw analogue meter

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Wind Dir", 360, 80, 4);
    tft.drawString(dir_str[s_v.direct], 360, 120, 4);
  }
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option

  x += r;
  y += r; // Calculate coords of centre of ring

  int w = r / 3; // Width of outer ring is 1/4 of radius

  int angle = 150; // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;

  // Draw colour blocks every inc degrees
  for (int i = -angle + inc / 2; i < angle - inc / 2; i += inc)
  {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v)
    { // Fill in coloured segments with 2 triangles
      switch (scheme)
      {
      case 0:
        colour = TFT_RED;
        break; // Fixed colour
      case 1:
        colour = TFT_GREEN;
        break; // Fixed colour
      case 2:
        colour = TFT_BLUE;
        break; // Fixed colour
      case 3:
        colour = rainbow(map(i, -angle, angle, 0, 127));
        break; // Full spectrum blue to red
      case 4:
        colour = rainbow(map(i, -angle, angle, 70, 127));
        break; // Green to red (high temperature etc)
      case 5:
        colour = rainbow(map(i, -angle, angle, 127, 63));
        break; // Red to green (low battery etc)
      default:
        colour = TFT_BLUE;
        break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 3;
  if (value > 999)
    len = 5;
  dtostrf(value, len, 0, buf);
  buf[len] = ' ';
  buf[len + 1] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(1);

  if (value < vmin || value > vmax)
  {
    drawAlert(x, y + 90, 50, 1);
  }
  else
  {
    drawAlert(x, y + 90, 50, 0);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(colour, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  if (r > 84)
  {
    tft.setTextPadding(55 * 3);   // Allow for 3 digits each 55 pixels wide
    tft.drawString(buf, x, y, 8); // Value in middle
  }
  else
  {
    tft.setTextPadding(3 * 14);   // Allow for 3 digits each 14 pixels wide
    tft.drawString(buf, x, y, 4); // Value in middle
  }
  tft.setTextSize(1);
  tft.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (r > 84)
    tft.drawString(units, x, y + 60, 4); // Units display
  else
    tft.drawString(units, x, y + 15, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

void drawAlert(int x, int y, int side, boolean draw)
{
  if (draw && !range_error)
  {
    drawIcon(alert, x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight);
    range_error = 1;
  }
  else if (!draw)
  {
    tft.fillRect(x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight, TFT_BLACK);
    range_error = 0;
  }
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0;   // Red is the top 5 bits of a 16 bit colour value
  byte green = 0; // Green is the middle 6 bits
  byte blue = 0;  // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0)
  {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1)
  {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2)
  {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3)
  {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase)
{
  return sin(phase * 0.0174532925);
}

//====================================================================================
// This is the function to draw the icon stored as an array in program memory (FLASH)
//====================================================================================

// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64

// Draw array "icon" of defined width and height at coordinate x,y
// Maximum icon size is 255x255 pixels to avoid integer overflow

void drawIcon(const unsigned short *icon, int16_t x, int16_t y, int8_t width, int8_t height)
{

  uint16_t pix_buffer[BUFF_SIZE]; // Pixel buffer (16 bits per pixel)

  tft.startWrite();

  // Set up a window the right size to stream pixels into
  tft.setAddrWindow(x, y, width, height);

  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++)
  {
    for (int j = 0; j < BUFF_SIZE; j++)
    {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np)
  {
    for (int i = 0; i < np; i++)
      pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }

  tft.endWrite();
}

void setup_wifi()
{

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
} //链接WiFi

void callback(char *topic, byte *payload, unsigned int length)
{
  msg = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    msg += (char)payload[i];
    //Serial.print((char)payload[i]);
  } //串口打印出收到的信息
  Serial.println(msg);
  decode_msg(msg);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Makerfabs-";          //该板子的链接名称
    clientId += String(random(0xffff), HEX); //产生一个随机数字 以免多块板子重名
    //尝试连接
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // 连接后，发布公告......
      client.publish("/public/TEST/makerfabs-T", "Weather Display Online"); //链接成功后 会发布这个主题和语句
      // ......并订阅
      client.subscribe("/public/TEST/makerfabs-T"); //这个是你让板子订阅的主题（接受该主题的消息）
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 如果链接失败 等待五分钟重新链接
      delay(5000);
    }
  }
}

void mqtt_loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}

void decode_msg(String msg)
{
  int h = msg.indexOf('h');
  int t = msg.indexOf('t');
  int s = msg.indexOf('s');
  int d = msg.indexOf('d');
  int p = msg.indexOf('p');
  Serial.println(s_v.humidity = msg.substring(h + 1, t).toInt());
  Serial.println(s_v.temperature = msg.substring(t + 1, s).toInt());
  Serial.println(s_v.speed = msg.substring(s + 1, d).toInt());
  Serial.println(s_v.direct = msg.substring(d + 1, p).toInt());
  Serial.println(s_v.pressure_value = msg.substring(p + 1).toInt());
}
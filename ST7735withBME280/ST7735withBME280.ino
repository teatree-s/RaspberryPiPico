/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
// #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <SPI.h>
#include<Adafruit_BME280.h>

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS         17
#define TFT_RST        20 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         16

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// For 1.14", 1.3", 1.54", 1.69", and 2.0" TFT with ST7789:
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
#define TFT_MOSI 19  // Data out
#define TFT_SCLK 18  // Clock out

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// OR for the ST7789-based displays, we will use this call
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Adafruit_BME280 bme; // I2C
#define SEALEVELPRESSURE_HPA (1013.25)

#define ST77XX_GRAY 0x3186
#define ST7735_BG_COLOR ST77XX_GRAY
#define ST7735_TEXT_COLOR ST77XX_BLACK

#define TFT_BTN 10
static bool digitalDisplay = false;

void setup(void) {
  Serial.begin(9600);

  Serial.print(F("Hello! ST77xx TFT Test"));

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(2);

  pinMode(TFT_BTN, INPUT_PULLUP);

  {
    // while(!Serial);    // time to get serial running
    Serial.println(F("BME280 test"));

    unsigned status;
    // default settings
    status = bme.begin(0x76);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
  }

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
  Serial.println(time, DEC);

  Serial.println("done");
  delay(1000);

  initDisplay(digitalDisplay);
}

void loop() {
  if (LOW == digitalRead(TFT_BTN)) {
    digitalDisplay = !digitalDisplay;
    initDisplay(digitalDisplay);
  } else {
    updateDisplay(digitalDisplay);
  }

  printValues();
  Serial.println("loop");
  delay(1000);
}

void initDisplay(bool digitalDisplay) {
  if (digitalDisplay) {
    initDigitalDisplay();
    updateDigitalDisplay(true);
  } else {
    initColorDisplay();
    updateColorDisplay(true);
  }
}

void updateDisplay(bool digitalDisplay) {
  if (digitalDisplay) {
    updateDigitalDisplay(false);
  } else {
    updateColorDisplay(false);
  }
}

void initColorDisplay() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(0, 18);
  tft.println("Temperature");
  tft.setCursor(0, 70);
  tft.println("Humidity");
  tft.setCursor(0, 122);
  tft.println("Pressure");
  tft.setCursor(90, 152);
  tft.println("hPa");

  tft.setFont(&FreeSans18pt7b);
}

void updateColorDisplay(bool update) {
  static String lastTemp(0.0, 1);
  float temp = bme.readTemperature();
  String strTemp(temp, 1);
  if ((lastTemp != strTemp)||update) {
    tft.fillRect(0, 24, 76, 34, ST77XX_BLACK);
    tft.setCursor(10, 50);
    tft.setTextColor(getTemperatureTextColor(temp));
    tft.println(strTemp+" °C");
    lastTemp = strTemp;
  }

  static String lastHum(0.0, 1);
  float hum = bme.readHumidity();
  String strHum(hum, 1);
  if ((lastHum != strHum)||update) {
    tft.fillRect(0, 76, 76, 34, ST77XX_BLACK);
    tft.setCursor(10, 102);
    tft.setTextColor(getHumidityTextColor(hum));
    tft.println(strHum+" %");
    lastHum = strHum;
  }

  static String lastPres(1000);
  int pres = (int)(bme.readPressure() / 100.0F);
  String strPres(pres);
  if ((lastPres != strPres)||update) {
    tft.fillRect(0, 124, 86, 34, ST77XX_BLACK);
    tft.setCursor(10, 152);
    tft.setTextColor(ST77XX_WHITE);
    tft.println(strPres);
    lastPres = strPres;
  }
}

uint16_t getTemperatureTextColor(float temp) {
  uint16_t color = ST77XX_WHITE;
  if (temp < 10.0)      { color = 0x7fff; } //#7fffff
  else if (temp < 20.0) { color = 0x7ff7; } //#7fffbf
  else if (temp < 30.0) { color = 0x7fef; } //#7fff7f
  else                  { color = 0xfdef; } //#ffbf7f
  return color;
}

uint16_t getHumidityTextColor(float hum) {
  uint16_t color = ST77XX_WHITE;
  if (hum < 20.0)      { color = 0xfdef; } //#ffbf7f
  else if (hum < 40.0) { color = 0x7fef; } //#7fff7f
  else if (hum < 60.0) { color = 0x7fff; } //#7fffff
  else                 { color = 0x7dff; } //#7fbfff
  return color;
}

void initDigitalDisplay() {
  tft.fillScreen(ST7735_BG_COLOR);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ST7735_TEXT_COLOR);

  tft.setCursor(0, 14);
  tft.println("Temperature");
  tft.setCursor(0, 94);
  tft.println("Humidity");
}

void updateDigitalDisplay(bool update) {
  int16_t x = 10;
  int16_t y = 20;

  float val = bme.readTemperature();
  int num1 = ((int)val / 10) % 10;
  int num2 = (int)val % 10;
  int num3 = (int)(val*10)%10;

  draw7SEG(x,y,num1);
  draw7SEG(x+34,y,num2);
  tft.fillRect(70, y+49, 4, 4, ST7735_TEXT_COLOR);
  draw7SEG(x+72,y,num3);
  tft.setCursor(x+98, y+50);
  tft.println("C");

  x = 10;
  y = 100;

  val = bme.readHumidity();
  num1 = ((int)val / 10) % 10;
  num2 = (int)val % 10;
  num3 = (int)(val*10)%10;

  draw7SEG(x,y,num1);
  draw7SEG(x+34,y,num2);
  tft.fillRect(70, y+49, 4, 4, ST7735_TEXT_COLOR);
  draw7SEG(x+72,y,num3);
  tft.setCursor(x+98, y+50);
  tft.println("%");
}

void draw7SEG(int16_t x, int16_t y, uint8_t num) {
  static uint8_t seg[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x6F};
  uint16_t color;

  color = (seg[num]&(0x01<<0)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawHorizontalLine(x, y, color);
  color = (seg[num]&(0x01<<1)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawVerticalLine(x+17, y+6, color);
  color = (seg[num]&(0x01<<2)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawVerticalLine(x+17, y+29, color);
  color = (seg[num]&(0x01<<3)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawHorizontalLine(x, y+46, color);
  color = (seg[num]&(0x01<<4)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawVerticalLine(x-7, y+29, color);
  color = (seg[num]&(0x01<<5)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawVerticalLine(x-7, y+6, color);
  color = (seg[num]&(0x01<<6)) ? (ST7735_TEXT_COLOR) : (ST7735_BG_COLOR);
  drawHorizontalLine(x, y+23, color);
}

void drawHorizontalLine(int16_t x, int16_t y, uint16_t color) {
  tft.fillRect(x, y, 16, 7, color);
  tft.fillTriangle(x, y, x, y+6, x-3, y+2, color);
  x += 16;
  tft.fillTriangle(x, y, x, y+6, x+3, y+2, color);
}

void drawVerticalLine(int16_t x, int16_t y, uint16_t color) {
  tft.fillRect(x, y, 7, 16, color);
  tft.fillTriangle(x, y, x+6, y, x+3, y-2, color);
  y += 16;
  tft.fillTriangle(x, y, x+6, y, x+3, y+2, color);
}

void printValues() {
  Serial.println();

  Serial.printf("%2.1fC\n", analogReadTemp());

  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("TFT_BTN = ");
  Serial.println(digitalRead(TFT_BTN));
}

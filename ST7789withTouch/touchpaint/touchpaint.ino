/***************************************************
  This is our touchscreen painting example for ST7789 and XPT2046.
  reference ----> http://www.adafruit.com/products/1651

  MIT license
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ST7789.h>
#include "XPT2046_Touchscreen.h"

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 450
#define TS_MINY 350
#define TS_MAXX 3800
#define TS_MAXY 3900

// default SPI PIN
//#define PIN_SPI_MISO  (16u)
//#define PIN_SPI_MOSI  (19u)
//#define PIN_SPI_SCK   (18u)
//#define PIN_SPI_SS    (17u)

#define TS_CS PIN_SPI_SS
#define TS_IRQ 20

//XPT2046_Touchscreen ts(TS_CS);
//XPT2046_Touchscreen ts(TS_CS);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(TS_CS, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 2
#define TFT_RST 1  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 0
#define TFT_MOSI 7  // Data out
#define TFT_SCLK 6  // Clock out
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 30
#define PENRADIUS 2
int oldcolor, currentcolor;

void setup(void) {
//  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));
  
  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  tft.init(240, 320);           // Init ST7789 320x240
  tft.invertDisplay(false);

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;
  Serial.println(time, DEC);
//  tft.setRotation(1);
  delay(500);
  
  // make the color selection boxes
  tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ST77XX_RED);
  tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ST77XX_YELLOW);
  tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, ST77XX_GREEN);
  tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, ST77XX_CYAN);
  tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, ST77XX_BLUE);
  tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, ST77XX_MAGENTA);
  tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
  tft.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, ST77XX_BLACK);
 
  // select the current color 'red'
  tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
  currentcolor = ST77XX_RED;

  if (!ts.begin(SPI)) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  ts.setRotation(2);

  Serial.println("Touchscreen started");
}


void loop()
{
  // See if there's any  touch data for us
  if (ts.bufferEmpty()) {
    return;
  }

  // You can also wait for a touch
  if (!ts.tirqTouched()) {
    if (!ts.touched()) {
      return;
    }
  }

  // Retrieve a point  
  TS_Point p = ts.getPoint();
  
//  Serial.print("X = "); Serial.print(p.x);
//  Serial.print("\tY = "); Serial.print(p.y);
//  Serial.print("\tPressure = "); Serial.println(p.z);  
 
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  /*
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");
  */

  if (p.y < BOXSIZE) {
    oldcolor = currentcolor;
    
    if (p.x < BOXSIZE) { 
      currentcolor = ST77XX_RED; 
      tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*2) {
      currentcolor = ST77XX_YELLOW;
      tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*3) {
      currentcolor = ST77XX_GREEN;
      tft.drawRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*4) {
      currentcolor = ST77XX_CYAN;
      tft.drawRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*5) {
      currentcolor = ST77XX_BLUE;
      tft.drawRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*6) {
      currentcolor = ST77XX_MAGENTA;
      tft.drawRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*7) {
      currentcolor = ST77XX_WHITE;
      tft.drawRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
    } else if (p.x < BOXSIZE*8) {
      currentcolor = ST77XX_BLACK;
      tft.drawRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, ST77XX_WHITE);
      tft.fillRect(0, BOXSIZE, 240, 320, ST77XX_BLACK);
    }

    if (oldcolor != currentcolor) {
      if (oldcolor == ST77XX_RED) 
        tft.fillRect(0, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_YELLOW) 
        tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_GREEN) 
        tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_CYAN) 
        tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_BLUE) 
        tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_MAGENTA) 
        tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_WHITE) 
        tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, oldcolor);
      if (oldcolor == ST77XX_BLACK) 
        tft.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, oldcolor);
    }
  }

  if (((p.y-PENRADIUS) > BOXSIZE) && ((p.y+PENRADIUS) < tft.height())) {
    tft.fillCircle(p.x, p.y, PENRADIUS, currentcolor);
  }
}

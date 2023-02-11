
/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <ArduinoJson.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "ESP32_SPIFFS_UTF8toSJIS.h"
#include "ShinonomeFONTread.h"
#include "UTF8toSJIS.h"

// TFT
#define TFT_CS 2
#define TFT_RST 1
#define TFT_DC 0
//#define SD_CS 4  // Chip select line for SD card

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Font
#define TXT_MAX 1000
const char* UTF8SJIS_file = "/Utf8Sjis.tbl";
UTF8toSJIS u8ts;
const char* ZenkakuFontFile = "/shnmk16.bdf";  // 全角フォントファイル名を定義
const char* HalfFontFile = "/shnm8x16r.bdf";  // 半角フォントファイル名を定義
ShinonomeFONTread SFR;

void dispShinonomeFont(uint8_t x0, uint8_t y0, String strUTF8, uint16_t color=ST7735_WHITE);
void dispShinonomeFontNews(uint8_t x0, uint8_t y0, String strUTF8, uint16_t color=ST7735_WHITE);

// WiFi
#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

WiFiMulti WiFiMulti;

// Japan News API
// https://newsapi.org/s/japan-news-api
// change XXXXXXXXXX to your API key
const char* NewsAPIURL = "http://newsapi.org/v2/top-headlines?country=jp&apiKey=XXXXXXXXXX";

void setup(void) {
  Serial.begin(9600);

  // TFT
  Serial.println(F("Hello! ST77xx TFT Test"));

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);

  Serial.println(F("Initialized"));

  tft.setRotation(3);
  tft.setFont(&FreeSerif9pt7b);
  //  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  LittleFS.begin();

  // WiFi
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  bool ret = WiFiMulti.addAP(ssid, pass);
  Serial.println(ret);

  // Draw Bitmap
  dispBitmap();
  delay(10000UL);

  Serial.println("done");
}

StaticJsonDocument<1024*30> doc;

void loop() {
  Serial.println("loop");
  static int count = 0;

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;
    String payload = "";

    Serial.print("[HTTP] begin...\n");
    if (http.begin(NewsAPIURL)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
    
    if (payload.length()) {
      deserializeJson(doc, payload);

      tft.fillScreen(ST77XX_BLACK);

      int line = 0;

      // Japan News API
      dispShinonomeFont(16 * line++, 0, "   Japan News API", ST77XX_CYAN);
      dispShinonomeFont(16 * line++, 0, "--------------------");

      // count
      dispShinonomeFont(0, 0, String(count));

      // publishedAt
      String publishedAt = doc["articles"][0]["publishedAt"];
      publishedAt.setCharAt(10, ' ');
      if (publishedAt.length()) {
        dispShinonomeFont(16 * line++, 0, publishedAt.substring(0, 16));
      }

      // title
      String title = doc["articles"][count]["title"];
      dispShinonomeFontNews(3, 0, title);
    }
  }

  count++;
  if (9 < count) count = 0;

  delay(10000UL);
}

void dispShinonomeFontNews(uint8_t x0, uint8_t y0, String strUTF8, uint16_t color) {
  uint8_t sj_txt[TXT_MAX];        // Shift_JISコード
  uint16_t sj_length;        // Shift_JISコードの長さ
  uint8_t font_buf[TXT_MAX][16];  // 東雲フォントデータ
  uint8_t font_byte[TXT_MAX] = {0};
  uint8_t x = x0;
  uint8_t y = 0;

  u8ts.UTF8_to_SJIS_str_cnv_ex(UTF8SJIS_file, strUTF8, sj_txt, &sj_length, font_byte);
  SFR.SjisToShinonome16FontRead_ALL(ZenkakuFontFile, HalfFontFile, 0, 0, sj_txt, sj_length, font_buf);
  sj_length = 0;
  for (int font_cnt = 0; font_byte[font_cnt] != 0; font_cnt++) {
    if (19 < y + font_byte[font_cnt]) {
      x++;
      y = 0;
    }
    if (7 < x) {
      break;
    }
    
    TFT_16x16_Font_DisplayOut(x*16, y*8, &font_buf[sj_length], font_byte[font_cnt], color);
    sj_length += font_byte[font_cnt];
    y += font_byte[font_cnt];
  }
}

void dispShinonomeFont(uint8_t x0, uint8_t y0, String strUTF8, uint16_t color) {
  uint8_t sj_txt[32];        // Shift_JISコード
  uint16_t sj_length;        // Shift_JISコードの長さ
  uint8_t font_buf[32][16];  // 東雲フォントデータ

  u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, strUTF8, sj_txt, &sj_length);
  SFR.SjisToShinonome16FontRead_ALL(ZenkakuFontFile, HalfFontFile, 0, 0, sj_txt,
                                    sj_length, font_buf);
  TFT_16x16_Font_DisplayOut(x0, y0, font_buf, sj_length, color);
}

//********* TFT 16x16フォント出力 ********************
void TFT_16x16_Font_DisplayOut(uint8_t x0, uint8_t y0, uint8_t Fnt[][16],
                               uint8_t txtMax, uint16_t color) {
  int i, j, k;
  uint8_t bt = 0b10000000;

  if (txtMax > TXT_MAX) txtMax = TXT_MAX;

  for (j = 0; j < txtMax; j++) {
    for (i = 0; i < 16; i++) {
      for (k = 0; k < 8; k++) {
        if (Fnt[j][i] & (bt >> k)) {
          int x = y0 + k + j * 8;
          int y = x0 + i;
          tft.drawPixel(x, y, color);
        } else {
        }
      }
    }
  }
}

void dispBitmap(void) {
  File file = LittleFS.open("/sample.bmp", "r");
  if (file) {
    bmpDraw(file, 0, 0);
    file.close();
  }
}

#define BUFFPIXEL 20

void bmpDraw(File bmpFile, uint8_t x, uint8_t y) {
  int bmpWidth, bmpHeight;          // W+H in pixels
  uint8_t bmpDepth;                 // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;          // Start of image data in file
  uint32_t rowSize;                 // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];  // pixel buffer (R+G+B per pixel)
  uint8_t buffidx = sizeof(sdbuffer);  // Current position in sdbuffer
  boolean goodBmp = false;             // Set to true on valid header parse
  boolean flip = true;                 // BMP is stored bottom-to-top
  int w, h, row, col;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) {  // BMP signature
    Serial.print("File size: ");
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile);             // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);  // Start of image data
    Serial.print("Image Offset: ");
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: ");
    Serial.println(read32(bmpFile));
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) {    // # planes -- must be '1'
      bmpDepth = read16(bmpFile);  // bits per pixel
      Serial.print("Bit Depth: ");
      Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) {  // 0 = uncompressed

        goodBmp = true;  // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width()) w = tft.width() - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.startWrite();
        tft.setAddrWindow(x, y, w, h);

        for (row = 0; row < h; row++) {  // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip)  // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else  // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) {  // Need seek?
            tft.endWrite();
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer);  // Force buffer reload
          }

          for (col = 0; col < w; col++) {  // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) {  // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0;  // Set index to beginning
              tft.startWrite();
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r, g, b));
          }  // end pixel
        }    // end scanline
        tft.endWrite();
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      }  // end goodBmp
    }
  }

  if (!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t*)&result)[0] = f.read();  // LSB
  ((uint8_t*)&result)[1] = f.read();  // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t*)&result)[0] = f.read();  // LSB
  ((uint8_t*)&result)[1] = f.read();
  ((uint8_t*)&result)[2] = f.read();
  ((uint8_t*)&result)[3] = f.read();  // MSB
  return result;
}

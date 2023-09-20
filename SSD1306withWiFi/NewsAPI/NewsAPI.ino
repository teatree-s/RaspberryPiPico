/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "ESP32_SPIFFS_MisakiFNT.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

// for Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_SDA_PIN   4
#define OLED_SCL_PIN   5
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

// for Font
#define TXT_MAX 1000
const char* UTF8SJIS_file = "/Utf8Sjis.tbl"; //UTF8 Shift_JIS 変換テーブルファイル名を記載しておく
const char* Misaki_Zen_Font_file = "/MSKG_13.FNT"; //全角フォントファイル名を定義
const char* Misaki_Half_Font_file = "/mgotec48.FNT"; //半角フォントファイル名を定義
ESP32_SPIFFS_MisakiFNT MFR;
UTF8toSJIS u8ts;

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
StaticJsonDocument<1024*64> doc;

void setup() {
  Serial.begin(9600);
  Serial.println("Start...");

  // for Display
  Wire.setSDA(OLED_SDA_PIN);
  Wire.setSCL(OLED_SCL_PIN);
  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(3000); // Pause

  // Clear the buffer
  display.clearDisplay();
  Serial.println("display.clearDisplay");

  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

  testdrawchar();      // Draw characters of the default font
  delay(3000); // Pause

  // for Font
  MFR.SPIFFS_Misaki_Init3F(UTF8SJIS_file, Misaki_Half_Font_file, Misaki_Zen_Font_file);
  
  // for WiFi
  bool ret = WiFiMulti.addAP(ssid, pass);
  Serial.println(ret);

  Serial.println("Done...");
}

void loop() {
  Serial.println("loop...");
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
      display.clearDisplay();
      deserializeJson(doc, payload);

      // Japan News API
      dispMisakiFont(0, 0, "　　　Ｊａｐａｎ　Ｎｅｗｓ　　　");
      dispMisakiFont(8, 0, "－－－－－－－－－－－－－－－－");

      // publishedAt
      String publishedAt = doc["articles"][0]["publishedAt"];
      publishedAt.setCharAt(10, ' ');
      if (publishedAt.length()) {
        dispMisakiFont(16, 0, publishedAt.substring(0, 16));
      }

      // title
      String title = doc["articles"][count]["title"];
      dispMisakiFontNews(3, 0, title);
      display.display();
    }
  }

  count++;
  if (9 < count) count = 0;
  delay(5000);
}

// for Display
void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
}

void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}

// for Font
void SSD1306_8x8_Font_DisplayOut(uint8_t x0, uint8_t y0, uint8_t Fnt[][8], uint8_t txtMax){
  int i, j, k;
  uint8_t bt = 0b10000000;

  if( txtMax > 16) txtMax = 16;

  for(j=0; j<txtMax; j++){
    for(i=0; i<8; i++){
      for(k=0; k<8; k++){
        if( Fnt[j][i] & (bt >> k) ){
          display.drawPixel(y0+k+j*8, x0+i, SSD1306_WHITE);
        } else {
        }
      }
    }
  }
}

void dispMisakiFont(uint8_t x0, uint8_t y0, String strUTF8) {
  uint8_t sj_txt[32];           // Shift_JISコード
  uint16_t sj_length;           // Shift_JISコードの長さ
  uint8_t font_buf[16][8]={0};  // 美咲フォントデータ

  u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, strUTF8, sj_txt, &sj_length);
  MFR.Sjis_To_MisakiFNT_Read_ALL(sj_txt, sj_length, font_buf);
  SSD1306_8x8_Font_DisplayOut(x0, y0, font_buf, sj_length);
}

void dispMisakiFontNews(uint8_t x0, uint8_t y0, String strUTF8) {
  uint8_t sj_txt[TXT_MAX];
  uint16_t sj_txt_length;
  uint16_t sj_length, sj_all_length;
  uint8_t font_byte[TXT_MAX]={0};
  uint8_t x = x0;
  uint8_t y = 0;

  u8ts.UTF8_to_SJIS_str_cnv_ex(UTF8SJIS_file, strUTF8, sj_txt, &sj_txt_length, font_byte);
  sj_length = 0;
  sj_all_length = 0;
  for (int font_cnt = 0; font_byte[font_cnt] != 0; font_cnt++) {
    uint8_t font_buf[16][8]={0};
    if (32 < sj_length + font_byte[font_cnt]) {
      MFR.Sjis_To_MisakiFNT_Read_ALL(&sj_txt[sj_all_length], sj_length, font_buf);
      SSD1306_8x8_Font_DisplayOut(x*8, 0, font_buf, sj_length);
      sj_all_length += sj_length;
      sj_length = 0;
      x++;
    }
    if (7 < x) {
      break;
    }
    sj_length += font_byte[font_cnt];
  }

  if ((sj_length + sj_all_length) == sj_txt_length) {
    uint8_t font_buf[16][8]={0};
    MFR.Sjis_To_MisakiFNT_Read_ALL(&sj_txt[sj_all_length], sj_length, font_buf);
    SSD1306_8x8_Font_DisplayOut(x*8, 0, font_buf, sj_length);
  }
}

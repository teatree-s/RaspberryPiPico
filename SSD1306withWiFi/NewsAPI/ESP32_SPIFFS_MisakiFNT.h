/*
  ESP32_SPIFFS_MisakiFNT.h - for ESP-WROOM-32 ( ESP32 )
  Beta version 1.0
  This is the Arduino library for reading Misaki font. (For ESP32)

The MIT License (MIT)

Copyright (c) 2017 Mgo-tec
Blog URL ---> https://www.mgo-tec.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ESP32_SPIFFS_MISAKIFNT_h_
#define ESP32_SPIFFS_MISAKIFNT_h_
#include "Arduino.h"
#include "LittleFS.h"
#include "UTF8toSJIS.h"

class ESP32_SPIFFS_MisakiFNT {
 public:
  ESP32_SPIFFS_MisakiFNT();

  void SPIFFS_Misaki_Init3F(const char* UTF8SJIS_file,
                            const char* Misaki_Half_Font_file,
                            const char* Misaki_Zen_Font_file);
  void SPIFFS_Misaki_Close3F();
  uint16_t Sjis_To_MisakiFNT_Read_ALL(uint8_t sj_txt[], uint16_t sj_length,
                                      uint8_t buf[][8]);
  uint16_t StrDirect_MisakiFNT_readALL(String str, uint8_t font_buf[][8]);
  void Sjis_To_Misaki_Font_Adrs(File f, uint8_t jisH, uint8_t jisL,
                                uint8_t* buf);
  void SPIFFS_MisakiFontRead(File f1, uint32_t addrs, uint8_t* buf);

 private:
  File _UtoS;
  File _MisakiZ;
  File _MisakiH;

  uint8_t bridge_buf1[8] = {0};
  boolean Zenkaku_bridge = false;

  UTF8toSJIS _u8ts;
};

#endif
/*
  ESP32_SPIFFS_UTF8toSJIS.cpp - Arduino core for the ESP32 Library.
  Beta version 1.2
  
  This is a library for converting from UTF-8 code string to Shift_JIS code string.
  In advance, you need to upload a conversion table file Utf8Sjis.tbl using micro LittleFS & ESP-WROOM-32 ( ESP32 ).
  
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

#ifndef ESP32_SPIFFS_UTF8TOSJIS_H_INCLUDED
#define ESP32_SPIFFS_UTF8TOSJIS_H_INCLUDED
#include <Arduino.h>
#include "LittleFS.h"

extern File _UtoS;

class ESP32_SPIFFS_UTF8toSJIS
{
public:
  ESP32_SPIFFS_UTF8toSJIS();

  void ESP32_SPIFFS_UTF8toSJIS_Init(const char* UTF8SJIS_file);
  void ESP32_SPIFFS_UTF8toSJIS_Close();
  uint16_t UTF8_to_SJIS(String strUTF8, uint8_t* sjis_byte);
  void UTF8_to_SJIS_str_cnv(File UTF8toSjisFile, String strUTF8, uint8_t* sjis_byte, uint16_t* sj_length);
  void UTF8_To_SJIS_code_cnv(uint8_t utf8_1, uint8_t utf8_2, uint8_t utf8_3, uint32_t* spiffs_addrs);
  void SPIFFS_Flash_UTF8SJIS_Table_Read(File UTF8toSjisFile, uint32_t addrs, uint8_t* buf);

private:

};

#endif

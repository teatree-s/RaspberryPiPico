/*
  ShinonomeFONTread.cpp - for ESP-WROOM-02 ( esp8266 )
  Beta version 1.40
  This is the Arduino library for reading Shinonome font. (For ESP8266)

The MIT License (MIT)

Copyright (c) 2016 Mgo-tec
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

FS.h(LittleFS-File system) - Included library
-->https://github.com/esp8266/arduino-esp8266fs-plugin
Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
Released under the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1

Licence of Shinonome Font is Public Domain.
Maintenance development of Font is /efont/.
*/

#include "ShinonomeFONTread.h"

#include "Arduino.h"

ShinonomeFONTread::ShinonomeFONTread() {}

//********東雲フォントライブラリ初期化　２ファイル*************************************************************
void ShinonomeFONTread::SPIFFS_Shinonome_Init2F(
    const char* Shino_Half_Font_file, const char* Shino_Zen_Font_file) {
  LittleFS.begin();

  _SinoH = LittleFS.open(Shino_Half_Font_file, "r");
  if (!_SinoH) {
    Serial.print(Shino_Half_Font_file);
    Serial.print(" File not found");
    return;
  } else {
    Serial.print(Shino_Half_Font_file);
    Serial.println(" File read OK!");
  }
  _SinoZ = LittleFS.open(Shino_Zen_Font_file, "r");
  if (!_SinoZ) {
    Serial.print(Shino_Zen_Font_file);
    Serial.print(" File not found");
    return;
  } else {
    Serial.print(Shino_Zen_Font_file);
    Serial.println(" File read OK!");
  }
}
//*********東雲フォントライブラリ初期化3ファイル*************************************************************
void ShinonomeFONTread::SPIFFS_Shinonome_Init3F(
    const char* UTF8SJIS_file, const char* Shino_Half_Font_file,
    const char* Shino_Zen_Font_file) {
  LittleFS.begin();

  Serial.println("card initialized.");
  __UtoS = LittleFS.open(UTF8SJIS_file, "r");
  if (!__UtoS) {
    Serial.print(UTF8SJIS_file);
    Serial.println(" File not found");
    return;
  } else {
    Serial.print(UTF8SJIS_file);
    Serial.println(" File read OK!");
  }
  _SinoH = LittleFS.open(Shino_Half_Font_file, "r");
  if (!_SinoH) {
    Serial.print(Shino_Half_Font_file);
    Serial.print(" File not found");
    return;
  } else {
    Serial.print(Shino_Half_Font_file);
    Serial.println(" File read OK!");
  }
  _SinoZ = LittleFS.open(Shino_Zen_Font_file, "r");
  if (!_SinoZ) {
    Serial.print(Shino_Zen_Font_file);
    Serial.print(" File not found");
    return;
  } else {
    Serial.print(Shino_Zen_Font_file);
    Serial.println(" File read OK!");
  }
}
//*******************東雲フォントメインクラス*************************************************************
uint8_t ShinonomeFONTread::SjisToShinonome16FontRead(
    const char* FNT_file_16x16, const char* FNT_file_8x16, uint8_t Direction,
    int16_t Angle, uint8_t jisH, uint8_t jisL, uint8_t* buf1, uint8_t* buf2) {
  uint16_t fnt_adrs_half = 0x1346;  // space
  uint32_t fnt_adrs_Zen = 0x467;    // Space
  uint8_t cp;

  if ((jisH >= 0x20 && jisH <= 0x7E) || (jisH >= 0xA1 && jisH <= 0xDF)) {
    if (jisH <= 0x63)
      fnt_adrs_half = 0x1346 + (jisH - 0x20) * 126;
    else if (jisH <= 0x7E)
      fnt_adrs_half = 0x34BF + (jisH - 0x64) * 127;
    else if (jisH >= 0xA1)
      fnt_adrs_half = 0x4226 + (jisH - 0xA1) * 129;

    ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf(FNT_file_8x16,
                                                          fnt_adrs_half, buf1);

    cp = 1;

    if ((jisL >= 0x20 && jisL <= 0x7E) || (jisL >= 0xA1 && jisL <= 0xDF)) {
      if (jisL <= 0x63)
        fnt_adrs_half = 0x1346 + (jisL - 0x20) * 126;
      else if (jisL <= 0x7E)
        fnt_adrs_half = 0x34BF + (jisL - 0x64) * 127;
      else if (jisL >= 0xA1)
        fnt_adrs_half = 0x4226 + (jisL - 0xA1) * 129;

      ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf(
          FNT_file_8x16, fnt_adrs_half, buf2);

      cp = 2;
    } else {
      cp = 1;
    }
  } else {
    ShinonomeFONTread::SjisToShinonomeFNTadrs(jisH, jisL, &fnt_adrs_Zen);
    ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread(FNT_file_16x16,
                                                     fnt_adrs_Zen, buf1, buf2);
    cp = 2;
  }
  return cp;
}
//*************S-jisまとめて変換*****************************************
void ShinonomeFONTread::SjisToShinonome16FontRead_ALL(uint8_t* Sjis,
                                                      uint16_t sj_length,
                                                      uint8_t font_buf[][16]) {
  ShinonomeFONTread::FontRead_ALL_nofile(_SinoZ, _SinoH, Sjis, sj_length,
                                         font_buf);
}
//*************S-jisまとめて変換*****************************************
void ShinonomeFONTread::SjisToShinonome16FontRead_ALL(
    const char* FNT_file_16x16, const char* FNT_file_8x16, uint8_t Direction,
    int16_t Angle, uint8_t* Sjis, uint16_t sj_length, uint8_t font_buf[][16]) {
  File f1 =
      LittleFS.open(FNT_file_16x16, "r");  // 東雲フォント全角ファイル読み込み
  File f2 = LittleFS.open(FNT_file_8x16, "r");  // 東雲フォント半角ファイル読み込み

  ShinonomeFONTread::FontRead_ALL_nofile(f1, f2, Sjis, sj_length, font_buf);

  f1.close();
  f2.close();
}
//*************S-jisまとめて変換*****************************************
void ShinonomeFONTread::FontRead_ALL_nofile(File f1, File f2, uint8_t* Sjis,
                                            uint16_t sj_length,
                                            uint8_t font_buf[][16]) {
  uint16_t fnt_adrs_half = 0x1346;  // space
  uint32_t fnt_adrs_Zen = 0x467;    // Space
  uint16_t i = 0;

  while (i < sj_length) {
    if (Sjis[i] < 0x20) Sjis[i] = 0x20;  // 制御コードは全てスペース
    if ((Sjis[i] >= 0x20 && Sjis[i] <= 0x7E) ||
        (Sjis[i] >= 0xA1 && Sjis[i] <= 0xDF)) {
      if (Sjis[i] <= 0x63)
        fnt_adrs_half = 0x1346 + (Sjis[i] - 0x20) * 126;
      else if (Sjis[i] <= 0x7E)
        fnt_adrs_half = 0x34BF + (Sjis[i] - 0x64) * 127;
      else if (Sjis[i] >= 0xA1)
        fnt_adrs_half = 0x4226 + (Sjis[i] - 0xA1) * 129;

      ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf_FHN(
          f2, fnt_adrs_half, font_buf[i]);
      if (Sjis[i + 1] < 0x20) Sjis[i + 1] = 0x20;  // 制御コードは全てスペース
      if ((Sjis[i + 1] >= 0x20 && Sjis[i + 1] <= 0x7E) ||
          (Sjis[i + 1] >= 0xA1 && Sjis[i + 1] <= 0xDF)) {
        if (Sjis[i + 1] <= 0x63)
          fnt_adrs_half = 0x1346 + (Sjis[i + 1] - 0x20) * 126;
        else if (Sjis[i + 1] <= 0x7E)
          fnt_adrs_half = 0x34BF + (Sjis[i + 1] - 0x64) * 127;
        else if (Sjis[i + 1] >= 0xA1)
          fnt_adrs_half = 0x4226 + (Sjis[i + 1] - 0xA1) * 129;

        ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf_FHN(
            f2, fnt_adrs_half, font_buf[i + 1]);

        i = i + 2;
      } else {
        i++;
      }
    } else {
      ShinonomeFONTread::SjisToShinonomeFNTadrs(Sjis[i], Sjis[i + 1],
                                                &fnt_adrs_Zen);
      ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_FHN(
          f1, fnt_adrs_Zen, font_buf[i], font_buf[i + 1]);
      i = i + 2;
    }
    yield();
  }
}
//*******************Shift_JISコードから東雲フォントアドレス計算********************************************
void ShinonomeFONTread::SjisToShinonomeFNTadrs(
    uint8_t jisH, uint8_t jisL,
    uint32_t*
        fnt_adrs) {  // S-JISコードから東雲フォントファイル上のバイト位置をポインタで返す。
  uint16_t jisCode;
  int32_t adj;

  if (jisH != '\0') {  //'\0'ならば読み込まない。
    jisCode = ((uint16_t)jisH << 8) | jisL;
    if (jisCode >= 0x8140 && jisCode <= 0x83D6) {
      if (jisCode <= 0x817E)
        adj = 1127;  // スペース～×
      else if (jisCode <= 0x81AC)
        adj = 963;  // ÷～〓
      else if (jisCode <= 0x81BF)
        adj = -841;  // ∈～∩
      else if (jisCode <= 0x81CE)
        adj = -2153;  // ∧～∃
      else if (jisCode <= 0x81E8)
        adj = -3957;  // ∠～∬
      else if (jisCode <= 0x81F7)
        adj = -5105;  // Å～¶
      else if (jisCode == 0x81FC)
        adj = -5761;  // ◯
      else if (jisCode <= 0x8258)
        adj = -19209;  // 一般記号
      else if (jisCode <= 0x8279)
        adj = -20357;  // 数字、英語大文字
      else if (jisCode <= 0x829A)
        adj = -21505;  // 英小文字、ひらがな
      else if (jisCode <= 0x82F1)
        adj = -22161;  // 英小文字、ひらがな
      else if (jisCode <= 0x837e)
        adj = -34953;  // カタカナ
      else if (jisCode <= 0x8396)
        adj = -35117;  // カタカナ
      else if (jisCode <= 0x83B6)
        adj = -36429;  // Α～Ω
      else if (jisCode >= 0x83BF)
        adj = -37741;  // α～ω
      *fnt_adrs = (jisCode - 0x8140) * 164 + adj;
    } else if (jisCode >= 0x8440 && jisCode <= 0x84BE) {
      if (jisCode <= 0x8460)
        adj = 70992;  // А～Я
      else if (jisCode <= 0x847E)
        adj = 68517;  // а～н
      else if (jisCode <= 0x8491)
        adj = 68352;  // о～я
      else if (jisCode >= 0x849F)
        adj = 66207;  // 罫線
      *fnt_adrs = (jisCode - 0x8440) * 165 + adj;
    } else if (jisCode >= 0x8740 &&
               jisCode <= 0x879E) {  // JIS13区は無いので"？"マークにする
      *fnt_adrs = 0x987;             //"？"文字
    } else if (jisCode >= 0x889f && jisCode <= 0x88FC) {  // JIS第１水準 亜～蔭
      *fnt_adrs = (jisCode - 0x889F) * 165 + 87162;
    } else if (jisCode >= 0x8940 && jisCode <= 0x9872) {  // JIS第１水準 院～腕
      if (jisL <= 0x7E) {
        *fnt_adrs = (jisCode - 0x889F) * 165 + 87162 - (jisH - 0x88) * 11055 -
                    (jisH - 0x88 - 1) * 165;
      } else if (jisL >= 0x80) {
        *fnt_adrs = (jisCode - 0x889F) * 165 + 87162 - (jisH - 0x88) * 11055 -
                    (jisH - 0x88) * 165;
      }
    } else if (jisCode >= 0x989F && jisCode <= 0x9FFC) {  // JIS第２水準 弌～滌
      if (jisL <= 0x7E) {
        *fnt_adrs = (jisCode - 0x889F) * 165 + 80067 - (jisH - 0x88) * 11055 -
                    (jisH - 0x88 - 1) * 165;
      } else if (jisL >= 0x80) {
        *fnt_adrs = (jisCode - 0x889F) * 165 + 80067 - (jisH - 0x88) * 11055 -
                    (jisH - 0x88) * 165;
      }
    } else if (jisCode >= 0xE040 && jisCode <= 0xEAA4) {  // JIS第２水準 漾～熙
      if (jisL <= 0x7E) {
        *fnt_adrs = (jisCode - 0x889F) * 165 - 2634348 - (jisH - 0xC9) * 11055 -
                    (jisH - 0xC9) * 165;
      } else if (jisL >= 0x80) {
        *fnt_adrs = (jisCode - 0x889F) * 165 - 2634348 - (jisH - 0xC9) * 11055 -
                    (jisH - 0xC9 + 1) * 165;
      }
    } else {
      *fnt_adrs = 0x467;  // 対応文字コードがなければ 全角スペースを返す
    }
  } else {
    *fnt_adrs = 0x467;  // 対応文字コードがなければ 全角スペースを返す
  }
}
//*****************全角フォントファイル読み込み**************************************
void ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread(const char* font_file,
                                                      uint32_t addrs,
                                                      uint8_t* buf1,
                                                      uint8_t* buf2) {
  //  Dir dir =
  //  LittleFS.openDir("/");//これは読み取り速度が遅くなるかもしれないのでコメントアウトしている
  uint8_t i;
  uint8_t j = 0;
  uint8_t c1, c2, c3, c4;
  char c[85];
  File f1 = LittleFS.open(font_file, "r");  // 東雲フォントファイル読み込み
  if (f1) {
    f1.seek(addrs, SeekSet);
    f1.readBytes(c, 80);  // 4byte+"." -->5byte*16=80byte
    for (i = 0; i < 16; i++) {
      if (c[j] >= 0x61)
        c1 = (c[j] - 0x61) + 10;
      else
        c1 = c[j] - 0x30;
      if (c[j + 1] >= 0x61)
        c2 = (c[j + 1] - 0x61) + 10;
      else
        c2 = c[j + 1] - 0x30;
      if (c[j + 2] >= 0x61)
        c3 = (c[j + 2] - 0x61) + 10;
      else
        c3 = c[j + 2] - 0x30;
      if (c[j + 3] >= 0x61)
        c4 = (c[j + 3] - 0x61) + 10;
      else
        c4 = c[j + 3] - 0x30;

      buf1[i] = (c1 << 4) | c2;
      buf2[i] = (c3 << 4) | c4;
      j = j + 5;
    }
    f1.close();
  } else {
    Serial.print(font_file);
    Serial.println(
        F(" file has not been uploaded to the flash in LittleFS file system"));
    Serial.println(F(" ShinonomeFONTread 275"));
    delay(30000);
  }
}
//*****************全角フォントファイル読み込み、ファイルハンドル渡し**************************************
void ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_FHN(File ff,
                                                          uint32_t addrs,
                                                          uint8_t* buf1,
                                                          uint8_t* buf2) {
  //  Dir dir =
  //  LittleFS.openDir("/");//これは読み取り速度が遅くなるかもしれないのでコメントアウトしている
  uint8_t i;
  uint8_t j = 0;
  uint8_t c1, c2, c3, c4;
  char c[85];
  if (ff) {
    ff.seek(addrs, SeekSet);
    ff.readBytes(c, 80);  // 4byte+"." -->5byte*16=80byte
    for (i = 0; i < 16; i++) {
      if (c[j] >= 0x61)
        c1 = (c[j] - 0x61) + 10;
      else
        c1 = c[j] - 0x30;
      if (c[j + 1] >= 0x61)
        c2 = (c[j + 1] - 0x61) + 10;
      else
        c2 = c[j + 1] - 0x30;
      if (c[j + 2] >= 0x61)
        c3 = (c[j + 2] - 0x61) + 10;
      else
        c3 = c[j + 2] - 0x30;
      if (c[j + 3] >= 0x61)
        c4 = (c[j + 3] - 0x61) + 10;
      else
        c4 = c[j + 3] - 0x30;

      buf1[i] = (c1 << 4) | c2;
      buf2[i] = (c3 << 4) | c4;
      j = j + 5;
    }
  } else {
    Serial.println(
        F(" file has not been uploaded to the flash in LittleFS file system"));
    Serial.println(F(" ShinonomeFONTread 306"));
    delay(30000);
  }
}
//*****************半角フォントファイル読み込み**************************************
void ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf(
    const char* font_file, uint32_t addrs, uint8_t* buf) {
  //  Dir dir =
  //  LittleFS.openDir("/");//これは読み取り速度が遅くなるかもしれないのでコメントアウトしている
  uint8_t i;
  uint8_t j = 0;
  uint8_t c1, c2;
  char c[50];
  File f1 = LittleFS.open(font_file, "r");  // 東雲フォントファイル読み込み
  if (f1) {
    f1.seek(addrs, SeekSet);
    f1.readBytes(c, 48);  // 2byte+"." -->3byte*16=48byte
    for (i = 0; i < 16; i++) {
      if (c[j] >= 0x61)
        c1 = (c[j] - 0x61) + 10;
      else
        c1 = c[j] - 0x30;
      if (c[j + 1] >= 0x61)
        c2 = (c[j + 1] - 0x61) + 10;
      else
        c2 = c[j + 1] - 0x30;

      buf[i] = (c1 << 4) | c2;

      j = j + 3;
    }
    f1.close();
  } else {
    Serial.print(font_file);
    Serial.println(
        F(" file has not been uploaded to the flash in LittleFS file system"));
    Serial.println(F(" ShinonomeFONTread 336"));
    delay(30000);
  }
}
//*****************半角フォントファイル読み込み、ファイルハンドル渡し**************************************
void ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf_FHN(File ff,
                                                               uint32_t addrs,
                                                               uint8_t* buf) {
  //  Dir dir =
  //  LittleFS.openDir("/");//これは読み取り速度が遅くなるかもしれないのでコメントアウトしている
  uint8_t i;
  uint8_t j = 0;
  uint8_t c1, c2;
  char c[50];

  if (ff) {
    ff.seek(addrs, SeekSet);
    ff.readBytes(c, 48);  // 2byte+"." -->3byte*16=48byte
    for (i = 0; i < 16; i++) {
      if (c[j] >= 0x61)
        c1 = (c[j] - 0x61) + 10;
      else
        c1 = c[j] - 0x30;
      if (c[j + 1] >= 0x61)
        c2 = (c[j + 1] - 0x61) + 10;
      else
        c2 = c[j + 1] - 0x30;

      buf[i] = (c1 << 4) | c2;

      j = j + 3;
    }
  } else {
    Serial.println(
        F(" file has not been uploaded to the flash in LittleFS file system"));
    Serial.println(F(" ShinonomeFONTread 365"));
    delay(30000);
  }
}
//*******************Shift_JIS
//フォント変換、自動インクリメント*************************************************************
uint8_t ShinonomeFONTread::Sjis_inc_FntRead(const char* FNT_file_16x16,
                                            const char* FNT_file_8x16,
                                            uint8_t* sj, uint16_t length,
                                            uint16_t* sj_cnt,
                                            uint8_t buf[2][16]) {
  uint8_t cp;
  File f1 =
      LittleFS.open(FNT_file_16x16, "r");  // 東雲フォント全角ファイル読み込み
  File f2 = LittleFS.open(FNT_file_8x16, "r");  // 東雲フォント半角ファイル読み込み

  cp = ShinonomeFONTread::Sj_inc_read(f1, f2, sj, length, sj_cnt, buf);

  f1.close();
  f2.close();
  return cp;
}
//*******************Shift_JIS
//フォント変換、自動インクリメント*************************************************************
uint8_t ShinonomeFONTread::Sjis_inc_FntRead(uint8_t* sj, uint16_t length,
                                            uint16_t* sj_cnt,
                                            uint8_t buf[2][16]) {
  return ShinonomeFONTread::Sj_inc_read(_SinoZ, _SinoH, sj, length, sj_cnt,
                                        buf);
}
//*******************Shift_JIS
//フォント変換、自動インクリメント*************************************************************
uint8_t ShinonomeFONTread::Sj_inc_read(File f1, File f2, uint8_t* sj,
                                       uint16_t length, uint16_t* sj_cnt,
                                       uint8_t buf[2][16]) {
  uint16_t fnt_adrs_half = 0x1346;  // space
  uint32_t fnt_adrs_Zen = 0x467;    // Space
  uint8_t cp;

  if ((*(sj + *sj_cnt) >= 0x20 && *(sj + *sj_cnt) <= 0x7E) ||
      (*(sj + *sj_cnt) >= 0xA1 && *(sj + *sj_cnt) <= 0xDF)) {
    if (*(sj + *sj_cnt) <= 0x63)
      fnt_adrs_half = 0x1346 + (*(sj + *sj_cnt) - 0x20) * 126;
    else if (*(sj + *sj_cnt) <= 0x7E)
      fnt_adrs_half = 0x34BF + (*(sj + *sj_cnt) - 0x64) * 127;
    else if (*(sj + *sj_cnt) >= 0xA1)
      fnt_adrs_half = 0x4226 + (*(sj + *sj_cnt) - 0xA1) * 129;

    ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf_FHN(f2, fnt_adrs_half,
                                                              buf[0]);
    cp = 1;

    if ((*(sj + *sj_cnt + 1) >= 0x20 && *(sj + *sj_cnt + 1) <= 0x7E) ||
        (*(sj + *sj_cnt + 1) >= 0xA1 && *(sj + *sj_cnt + 1) <= 0xDF)) {
      if (*(sj + *sj_cnt + 1) <= 0x63)
        fnt_adrs_half = 0x1346 + (*(sj + *sj_cnt + 1) - 0x20) * 126;
      else if (*(sj + *sj_cnt + 1) <= 0x7E)
        fnt_adrs_half = 0x34BF + (*(sj + *sj_cnt + 1) - 0x64) * 127;
      else if (*(sj + *sj_cnt + 1) >= 0xA1)
        fnt_adrs_half = 0x4226 + (*(sj + *sj_cnt + 1) - 0xA1) * 129;

      ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_Harf_FHN(
          f2, fnt_adrs_half, buf[1]);
      cp = 2;
    } else {
      cp = 1;
    }
  } else {
    ShinonomeFONTread::SjisToShinonomeFNTadrs(
        *(sj + *sj_cnt), *(sj + *sj_cnt + 1), &fnt_adrs_Zen);
    ShinonomeFONTread::SPIFFS_Flash_ShinonomeFNTread_FHN(f1, fnt_adrs_Zen,
                                                         buf[0], buf[1]);
    cp = 2;
  }

  *sj_cnt = *sj_cnt + cp;
  if (*sj_cnt >= length) *sj_cnt = 0;

  return cp;
}
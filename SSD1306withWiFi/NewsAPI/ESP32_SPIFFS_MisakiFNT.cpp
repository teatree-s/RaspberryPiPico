/*
  ESP32_SPIFFS_MisakiFNT.cpp - for ESP-WROOM-32 ( ESP32 )
  Beta version 1.0
  This is the Arduino library for reading Misaki font. (For ESP32 & LittleFS) 
  
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

#include "Arduino.h"
#include "ESP32_SPIFFS_MisakiFNT.h"
#include "LittleFS.h"

ESP32_SPIFFS_MisakiFNT::ESP32_SPIFFS_MisakiFNT(){}

//*********美咲フォントライブラリ初期化3ファイル*************************************************************
void ESP32_SPIFFS_MisakiFNT::SPIFFS_Misaki_Init3F(const char* UTF8SJIS_file, const char* Misaki_Half_Font_file, const char* Misaki_Zen_Font_file)
{
  LittleFS.begin();
  
  Serial.println("card initialized.");
  _UtoS = LittleFS.open(UTF8SJIS_file, "r");
  if (!_UtoS) {
    Serial.print(UTF8SJIS_file);
    Serial.println(" File not found");
    return;
  }else{
    Serial.print(UTF8SJIS_file);
    Serial.println(" File read OK!");
  }
  _MisakiH = LittleFS.open(Misaki_Half_Font_file, "r");
  if (!_MisakiH) {
    Serial.print(Misaki_Half_Font_file);
    Serial.print(" File not found");
    return;
  }else{
    Serial.print(Misaki_Half_Font_file);
    Serial.println(" File read OK!");
  }
  _MisakiZ = LittleFS.open(Misaki_Zen_Font_file, "r");
  if (!_MisakiZ) {
    Serial.print(Misaki_Zen_Font_file);
    Serial.print(" File not found");
    return;
  }else{
    Serial.print(Misaki_Zen_Font_file);
    Serial.println(" File read OK!");
  }
}

//**************３ファイル　フォントファイルクローズ********************
void ESP32_SPIFFS_MisakiFNT::SPIFFS_Misaki_Close3F(){
  delay(1);
  _UtoS.close();
  delay(1);
  _MisakiH.close();
  delay(1);
  _MisakiZ.close();
  delay(1);
  Serial.println("--------------3 files closed");
}

//*******************美咲フォント一括変換*************************************************************
uint16_t ESP32_SPIFFS_MisakiFNT::Sjis_To_MisakiFNT_Read_ALL(uint8_t sj_txt[], uint16_t sj_length, uint8_t buf[][8])
{
  uint16_t fnt_adrs_half;
  uint8_t i;
  uint16_t cnt = 0;
  uint16_t buf_cnt = 0;

  for(cnt=0; cnt<sj_length; cnt++){
    uint8_t dummy_buf1[8] = {0};
    uint8_t dummy_buf2[8] = {0};
    uint8_t dummy_buf3[8] = {0};
    uint8_t dummy_buf4[8] = {0};

    if((Zenkaku_bridge == true) || (sj_txt[cnt]>=0x20 && sj_txt[cnt]<=0x7F) || (sj_txt[cnt]>=0xA1 && sj_txt[cnt]<=0xDF)){
      if(Zenkaku_bridge == true){
        for(i=0; i<8; i++){
          dummy_buf1[i] = bridge_buf1[i] << 4;
        }
      }else{
        fnt_adrs_half = 0x110 + (sj_txt[cnt] - 0x20)*8;
        ESP32_SPIFFS_MisakiFNT::SPIFFS_MisakiFontRead(_MisakiH, fnt_adrs_half, dummy_buf1);
      }
      Zenkaku_bridge = false;

      if(cnt + 1 < sj_length){
        cnt++;
      }else{
        for(i=0; i<8; i++){
          buf[buf_cnt][i] = dummy_buf1[i];
        }
        return buf_cnt + 1;
        break;
      }

      if((sj_txt[cnt]>=0x20 && sj_txt[cnt]<=0x7F) || (sj_txt[cnt]>=0xA1 && sj_txt[cnt]<=0xDF)){
        fnt_adrs_half = 0x110 + (sj_txt[cnt] - 0x20)*8;

        ESP32_SPIFFS_MisakiFNT::SPIFFS_MisakiFontRead(_MisakiH, fnt_adrs_half, dummy_buf2);

        for(i=0; i<8; i++){
          dummy_buf2[i] = dummy_buf2[i]>>4;
          buf[buf_cnt][i] = dummy_buf1[i] | dummy_buf2[i];
        }			

        Zenkaku_bridge = false;

      }else if((sj_txt[cnt]>=0x81 && sj_txt[cnt]<=0x9F) || (sj_txt[cnt]>=0xE0 && sj_txt[cnt]<=0xEA)){
        ESP32_SPIFFS_MisakiFNT::Sjis_To_Misaki_Font_Adrs(_MisakiZ, sj_txt[cnt], sj_txt[cnt+1], dummy_buf3);
        for( uint8_t col = 0; col < 8; col++ ) {
          for( uint8_t row = 0; row < 8; row++ ) {
            bitWrite( dummy_buf4[7-row], 7-col , bitRead( dummy_buf3[col], 7-row ) );
          }
        }
        for(i=0; i<8; i++){
          bridge_buf1[i] = dummy_buf4[i];
          buf[buf_cnt][i] = dummy_buf1[i] | dummy_buf4[i]>>4;
        }

        Zenkaku_bridge = true;
      }

      buf_cnt++;

    }else if((sj_txt[cnt]>=0x81 && sj_txt[cnt]<=0x9F) || (sj_txt[cnt]>=0xE0 && sj_txt[cnt]<=0xEA)){
      ESP32_SPIFFS_MisakiFNT::Sjis_To_Misaki_Font_Adrs(_MisakiZ, sj_txt[cnt], sj_txt[cnt+1], dummy_buf3);
      for( uint8_t col = 0; col < 8; col++ ) {
        for( uint8_t row = 0; row < 8; row++ ) {
          bitWrite( buf[buf_cnt][7-row], 7-col , bitRead( dummy_buf3[col], 7-row ) );
        }
      }
      cnt++;
      buf_cnt++;
      Zenkaku_bridge = false;
    }
  }
  return buf_cnt;
}
//*******************美咲フォント全変換*************************
uint16_t ESP32_SPIFFS_MisakiFNT::StrDirect_MisakiFNT_readALL(String str, uint8_t font_buf[][8])
{
  uint8_t sj_txt[str.length()];
  uint16_t sj_length;
  
  _u8ts.UTF8_to_SJIS_str_cnv(_UtoS, str, sj_txt, &sj_length);
  ESP32_SPIFFS_MisakiFNT::Sjis_To_MisakiFNT_Read_ALL(sj_txt, sj_length, font_buf);
  return sj_length;
}

//*******************Shift_JISコードから美咲フォントアドレス計算********************************************
void ESP32_SPIFFS_MisakiFNT::Sjis_To_Misaki_Font_Adrs(File f, uint8_t jisH, uint8_t jisL, uint8_t* buf) 
{    // S-JISコードからMisakiフォントファイル上のバイト位置をポインタで返す。
  uint16_t SjisCode = 0;
  int16_t adj = 0;
  uint32_t fnt_adrs = (0x8140 + 16)*8;

  if( jisH != '\0'){  //'\0'ならば読み込まない。
    if((jisH >= 0x81 && jisH <=0x9f) || (jisH >= 0xe0 && jisH <=0xef)){  //全角の場合
      SjisCode = ((uint16_t)jisH << 8 )+jisL;
      if(SjisCode>=0x8140 && SjisCode <=0x88fc){
        if     (SjisCode>=0x8140 && SjisCode<=0x817e) adj =  16;  // 一般記号
        else if(SjisCode>=0x8180 && SjisCode<=0x81fc) adj =  15;  // 一般記号
        else if(SjisCode>=0x824f && SjisCode<=0x8279) adj = -52;  // 数字、英語大文字
        else if(SjisCode>=0x8281 && SjisCode<=0x82f1) adj = -53;  // 英小文字、ひらがな
        else if(SjisCode>=0x8340 && SjisCode<=0x837e) adj = -120;  // カタカナ
        else if(SjisCode>=0x8380 && SjisCode<=0x83d6) adj = -121;  // カタカナ
        else if(SjisCode>=0x8440 && SjisCode<=0x847e) adj = -188;  // 外国大文字
        else if(SjisCode>=0x8480 && SjisCode<=0x84be) adj = -189;  // 外国小文字、罫線、※丸囲み文字やその他特殊文字は美咲フォントには無い。
        else if(SjisCode>=0x8740 && SjisCode<=0x877e) adj = -768; // １３区
        else if(SjisCode>=0x8780 && SjisCode<=0x879c) adj = -769; // １３区
        else if(SjisCode>=0x889f && SjisCode<=0x88fc) adj = -837;  // 第一水準漢字 亜～蔭まで
        fnt_adrs = ((SjisCode-0x8140)+adj)*8;
      }else if(SjisCode>=0x8940 && SjisCode<=0x9ffc){ //院～滌
        if(jisL <= 0x7e){
          fnt_adrs = ((SjisCode-0x8140)-(836+(jisH-0x88))-67*(jisH-0x88))*8;
        }else if(jisL >= 0x80 ){
          fnt_adrs = ((SjisCode-0x8140)-(837+(jisH-0x88))-67*(jisH-0x88))*8;
        }
      }else if(SjisCode>=0xe040 && SjisCode <=0xeaa4){ //漾～熙
        if(jisL <= 0x7e ){
          fnt_adrs = ((SjisCode-0x8140)-(836+(jisH-0x88))-67*(jisH-0x88)-12032)*8;
        }else if(jisL >= 0x80 ){
          fnt_adrs = ((SjisCode-0x8140)-(837+(jisH-0x88))-67*(jisH-0x88)-12032)*8;
        }
      }else{
        fnt_adrs = (0x8140 + 16)*8;  // 対応文字コードがなければ 全角スペースを返す
      }
    }else {
      fnt_adrs = (0x8140 + 16)*8;  // 対応文字コードがなければ 全角スペースを返す
    }
  }else {
    fnt_adrs = (0x8140 + 16)*8;  // 対応文字コードがなければ 全角スペースを返す
  }
  
  ESP32_SPIFFS_MisakiFNT::SPIFFS_MisakiFontRead(f, fnt_adrs, buf);
}
//*****************フォントファイル読み込み**************************************
void ESP32_SPIFFS_MisakiFNT::SPIFFS_MisakiFontRead(File f1, uint32_t addrs, uint8_t* buf)
{
  if(f1){
    f1.seek(addrs);
    f1.read(buf, 8);
  }else{
    Serial.println(F(" Misaki font file has not been uploaded to the LittleFS"));
    delay(30000);
  }
}
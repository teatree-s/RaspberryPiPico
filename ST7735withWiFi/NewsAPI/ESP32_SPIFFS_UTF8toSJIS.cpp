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

#include "ESP32_SPIFFS_UTF8toSJIS.h"

ESP32_SPIFFS_UTF8toSJIS::ESP32_SPIFFS_UTF8toSJIS(){}

//*********UTF8 to Shift_JIS 初期化*************************************************************
void ESP32_SPIFFS_UTF8toSJIS::ESP32_SPIFFS_UTF8toSJIS_Init(const char* UTF8SJIS_file)
{
  LittleFS.begin();
  //LittleFS.begin(true);

  Serial.println(F("LittleFS initialized."));
  _UtoS = LittleFS.open(UTF8SJIS_file, "r");
  if (!_UtoS) {
    Serial.print(UTF8SJIS_file);
    Serial.println(F(" File not found"));
    return;
  }else{
    Serial.print(UTF8SJIS_file);
    Serial.println(F(" File read OK!"));
  }
}
//**************フォントファイルクローズ********************
void ESP32_SPIFFS_UTF8toSJIS::ESP32_SPIFFS_UTF8toSJIS_Close(){
  delay(1);
  _UtoS.close();
  delay(1);
  Serial.println(F("--------------3 files closed"));
  LittleFS.end();
  Serial.println(F("--------------LittleFS.end"));
}
//***********String型文字列をShift_JISコードに変換 ファイルハンドル込み************************************
uint16_t ESP32_SPIFFS_UTF8toSJIS::UTF8_to_SJIS(String strUTF8, uint8_t* sjis_byte)
{
  uint16_t sj_length = 0;
  ESP32_SPIFFS_UTF8toSJIS::UTF8_to_SJIS_str_cnv(_UtoS, strUTF8, sjis_byte, &sj_length);

  return sj_length;
}
//***********String型文字列をShift_JISコードに変換 ファイルハンドル外部代入************************************
void ESP32_SPIFFS_UTF8toSJIS::UTF8_to_SJIS_str_cnv(File f2, String strUTF8, uint8_t* sjis_byte, uint16_t* sj_length)
{
  uint16_t sj_cnt = 0;
  uint16_t fnt_cnt = 0;
  uint32_t sp_addres = 0x9DCC;//スペース
  uint8_t SJ[2];

  while(strUTF8[fnt_cnt] != '\0'){
    if(strUTF8[fnt_cnt]>=0xC2 && strUTF8[fnt_cnt]<=0xD1){//2バイト文字
      ESP32_SPIFFS_UTF8toSJIS::UTF8_To_SJIS_code_cnv(strUTF8[fnt_cnt],strUTF8[fnt_cnt+1],0x00, &sp_addres);
      ESP32_SPIFFS_UTF8toSJIS::SPIFFS_Flash_UTF8SJIS_Table_Read(f2, sp_addres, SJ);
      sjis_byte[sj_cnt] = SJ[0];
      sjis_byte[sj_cnt+1] = SJ[1];
      sj_cnt = sj_cnt + 2;
      fnt_cnt = fnt_cnt + 2;
    }else if(strUTF8[fnt_cnt]>=0xE2 && strUTF8[fnt_cnt]<=0xEF){
      ESP32_SPIFFS_UTF8toSJIS::UTF8_To_SJIS_code_cnv(strUTF8[fnt_cnt],strUTF8[fnt_cnt+1],strUTF8[fnt_cnt+2], &sp_addres);
      ESP32_SPIFFS_UTF8toSJIS::SPIFFS_Flash_UTF8SJIS_Table_Read(f2, sp_addres, SJ);
      if((SJ[0]>=0xA1 && SJ[0]<=0xDF)){ //Shift_JISで半角カナコードが返ってきた場合の対処
        sjis_byte[sj_cnt] = SJ[0];
        sj_cnt++;
      }else{
        sjis_byte[sj_cnt] = SJ[0];
        sjis_byte[sj_cnt+1] = SJ[1];
        sj_cnt = sj_cnt + 2;
      }
      fnt_cnt = fnt_cnt +3;
    }else if(strUTF8[fnt_cnt]>=0x20 && strUTF8[fnt_cnt]<=0x7E){
      SJ[0] = strUTF8[fnt_cnt];
      sjis_byte[sj_cnt] = strUTF8[fnt_cnt];
      sj_cnt++;
      fnt_cnt++;
    }else{ //その他は全て半角スペースとする。
      sjis_byte[sj_cnt] = 0x20;
      sj_cnt++;
      fnt_cnt++;
    }
    yield();
  }
  *sj_length = sj_cnt;
}

//***********UTF-8コードをLittleFS内の変換テーブルを読み出してShift-JISコードに変換****
void ESP32_SPIFFS_UTF8toSJIS::UTF8_To_SJIS_code_cnv(uint8_t utf8_1, uint8_t utf8_2, uint8_t utf8_3, uint32_t* SPIFFS_addrs)
{
  if(utf8_1>=0xC2 && utf8_1<=0xD1){
    //0xB0からS_JISコード実データ。0x00-0xAFまではライセンス文ヘッダ。
    *SPIFFS_addrs = ((utf8_1<<8 | utf8_2)-0xC2A2)*2 + 0xB0; //文字"¢" UTF8コード C2A2～、S_jisコード8191
  }else if(utf8_2>=0x80){
    uint32_t UTF8uint = (utf8_1<<16) | (utf8_2<<8) | utf8_3;

    switch(utf8_1){
      case 0xE2:
        //*SPIFFS_addrs = (UTF8uint-0xE28090)*2 + 0x1EEC; //文字"‐" UTF8コード E28090～、S_jisコード815D
        *SPIFFS_addrs = UTF8uint*2 - 0x01C4E234; //文字"‐" UTF8コード E28090～、S_jisコード815D
        break;
      case 0xE3:
        //*SPIFFS_addrs = (UTF8uint-0xE38080)*2 + 0x9DCC; //スペース UTF8コード E38080～、S_jisコード8140
        *SPIFFS_addrs = UTF8uint*2 - 0x01C66334; //スペース UTF8コード E38080～、S_jisコード8140
        break;
      case 0xE4:
        //*SPIFFS_addrs = (UTF8uint-0xE4B880)*2 + 0x11CCC; //文字"一" UTF8コード E4B880～、S_jisコード88EA
        *SPIFFS_addrs = UTF8uint*2 - 0x01C85434; //文字"一" UTF8コード E4B880～、S_jisコード88EA
        break;
      case 0xE5:
        //*SPIFFS_addrs = (UTF8uint-0xE58085)*2 + 0x12BCC; //文字"倅" UTF8コード E58085～、S_jisコード98E4
        *SPIFFS_addrs = UTF8uint*2 - 0x01C9D53E; //文字"倅" UTF8コード E58085～、S_jisコード98E4
        break;
      case 0xE6:
        //*SPIFFS_addrs = (UTF8uint-0xE6808E)*2 + 0x1AAC2; //文字"怎" UTF8コード E6808E～、S_jisコード9C83
        *SPIFFS_addrs = UTF8uint*2 - 0x01CB565A; //文字"怎" UTF8コード E6808E～、S_jisコード9C83
        break;
      case 0xE7:
        //*SPIFFS_addrs = (UTF8uint-0xE78081)*2 + 0x229A6; //文字"瀁" UTF8コード E78081～、S_jisコードE066
        *SPIFFS_addrs = UTF8uint*2 - 0x01CCD75C; //文字"瀁" UTF8コード E78081～、S_jisコードE066
        break;
      case 0xE8:
        //*SPIFFS_addrs = (UTF8uint-0xE88080)*2 + 0x2A8A4; //文字"耀" UTF8コード E88080～、S_jisコード9773
        *SPIFFS_addrs = UTF8uint*2 - 0x01CE585C; //文字"耀" UTF8コード E88080～、S_jisコード9773
        break;
      case 0xE9:
        //*SPIFFS_addrs = (UTF8uint-0xE98080)*2 + 0x327A4; //文字"退" UTF8コード E98080～、S_jisコード91DE
        *SPIFFS_addrs = UTF8uint*2 - 0x01CFD95C; //文字"退" UTF8コード E98080～、S_jisコード91DE
        break;
      default:
        if(utf8_1>=0xEF && utf8_2>=0xBC){
          //*SPIFFS_addrs = (UTF8uint-0xEFBC81)*2 + 0x3A6A4; //文字"！" UTF8コード EFBC81～、S_jisコード8149
          *SPIFFS_addrs = UTF8uint*2 - 0x01DBD25E; //文字"！" UTF8コード EFBC81～、S_jisコード8149
          if(utf8_1==0xEF && utf8_2==0xBD && utf8_3==0x9E){
            *SPIFFS_addrs = 0x3A8DE; // "～" UTF8コード EFBD9E、S_jisコード8160
          }
        }
        break;
    }
  } 
}
//***************************************
void ESP32_SPIFFS_UTF8toSJIS::SPIFFS_Flash_UTF8SJIS_Table_Read(File ff, uint32_t addrs, uint8_t* buf)
{
  if(ff){
    ff.seek(addrs);
    ff.read(buf, 2);
  }else{
    Serial.println(F(" UTF8toSjis file has not been uploaded to the flash in LittleFS file system"));
    delay(30000);
  }
}

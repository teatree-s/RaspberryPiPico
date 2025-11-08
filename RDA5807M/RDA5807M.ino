///
/// \file  TestRDA5807M.ino
/// \brief An Arduino sketch to operate a SI4705 chip based radio using the Radio library.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014 by Matthias Hertel.\n
/// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
///
/// \details
/// This sketch implements a "as simple as possible" radio without any possibility to modify the settings after initializing the chip.\n
/// The radio chip is initialized and setup to a fixed band and frequency. These settings can be changed by modifying the
/// FIX_BAND and FIX_STATION definitions.
///
/// Open the Serial console with 57600 baud to see the current radio information.
///
/// Wiring
/// ------
/// The RDA5807M board/chip has to be connected by using the following connections:
///
/// | Signal       | Arduino UNO | ESP8266 | ESP32  | Radio chip signal |
/// | ------------ | ------------| ------- | ------ | ----------------- |
/// | VCC (red)    | 3.3V        | 3v3     | 3v3    | VCC               |
/// | GND (black)  | GND         | GND     | GND    | GND               |
/// | SCL (yellow) | A5 / SCL    | D1      | 22     | SCLK              |
/// | SDA (blue)   | A4 / SDA    | D2      | 21     | SDIO              |
///
/// The locations of the signals on the RDA5807M side depend on the board you use.
/// More documentation is available at http://www.mathertel.de/Arduino
/// Source Code is available on https://github.com/mathertel/Radio
///
/// ChangeLog:
/// ----------
/// * 05.12.2014 created.
/// * 19.05.2015 extended.

#include <Arduino.h>
#include <Wire.h>

#include <radio.h>
#include <RDA5807M.h>

// ----- Fixed settings here. -----

#define FIX_BAND RADIO_BAND_FMWORLD  ///< The band that will be tuned by this sketch is FM.
#define FIX_STATION 8020        ///< The station that will be tuned by this sketch is 89.30 MHz.
#define FIX_VOLUME 4           ///< The volume that will be set by this sketch is level 4.

#define BUTTON_UP   10  // GP10
#define BUTTON_DOWN 20  // GP20
#define BUTTON_MODE 21  // GP21

#if defined(LED_BUILTIN)
  #define LED_PIN LED_BUILTIN
#elif defined(PICO_DEFAULT_LED_PIN)
  #define LED_PIN PICO_DEFAULT_LED_PIN
#else
  #define LED_PIN 25  // fallback（通常PicoはGP25）
#endif

RDA5807M radio;  // Create an instance of Class for RDA5807M Chip
int currentVolume = FIX_VOLUME;
int currentMode = 1;

/// Setup a FM only radio configuration
/// with some debugging on the Serial port
void setup() {
  delay(3000);
  // open the Serial port
  Serial.begin(115200);
  Serial.println("RDA5807M Radio...");
  delay(200);

  // Setup
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Enable information to the Serial port
  radio.debugEnable(true);
  radio._wireDebug(false);

  // Set FM Options for Europe
  radio.setup(RADIO_FMSPACING, RADIO_FMSPACING_100);   // for EUROPE
  radio.setup(RADIO_DEEMPHASIS, RADIO_DEEMPHASIS_50);  // for EUROPE

  // Initialize the Radio
  if (!radio.initWire(Wire)) {
    Serial.println("no radio chip found.");
    delay(4000);
    return;
  };

  // Set all radio setting to the fixed values.
  radio.setBandFrequency(FIX_BAND, FIX_STATION);
  radio.setVolume(FIX_VOLUME);
  radio.setMono(false);
  radio.setMute(false);
}  // setup

void controlMode() {
  if (digitalRead(BUTTON_MODE) == LOW) {
    currentMode = (currentMode) ? 0 : 1;
    Serial.print("Mode: ");
    Serial.println(currentMode);
    digitalWrite(LED_PIN, true);
    delay(300);
    digitalWrite(LED_PIN, false);
  }
}

void controlVolume() {
  // Volume UP
  if (digitalRead(BUTTON_UP) == LOW) {
    if (currentVolume < 15) { // MAX 15
      currentVolume++;
      radio.setVolume(currentVolume);
      Serial.print("Volume UP: ");
      Serial.println(currentVolume);
      digitalWrite(LED_PIN, true);
      delay(300);
      digitalWrite(LED_PIN, false);
    }
  }

  // Volume DOWN
  if (digitalRead(BUTTON_DOWN) == LOW) {
    if (currentVolume > 0) { // MIN 0
      currentVolume--;
      radio.setVolume(currentVolume);
      Serial.print("Volume DOWN: ");
      Serial.println(currentVolume);
      digitalWrite(LED_PIN, true);
      delay(300);
      digitalWrite(LED_PIN, false);
    }
  }
}

void controlBandFrequency() {
  if (digitalRead(BUTTON_UP) == LOW) {
    radio.setBandFrequency(FIX_BAND, FIX_STATION);
    Serial.print("BandFrequency: ");
    Serial.println(FIX_STATION);
    digitalWrite(LED_PIN, true);
    delay(300);
    digitalWrite(LED_PIN, false);
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    radio.setBandFrequency(FIX_BAND, 8510);
    Serial.print("BandFrequency: ");
    Serial.println(8510);
    digitalWrite(LED_PIN, true);
    delay(300);
    digitalWrite(LED_PIN, false);
  }
}

/// show the current chip data every 3 seconds.
void loop() {

  controlMode();
  if (currentMode) {
    controlVolume();
  } else {
    controlBandFrequency();
  }

  static unsigned long lastInfo = 0;
  if (millis() - lastInfo > 3000) {
    lastInfo = millis();
    char s[12];
    radio.formatFrequency(s, sizeof(s));
    Serial.print("Station: ");
    Serial.println(s);

    Serial.print("Radio: ");
    radio.debugRadioInfo();

    Serial.print("Audio: ");
    radio.debugAudioInfo();
  }
}  // loop

// End.

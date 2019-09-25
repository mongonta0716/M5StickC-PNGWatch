/*
 Analog clock for M5StickC.

 'BackgroundLayer.cpp' and 'BackgroundLayer.h' are required.

 If you want to use your own image, prepare a clockC.png file(size 160x80).
 After that, convert to clockC.png.h using the following tool.

 https://github.com/mongonta0716/M5Stack_Test_tools

 
 Based on a sketch by M5StickC's example TFT_Clock 25/9/2019
 Based on a sketch by Gilchrist 6/2/2014 1.0
 */

#include <M5StickC.h>
#include <time.h>
#include <WiFi.h>
#include "BackgroundLayer.h"
#include "clockC.png.h"

TFT_bgLayer bg = TFT_bgLayer(&M5.Lcd);

float x[3][3], y[3][3]; // For coordinate calculation
float deg[3];           // Degree of clock hands

uint32_t targetTime = 0;                    // for next 1 second timeout

uint8_t hms[3] = { 0, 0, 0 };  // Get H, M, S from RTC


// ----------------------------------------------------------------------------------
// initial Settings
// Array index : 0:hh 1:mm 2:ss
// ----------------------------------------------------------------------------------
const char ssid[] = "XXXXXXXXXXXXXXXXXXXX";         // set your SSID
const char pass[] = "XXXXXXXXXXXXXXXXXXXX";         // set your passphrase
const char ntpServer[] = "XXXXXXXXXXXXXXXXXXXX";    // set your NTP Server

const uint8_t  brightness = 8;                     // LCD brightness(7 - 15)
const uint16_t clockx = 40;                       // clock center X
const uint16_t clocky = 40;                        // clock center Y
const uint16_t len[3] = { 25, 30, 35 };            // Size of clock hands
const uint16_t colorhh = M5.Lcd.color565(0, 0, 0); // Color of clock hand(hour)
const uint16_t colormm = M5.Lcd.color565(0, 0, 0); // Color of clock hand(minute)
const uint16_t colorss = M5.Lcd.color565(255, 0, 0);    // Color of clock hand(second)

// ----------------------------------------------------------------------------------
// End of initial Settings
// ----------------------------------------------------------------------------------

uint16_t handscolor[3] = {colorhh, colormm, colorss};
uint16_t ox[3] = { 0, 0, 0 } ,oy[3] = { 0, 0, 0 };           // Outer coordinates of clock hands
uint16_t cx[3][3]= { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0} }; // Center coordinates of clock hands.
uint16_t cy[3][3]= { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0} }; // Center coordinates of clock hands.

bool initial = true;

const char filler[] = "    ";

void startRTC() {
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);
  
  hms[0] = (uint8_t)RTC_TimeStruct.Hours;
  hms[1] = (uint8_t)RTC_TimeStruct.Minutes;
  hms[2] = (uint8_t)RTC_TimeStruct.Seconds;
}

void adjustRTCfromNTP() {
  WiFi.begin(ssid, pass);
  M5.Lcd.setCursor(0, 0);
  while (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(500);
  }
  M5.Lcd.println("WiFi connected");
  
  configTime(9 * 3600, 0, ntpServer);
  struct tm timeinfo;

  M5.Lcd.println("Get Time");
  while (!getLocalTime(&timeinfo)) {
    M5.Lcd.print(".");
    delay(1000);
  }
  RTC_TimeTypeDef timeStruct;
  timeStruct.Hours   = timeinfo.tm_hour;
  timeStruct.Minutes = timeinfo.tm_min;
  timeStruct.Seconds = timeinfo.tm_sec;
  M5.Rtc.SetTime(&timeStruct);

  RTC_DateTypeDef dateStruct;
  dateStruct.WeekDay = timeinfo.tm_wday;
  dateStruct.Month   = timeinfo.tm_mon + 1;
  dateStruct.Date    = timeinfo.tm_mday;
  dateStruct.Year    = timeinfo.tm_year + 1900;
  M5.Rtc.SetData(&dateStruct);

  M5.Lcd.println("RTC adjusted     ");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(5000);

  
  startRTC();
  bg.pushLayer(0, 0);
  
}

void setup(void) {
  M5.begin();
  M5.Axp.ScreenBreath(brightness);
  M5.Lcd.setRotation(3);
  startRTC();
  
  targetTime = millis() + 1000; 
  bg.setColorDepth(16);
  
  bg.createLayer(160, 80);
  bg.pushImage(0, 0, 160, 80, clockC );



}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    adjustRTCfromNTP();
  }

  if (targetTime < millis()) {
    targetTime += 1000;
    hms[2]++;              // Advance second
    if (hms[2] == 60) {
      hms[2] = 0;
      hms[1]++;            // Advance minute
      if(hms[1] > 59) {
        hms[1] = 0;
        hms[0]++;          // Advance hour
        if (hms[0] > 23) {
          hms[0] = 0;
        }
      }
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    deg[2] = hms[2] * 6;                  // 0-59 -> 0-354
    deg[1] = hms[1] * 6  + deg[2] * 0.01666667;  // 0-59 -> 0-360 - includes seconds
    deg[0] = hms[0] * 30 + deg[1] * 0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds

    float factor = 0.0174523925; // = pai / 180
    for (int i = 0; i < 3 ; i++) { // hms loop
      for (int j = 0; j < 3; j++) { // width loop
        x[i][j] = cos((deg[i] - (90 * j)) * factor);
        y[i][j] = sin((deg[i] - (90 * j)) * factor);
      }
    }
    
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        // Redraw old hand positions
        bg.drawLine(ox[i], oy[i], cx[i][j], cy[i][j]);
        cx[i][j] = clockx + x[i][j];
        cy[i][j] = clocky + y[i][j];
      }
      cx[i][1] = clockx;
      cy[i][1] = clocky;
      ox[i] = x[i][1] * len[i] + clockx;
      oy[i] = y[i][1] * len[i] + clocky;
    }

    // Draw new clock hands.
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        M5.Lcd.drawLine(ox[i], oy[i], cx[i][j], cy[i][j], handscolor[i]);
      }
    }

    M5.Lcd.fillCircle(clockx, clocky, 3, handscolor[2]);

  }
 
}

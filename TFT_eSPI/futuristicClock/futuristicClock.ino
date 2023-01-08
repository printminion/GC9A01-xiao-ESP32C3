#include "face_01_blank.h"

#include "TFT_eSPI.h"
#include <WiFi.h>
#include "time.h"

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);
TFT_eSprite sprTime = TFT_eSprite(&lcd); // sprite for time
TFT_eSprite sprSeconds = TFT_eSprite(&lcd); // sprite for seconds
TFT_eSprite sprConnection = TFT_eSprite(&lcd); // sprite for connection status

const char *ssid = "PUT_YOUR_WIFI_SSID_HERE";
const char *password = "PUT_YOUR_WIFI_PASS_HERE";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;    // time zone * 3600 , my time zone is  +1 GTM
const int daylightOffset_sec = 3600;


#define paleOrange 0x11FF

struct tm timeinfo;
char timeHour[3];
char timeMin[3];
char timeSec[3];
char day[3];
char month[12];
char year[5];
char timeWeekDay[3];
String dayInWeek;
String IP;


void setup(void) {

    lcd.init();
    lcd.setRotation(0);

    sprite.createSprite(240, 240);
    sprite.setSwapBytes(true);
    sprite.setTextColor(TFT_WHITE, 0xEAA9);
    sprite.setTextDatum(4);

    sprTime.createSprite(90, 40);
    sprTime.setTextColor(paleOrange, TFT_GREEN);
    sprTime.setFreeFont(&Orbitron_Light_24);

    sprSeconds.createSprite(40, 40);
    sprSeconds.setTextColor(TFT_WHITE, TFT_GREEN);

    sprConnection.createSprite(90, 30);
    sprConnection.fillSprite(TFT_GREEN);
    sprConnection.setTextDatum(4);
    sprConnection.setTextColor(TFT_WHITE, TFT_GREEN);
    sprConnection.setFreeFont(&Orbitron_Light_24);

    Serial.begin(9600);
    WiFi.begin(ssid, password);
    bool connectionBlink = false;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        sprite.fillSprite(TFT_BLACK);
        sprite.pushImage(0, 0, 240, 240, face_01_blank);

        // blink while connecting
        connectionBlink = !connectionBlink;

        sprConnection.drawString("Connecting...", 42, 6, 2);
        sprConnection.pushToSprite(&sprite, 80, 205, TFT_GREEN);

        if (connectionBlink) {
            sprite.fillCircle(119, 226, 5, 0x7D30); // green lamp
        }
        sprite.pushSprite(0, 0);
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    IP = WiFi.localIP().toString();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

int x = 0;

void printLocalTime() {

    if (!getLocalTime(&timeinfo)) {
        return;
    }

    strftime(timeHour, 3, "%H", &timeinfo);
    strftime(timeMin, 3, "%M", &timeinfo);
    strftime(timeSec, 3, "%S", &timeinfo);

    strftime(timeWeekDay, 10, "%A", &timeinfo);
    dayInWeek = String(timeWeekDay);

    strftime(day, 3, "%d", &timeinfo);
    strftime(month, 8, "%B", &timeinfo);
    strftime(year, 5, "%Y", &timeinfo);
}

long t = 0;

void loop() {
    sprite.fillSprite(TFT_BLACK);
    sprite.pushImage(0, 0, 240, 240, face_01_blank);
    sprite.setTextColor(0xFF11);
    sprite.drawString(String(day) + " " + String(month), 120, 188, 2);

    sprTime.fillSprite(TFT_GREEN);
    // center time label
    if (timeinfo.tm_hour < 20) {
        sprTime.drawString(String(timeHour) + ":" + String(timeMin), 8, 6);
    } else {
        sprTime.drawString(String(timeHour) + ":" + String(timeMin), 4, 6);
    }
    sprTime.pushToSprite(&sprite, 120 - 43, 102, TFT_GREEN);

    sprSeconds.fillSprite(TFT_GREEN);
    sprSeconds.drawString(String(timeSec), 0, 0);
    sprSeconds.pushToSprite(&sprite, 120 - 5, 120 + 28, TFT_GREEN);

    sprConnection.fillSprite(TFT_GREEN);
    sprConnection.drawString("CONNECTED", 44, 6, 2);
    sprConnection.setTextFont(0);
    sprConnection.drawString(IP, 46, 23);
    sprConnection.pushToSprite(&sprite, 80, 205, TFT_GREEN);

    sprite.pushSprite(0, 0);
    //delay(20);

    if (t + 1000 < millis()) {
        printLocalTime();
        t = millis();
    }
}

  

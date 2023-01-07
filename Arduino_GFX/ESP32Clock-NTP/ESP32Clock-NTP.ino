/*
   Arduino Watch Lite Version
   You may find full version at: https://github.com/moononournation/ArduinoWatch
*/
/**
 * Modified demo for
 * <a href="https://www.waveshare.com/1.28inch-lcd-module.htm">Waveshare 240x240, 1.28inch LCD Display</a> Module Embedded GC9A01 Driver, Using SPI Bus
 *
 * <a href="https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html">Studio XIAO ESP32C3</a> tiny MCU board with Wi-Fi and Bluetooth5.0, battery charge supported, power efficiency and rich Interface
 *
 * <a href="https://github.com/printminion/GC9A01-xiao-ESP32C3">Repo</a>
 * 3d printed cases https://cults3d.com/en/design-collections/printminion/various-cases-for-waveshare-1-28inch-lcd-display-and-studio-xiao-esp32c3
 */
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"

const char *ssid = "PUT_YOUR_WIFI_SSID_HERE";
const char *password = "PUT_YOUR_WIFI_PASS_HERE";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


#define TFT_MISO MISO
#define TFT_SCLK SCK //D8
#define TFT_MOSI MOSI // DIN D10
#define TFT_CS   SS  // ??? Chip select control pin D7
#define TFT_DC   D4  // ??? Data Command control pin
#define TFT_RST  D5   // TF-CS ??? Reset pin (could connect to Arduino RESET pin)

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
// #if defined(DISPLAY_DEV_KIT)
// Arduino_GFX *gfx = create_default_Arduino_GFX();
// #else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);


/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST /* RST */, 0 /* rotation */, true /* IPS */);

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

#define BACKGROUND BLACK
#define MARK_COLOR WHITE
#define SUBMARK_COLOR DARKGREY // LIGHTGREY
#define HOUR_COLOR WHITE
#define MINUTE_COLOR BLUE // LIGHTGREY
#define SECOND_COLOR RED

#define SIXTIETH 0.016666667
#define TWELFTH 0.08333333
#define SIXTIETH_RADIAN 0.10471976
#define TWELFTH_RADIAN 0.52359878
#define RIGHT_ANGLE_RADIAN 1.5707963

static uint8_t conv2d(const char *p) {
    uint8_t v = 0;
    return (10 * (*p - '0')) + (*++p - '0');
}

static int16_t w, h, center;
static int16_t hHandLen, mHandLen, sHandLen, markLen;
static float sdeg, mdeg, hdeg;
static int16_t osx = 0, osy = 0, omx = 0, omy = 0, ohx = 0, ohy = 0; // Saved H, M, S x & y coords
static int16_t nsx, nsy, nmx, nmy, nhx, nhy;                         // H, M, S x & y coords
static int16_t xMin, yMin, xMax, yMax;                               // redraw range
static int16_t hh, mm, ss;
static unsigned long targetTime; // next action time

static int16_t *cached_points;
static uint16_t cached_points_idx = 0;
static int16_t *last_cached_point;

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("No time available (yet)");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
    Serial.println("Got time adjustment from NTP!");
    printLocalTime();
}

void setup(void) {
    Serial.begin(115200);
    gfx->begin();
    gfx->fillScreen(BACKGROUND);

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    // init LCD constant
    w = gfx->width();
    h = gfx->height();
    if (w < h) {
        center = w / 2;
    } else {
        center = h / 2;
    }


    gfx->setCursor(0, h / 2);
    gfx->setTextColor(0xffff);
    gfx->setTextSize(1 /* x scale */, 1 /* y scale */, 1 /* pixel_margin */);


    // set notification call-back function
    sntp_set_time_sync_notification_cb(timeavailable);

    /**
     * NTP server address could be aquired via DHCP,
     *
     * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
     * otherwise SNTP option 42 would be rejected by default.
     * NOTE: configTime() function call if made AFTER DHCP-client run
     * will OVERRIDE aquired NTP server address
     */
    sntp_servermode_dhcp(1);    // (optional)

    /**
     * This will set configured ntp servers and constant TimeZone/daylightOffset
     * should be OK if your time zone does not need to adjust daylightOffset twice a year,
     * in such a case time adjustment won't be handled automagicaly.
     */
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

    /**
     * A more convenient approach to handle TimeZones with daylightOffset
     * would be to specify a environmnet variable with TimeZone definition including daylight adjustmnet rules.
     * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
     */
    //configTzTime(time_zone, ntpServer1, ntpServer2);

    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);

    gfx->println("Connecting...");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    gfx->fillScreen(BACKGROUND);
    Serial.println(" CONNECTED");
    gfx->println("CONNECTED");


    hHandLen = center * 3 / 8;
    mHandLen = center * 2 / 3;
    sHandLen = center * 5 / 6;
    markLen = sHandLen / 6;
    cached_points = (int16_t *) malloc((hHandLen + 1 + mHandLen + 1 + sHandLen + 1) * 2 * 2);

    gfx->fillScreen(BACKGROUND);

    // Draw 60 clock marks
    draw_round_clock_mark(
            // draw_square_clock_mark(
            center - markLen, center,
            center - (markLen * 2 / 3), center,
            center - (markLen / 2), center);

    hh = conv2d(__TIME__);
    mm = conv2d(__TIME__ + 3);
    ss = conv2d(__TIME__ + 6);

    targetTime = ((millis() / 1000) + 1) * 1000;
}

void loop() {
    unsigned long cur_millis = millis();
    if (cur_millis >= targetTime) {
        targetTime += 1000;
        ss++; // Advance second
        if (ss == 60) {
            ss = 0;
            mm++; // Advance minute
            if (mm > 59) {
                mm = 0;
                hh++; // Advance hour
                if (hh > 23) {
                    hh = 0;
                }
            }
        }
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = SIXTIETH_RADIAN * ((0.001 * (cur_millis % 1000)) + ss); // 0-59 (includes millis)
    nsx = cos(sdeg - RIGHT_ANGLE_RADIAN) * sHandLen + center;
    nsy = sin(sdeg - RIGHT_ANGLE_RADIAN) * sHandLen + center;
    if ((nsx != osx) || (nsy != osy)) {
        mdeg = (SIXTIETH * sdeg) + (SIXTIETH_RADIAN * mm); // 0-59 (includes seconds)
        hdeg = (TWELFTH * mdeg) + (TWELFTH_RADIAN * hh);   // 0-11 (includes minutes)
        mdeg -= RIGHT_ANGLE_RADIAN;
        hdeg -= RIGHT_ANGLE_RADIAN;
        nmx = cos(mdeg) * mHandLen + center;
        nmy = sin(mdeg) * mHandLen + center;
        nhx = cos(hdeg) * hHandLen + center;
        nhy = sin(hdeg) * hHandLen + center;

        // redraw hands
        redraw_hands_cached_draw_and_erase();

        ohx = nhx;
        ohy = nhy;
        omx = nmx;
        omy = nmy;
        osx = nsx;
        osy = nsy;

        delay(1);
    }
}

void draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3,
                           int16_t outerR3) {
    float x, y;
    int16_t x0, x1, y0, y1, innerR, outerR;
    uint16_t c;

    for (uint8_t i = 0; i < 60; i++) {
        if ((i % 15) == 0) {
            innerR = innerR1;
            outerR = outerR1;
            c = MARK_COLOR;
        } else if ((i % 5) == 0) {
            innerR = innerR2;
            outerR = outerR2;
            c = MARK_COLOR;
        } else {
            innerR = innerR3;
            outerR = outerR3;
            c = SUBMARK_COLOR;
        }

        mdeg = (SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN;
        x = cos(mdeg);
        y = sin(mdeg);
        x0 = x * outerR + center;
        y0 = y * outerR + center;
        x1 = x * innerR + center;
        y1 = y * innerR + center;

        gfx->drawLine(x0, y0, x1, y1, c);
    }
}

void draw_square_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3,
                            int16_t outerR3) {
    float x, y;
    int16_t x0, x1, y0, y1, innerR, outerR;
    uint16_t c;

    for (uint8_t i = 0; i < 60; i++) {
        if ((i % 15) == 0) {
            innerR = innerR1;
            outerR = outerR1;
            c = MARK_COLOR;
        } else if ((i % 5) == 0) {
            innerR = innerR2;
            outerR = outerR2;
            c = MARK_COLOR;
        } else {
            innerR = innerR3;
            outerR = outerR3;
            c = SUBMARK_COLOR;
        }

        if ((i >= 53) || (i < 8)) {
            x = tan(SIXTIETH_RADIAN * i);
            x0 = center + (x * outerR);
            y0 = center + (1 - outerR);
            x1 = center + (x * innerR);
            y1 = center + (1 - innerR);
        } else if (i < 23) {
            y = tan((SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN);
            x0 = center + (outerR);
            y0 = center + (y * outerR);
            x1 = center + (innerR);
            y1 = center + (y * innerR);
        } else if (i < 38) {
            x = tan(SIXTIETH_RADIAN * i);
            x0 = center - (x * outerR);
            y0 = center + (outerR);
            x1 = center - (x * innerR);
            y1 = center + (innerR);
        } else if (i < 53) {
            y = tan((SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN);
            x0 = center + (1 - outerR);
            y0 = center - (y * outerR);
            x1 = center + (1 - innerR);
            y1 = center - (y * innerR);
        }
        gfx->drawLine(x0, y0, x1, y1, c);
    }
}

void redraw_hands_cached_draw_and_erase() {
    gfx->startWrite();
    draw_and_erase_cached_line(center, center, nsx, nsy, SECOND_COLOR, cached_points, sHandLen + 1, false, false);
    draw_and_erase_cached_line(center, center, nhx, nhy, HOUR_COLOR, cached_points + ((sHandLen + 1) * 2), hHandLen + 1,
                               true, false);
    draw_and_erase_cached_line(center, center, nmx, nmy, MINUTE_COLOR,
                               cached_points + ((sHandLen + 1 + hHandLen + 1) * 2), mHandLen + 1, true, true);
    gfx->endWrite();
}

void draw_and_erase_cached_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color, int16_t *cache,
                                int16_t cache_len, bool cross_check_second, bool cross_check_hour) {
#if defined(ESP8266)
    yield();
#endif
    bool steep = _diff(y1, y0) > _diff(x1, x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    int16_t dx, dy;
    dx = _diff(x1, x0);
    dy = _diff(y1, y0);

    int16_t err = dx / 2;
    int8_t xstep = (x0 < x1) ? 1 : -1;
    int8_t ystep = (y0 < y1) ? 1 : -1;
    x1 += xstep;
    int16_t x, y, ox, oy;
    for (uint16_t i = 0; i <= dx; i++) {
        if (steep) {
            x = y0;
            y = x0;
        } else {
            x = x0;
            y = y0;
        }
        ox = *(cache + (i * 2));
        oy = *(cache + (i * 2) + 1);
        if ((x == ox) && (y == oy)) {
            if (cross_check_second || cross_check_hour) {
                write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
            }
        } else {
            write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
            if ((ox > 0) || (oy > 0)) {
                write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
            }
            *(cache + (i * 2)) = x;
            *(cache + (i * 2) + 1) = y;
        }
        if (err < dy) {
            y0 += ystep;
            err += dx;
        }
        err -= dy;
        x0 += xstep;
    }
    for (uint16_t i = dx + 1; i < cache_len; i++) {
        ox = *(cache + (i * 2));
        oy = *(cache + (i * 2) + 1);
        if ((ox > 0) || (oy > 0)) {
            write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
        }
        *(cache + (i * 2)) = 0;
        *(cache + (i * 2) + 1) = 0;
    }
}

void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour) {
    int16_t *cache = cached_points;
    if (cross_check_second) {
        for (uint16_t i = 0; i <= sHandLen; i++) {
            if ((x == *(cache++)) && (y == *(cache))) {
                return;
            }
            cache++;
        }
    }
    if (cross_check_hour) {
        cache = cached_points + ((sHandLen + 1) * 2);
        for (uint16_t i = 0; i <= hHandLen; i++) {
            if ((x == *(cache++)) && (y == *(cache))) {
                return;
            }
            cache++;
        }
    }
    gfx->writePixel(x, y, color);
}

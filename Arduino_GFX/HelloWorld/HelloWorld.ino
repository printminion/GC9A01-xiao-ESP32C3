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

#define TFT_MISO MISO
#define TFT_SCLK SCK
#define TFT_MOSI MOSI
#define TFT_CS   SS
#define TFT_DC   D4
#define TFT_RST  D5   // Reset pin (could connect to Arduino RESET pin)

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
// #if defined(DISPLAY_DEV_KIT)
// Arduino_GFX *gfx = create_default_Arduino_GFX();
// #else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);


/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST /* RST */, 0 /* rotation */, true /* IPS */);

//#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

void setup(void)
{
    gfx->begin();
    gfx->fillScreen(BLACK);

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    gfx->setCursor(10, 10);
    gfx->setTextColor(RED);
    gfx->println("Hello World!");

    delay(5000); // 5 seconds
}

void loop()
{
    gfx->setCursor(random(gfx->width()), random(gfx->height()));
    gfx->setTextColor(random(0xffff), random(0xffff));
    gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
    gfx->println("Hello World!");

    delay(1000); // 1 second
}

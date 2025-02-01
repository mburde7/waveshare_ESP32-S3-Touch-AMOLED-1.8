#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "16Bit_368x448px.h"
#include <ESP_IOExpander_Library.h>

#include "HWCDC.h"
HWCDC USBSerial;

static uint8_t Image_Flag = 0;

#define _EXAMPLE_CHIP_CLASS(name, ...) ESP_IOExpander_##name(__VA_ARGS__)
#define EXAMPLE_CHIP_CLASS(name, ...) _EXAMPLE_CHIP_CLASS(name, ##__VA_ARGS__)

ESP_IOExpander *expander = NULL;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_SH8601(bus, -1 /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

void setup() {
  USBSerial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);
  expander = new EXAMPLE_CHIP_CLASS(TCA95xx_8bit,
                                    (i2c_port_t)0, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000,
                                    IIC_SCL, IIC_SDA);
  expander->init();
  expander->begin();
  expander->pinMode(0, OUTPUT);
  expander->pinMode(1, OUTPUT);
  expander->pinMode(2, OUTPUT);
  expander->digitalWrite(0, LOW);
  expander->digitalWrite(1, LOW);
  expander->digitalWrite(2, LOW);
  delay(20);
  expander->digitalWrite(0, HIGH);
  expander->digitalWrite(1, HIGH);
  expander->digitalWrite(2, HIGH);

  while (FT3168->begin() == false) {
    USBSerial.println("FT3168 initialization fail");
    delay(2000);
  }
  USBSerial.println("FT3168 initialization successfully");

  gfx->begin();
  gfx->fillScreen(WHITE);

  for (int i = 0; i <= 255; i++) {
    gfx->Display_Brightness(i);
    delay(5);
  }

  gfx->fillScreen(RED);
  delay(1000);
  gfx->fillScreen(GREEN);
  delay(1000);
  gfx->fillScreen(BLUE);
  delay(1000);

  gfx->fillScreen(WHITE);
  gfx->setCursor(60, 100);
  gfx->setTextSize(5);
  gfx->setTextColor(BLACK);
  gfx->setCursor(60, 200);
  gfx->setTextSize(5);
  gfx->setTextColor(BLACK);
  gfx->printf("Touch me");

  USBSerial.printf("ID: %#X \n\n", (int32_t)FT3168->IIC_Read_Device_ID());
  delay(1000);
}

void loop() {
  if (FT3168->IIC_Interrupt_Flag == true) {
    FT3168->IIC_Interrupt_Flag = false;

    int32_t touch_x = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    int32_t touch_y = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
    uint8_t fingers_number = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

    if (fingers_number > 0) {
      switch (Image_Flag) {
        case 0:
          gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, LCD_WIDTH, LCD_HEIGHT);  // RGB
          break;
        case 1:
          gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, LCD_WIDTH, LCD_HEIGHT);  // RGB
          break;
        case 2:
          gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, LCD_WIDTH, LCD_HEIGHT);  // RGB
          break;
        case 3:
          gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, LCD_WIDTH, LCD_HEIGHT);  // RGB
          break;
        // case 4:
        //   gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, LCD_WIDTH, LCD_HEIGHT);  // RGB
        //   break;
        // case 5:
        //   gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_6, LCD_WIDTH, LCD_HEIGHT);  // RGB
        //   break;
        default:
          break;
      }

      Image_Flag++;

      if (Image_Flag > 3) {
        Image_Flag = 0;
      }
      delay(200);
    }
  }
}

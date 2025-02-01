#include "Wire.h"
#include "ESP_I2S.h"
I2SClass i2s;

#include "esp_check.h"
#include "es8311.h"
#include "canon.h"
#include "pin_config.h"

#define EXAMPLE_SAMPLE_RATE 16000
#define EXAMPLE_VOICE_VOLUME 80                  // 0 - 100
#define EXAMPLE_MIC_GAIN (es8311_mic_gain_t)(3)  // 0 - 7
#define EXAMPLE_RECV_BUF_SIZE (10000)


const char *TAG = "esp32p4_i2s_es8311";

esp_err_t es8311_codec_init(void) {
  es8311_handle_t es_handle = es8311_create(0, ES8311_ADDRRES_0);
  ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
  const es8311_clock_config_t es_clk = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = EXAMPLE_SAMPLE_RATE * 256,
    .sample_frequency = EXAMPLE_SAMPLE_RATE
  };

  ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
  ESP_RETURN_ON_ERROR(es8311_sample_frequency_config(es_handle, es_clk.mclk_frequency, es_clk.sample_frequency), TAG, "set es8311 sample frequency failed");
  ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");

  ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL), TAG, "set es8311 volume failed");
  ESP_RETURN_ON_ERROR(es8311_microphone_gain_set(es_handle, EXAMPLE_MIC_GAIN), TAG, "set es8311 microphone gain failed");
  return ESP_OK;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(PA, OUTPUT),
    digitalWrite(PA, HIGH);

  i2s.setPins(BCLKPIN, WSPIN, DIPIN, DOPIN, MCLKPIN);
  if (!i2s.begin(I2S_MODE_STD, EXAMPLE_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("Failed to initialize I2S bus!");
    return;
  }

  Wire.begin(IIC_SDA, IIC_SCL);

  es8311_codec_init();

  // i2s.write((uint8_t *)canon_pcm, canon_pcm_len);
  // Serial.println("[echo] Echo start");
}

void loop() {
  i2s.write((uint8_t *)canon_pcm, canon_pcm_len);
  vTaskDelay(1);
}

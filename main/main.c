#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"


static const char *TAG = "MAX31855";

float max31855_get_temperature(spi_device_handle_t spi) {
    uint8_t rx_data[4] = {0}; // 4バイトの受信バッファ
    spi_transaction_t t = {
        .length = 32,         // 転送データ長（ビット）
        .rx_buffer = rx_data, // 受信バッファ
    };

    // SPIデータ送受信
    spi_device_transmit(spi, &t);
  

    // 受信データの解析
    uint32_t raw_data = (rx_data[0] << 24) | (rx_data[1] << 16) | (rx_data[2] << 8) | rx_data[3];
 

    // 温度データを抽出（上位14ビットが温度）
    int16_t temp_data = (raw_data >> 18) & 0x3FFF;
    if (temp_data & 0x2000) { // 負の温度の処理（2の補数表現）
        temp_data |= 0xC000;
    }
    return temp_data * 0.25; // 温度（摂氏）を計算
}

void app_main(void)
{
  #define EEPROM_HOST SPI1_HOST
   gpio_set_direction(7, GPIO_MODE_OUTPUT);   

  spi_bus_config_t buscfg = {
      .miso_io_num = 5,
      .mosi_io_num = -1,
      .sclk_io_num = 4,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0,
      };

  spi_device_interface_config_t device = {
      .clock_speed_hz = 1 * 1000 * 1000, // 5Mhz
      .mode = 0,
      .spics_io_num = 7,
      .queue_size = 7,
  };

   ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
   spi_device_handle_t spi;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &device, &spi));
   while (1) {
        float temperature = max31855_get_temperature(spi);

        printf("Temperature: %.2f °C\n", temperature);
        printf("Hello World test serial\n");

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1秒待機
    }

}

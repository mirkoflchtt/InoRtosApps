/* Ino RTOS Example main.cpp

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "ino_rtos_adapter.h"
#include "ino_rtos_log.h"
#include "ino_rtos_network.h"

#include "driver/gpio.h"

#if defined(INO_TARGET_RTOS_ESP8266)
    #include "esp_system.h"
    // #include "esp_spi_flash.h"
    // Modern RTOS SDK v3.x path wrapper for flash APIs
    #include "spi_flash.h" 
#elif defined(INO_TARGET_RTOS_ESP32)  // FIXED: Changed from duplicate ESP8266 check
    #include "esp_chip_info.h"
    #include "esp_flash.h"
#else
    #error "Unsupported target hardware!"
#endif

#define INO_BLINK_GPIO          (GPIO_NUM_2)
#define INO_BLINK_MS            (1000)

// Standard C++ class example to handle the LED logic
class LedBlinker
{
public:
    typedef enum {
        ON = INO_ON,
        OFF = INO_OFF
    } State;

    LedBlinker(gpio_num_t pin) :
      m_pin(pin),
      m_state(ON)
      {}

    void toggle(void)
    {
        // Active-low logic for Wemos D1 Mini (0 = On, 1 = Off)
        m_state = (m_state == ON) ? OFF : ON;
        gpio_set_level(m_pin, m_state);
    }

    void init(void)
    {
#ifdef INO_TARGET_RTOS_ESP8266
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << m_pin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
#endif
#ifdef INO_TARGET_RTOS_ESP32
        gpio_reset_pin(m_pin);
        gpio_set_direction(m_pin, GPIO_MODE_OUTPUT);
#endif

        toggle();
    }

private:
    gpio_num_t      m_pin;
    State           m_state;
};


// Task function must accept a void pointer for FreeRTOS
void blink_task(void *pvParameters)
{
    LedBlinker blinker(INO_BLINK_GPIO);
    blinker.init();

    while (true)
    {
        blinker.toggle();
        INO_TASK_DELAY(INO_BLINK_MS);
    }
}

extern "C" void app_main(void)
{
    ino::Logger::getInstance().setThreshold(ino::LogLevel::DEBUG);

#ifdef INO_TARGET_RTOS_ESP32
    INO_LOGI("[Ino RTOS ESP32 C++ App]\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    INO_LOGI("This chip has %d CPU cores, WiFi", chip_info.cores);
    INO_LOGI(", silicon revision %d", chip_info.revision);

    uint32_t flash_size = 0;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        INO_LOGI(", %ldMB %s flash\n", (long int)(flash_size / (1024 * 1024)),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    } else {
        INO_LOGI(", failed to read flash size\n");
    }
#endif

#ifdef INO_TARGET_RTOS_ESP8266
    INO_LOGI("[Ino RTOS 8266 C++ App]\n");
    // Clean native printout matching legacy SDK functions directly
    INO_LOGI("This chip has 1 CPU core, WiFi\n");
    INO_LOGI("Flash size: %d MB\n", spi_flash_get_chip_size() / (1024 * 1024));
#endif

    // Utilizing our safe wrapped runtime creator to maintain multi-core layout logic
    ino::createTask(blink_task, "blink_task", INO_DEFAULT_STACK_SIZE, NULL, 5, 0);
}

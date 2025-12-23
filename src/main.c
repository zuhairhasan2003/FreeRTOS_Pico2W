#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

void vBlinkTask(void *pvParameters) {
    while (1) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    stdio_init_all();
    
    if (cyw43_arch_init()) {
        return -1;  // Init failed
    }
    
    xTaskCreate(vBlinkTask, "Blink", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    
    while (1);
}
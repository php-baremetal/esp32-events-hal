/*
   +----------------------------------------------------------------------+
   | php-baremetal: esp32-events-hal                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@aurecchia.com>             |
   +----------------------------------------------------------------------+
*/

/* ESP32 events HAL: FreeRTOS queue, esp_timer, GPIO interrupts, esp_pm lock. */
#include "bm_events_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_pm.h"
#include "phpbm.h"

int64_t bm_hal_now_us(void)
{
    return esp_timer_get_time();
}

/* The opaque handles are the ESP-IDF handles. */

static TickType_t to_ticks(uint32_t ms)
{
    return ms == UINT32_MAX ? portMAX_DELAY : pdMS_TO_TICKS(ms);
}

bm_hal_queue_t *bm_hal_queue_create(size_t depth, size_t item_size)
{
    return (bm_hal_queue_t *) xQueueCreate(depth, item_size);
}

bool bm_hal_queue_send(bm_hal_queue_t *q, const void *item, uint32_t wait_ms)
{
    return xQueueSend((QueueHandle_t) q, item, to_ticks(wait_ms)) == pdTRUE;
}

bool bm_hal_queue_send_isr(bm_hal_queue_t *q, const void *item)
{
    BaseType_t hp = pdFALSE;
    BaseType_t r = xQueueSendFromISR((QueueHandle_t) q, item, &hp);
    if (hp) {
        portYIELD_FROM_ISR();
    }
    return r == pdTRUE;
}

bool bm_hal_queue_receive(bm_hal_queue_t *q, void *out, uint32_t wait_ms)
{
    return xQueueReceive((QueueHandle_t) q, out, to_ticks(wait_ms)) == pdTRUE;
}

bm_hal_timer_t *bm_hal_timer_create(void (*cb)(void *arg), void *arg, const char *name)
{
    const esp_timer_create_args_t cfg = { .callback = cb, .arg = arg, .name = name };
    esp_timer_handle_t h;
    return esp_timer_create(&cfg, &h) == ESP_OK ? (bm_hal_timer_t *) h : NULL;
}

bool bm_hal_timer_start(bm_hal_timer_t *t, uint32_t period_ms)
{
    return esp_timer_start_periodic((esp_timer_handle_t) t, (uint64_t) period_ms * 1000) == ESP_OK;
}

void bm_hal_timer_stop(bm_hal_timer_t *t)
{
    esp_timer_stop((esp_timer_handle_t) t);
}

void bm_hal_gpio_input(int pin, int pull)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << pin,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = pull == BM_HAL_PULL_UP   ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE,
        .pull_down_en = pull == BM_HAL_PULL_DOWN ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
}

int bm_hal_gpio_level(int pin)
{
    return gpio_get_level((gpio_num_t) pin);
}

void bm_hal_gpio_on_falling(int pin, void (*isr)(void *arg), void *arg)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << pin,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io);
    static bool isr_service;
    if (!isr_service) {
        gpio_install_isr_service(0);
        isr_service = true;
    }
    gpio_isr_handler_add((gpio_num_t) pin, isr, arg);
}

bool bm_hal_power_save(void)
{
    return phpbm_power_save();
}

bm_hal_pmlock_t *bm_hal_pmlock_create(const char *name)
{
    esp_pm_lock_handle_t l;
    /* NULL when power management is off */
    return esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, name, &l) == ESP_OK ? (bm_hal_pmlock_t *) l : NULL;
}

void bm_hal_pmlock_acquire(bm_hal_pmlock_t *l)
{
    esp_pm_lock_acquire((esp_pm_lock_handle_t) l);
}

void bm_hal_pmlock_release(bm_hal_pmlock_t *l)
{
    esp_pm_lock_release((esp_pm_lock_handle_t) l);
}

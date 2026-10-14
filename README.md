# esp32-events-hal

The ESP32 side of [ext-events](package:php-baremetal/ext-events): the event queue, the periodic timers, the
GPIO interrupts and the light-sleep lock that `every()`, `watch_gpio()`, `watch_button()` and
`power_hold()` run on. It implements `bm_events_hal.h` from ext-events.

| Key | Value |
| --- | --- |
| Type | `bm-hal` |
| Platform | `esp32` (`extra.baremetal.platform`) |
| Provides | `php-baremetal/events-hal` |
| ESP-IDF components | `esp_timer`, `esp_driver_gpio`, `esp_pm`, `phpbm` (the SDK host component) |

## Install

Projects do not require it. A project requires `php-baremetal/ext-events` (or a package that needs it, such
as `ext-i2c` or `ext-web`), and conductor adds `php-baremetal/esp32-events-hal` because the project's SDK
(`"sdk": "esp32@^1.0"`) has the platform `esp32`:

```bash
composer require php-baremetal/ext-events
```

## Behaviour on ESP32

| ext-events feature | ESP-IDF mechanism |
| --- | --- |
| Event queue | A FreeRTOS queue. Posting never waits for sources; ISRs post with the ISR variant and yield if a higher-priority task woke. |
| `every()`, the button sampler | Periodic `esp_timer` timers. Their callbacks run on the `esp_timer` task (core 0) and only post to the queue. |
| `watch_gpio()` | The pin is configured as input with the internal pull-up and a falling-edge interrupt; the GPIO ISR service is installed on first use. |
| `watch_button()` | The pin is configured as input with a pull-up (pull-down when `activeLow` is false), interrupts off; the sampler reads its level. |
| `power_hold()` / `power_release()` | An `esp_pm` lock of type `ESP_PM_NO_LIGHT_SLEEP` named `php_hold`. Creating it fails when power management is off, and the two functions then do nothing. |
| power-save state | Read from the SDK host (`phpbm_power_save()`): true when the `power-save` option is on and power management started at boot. |
| Clock | `esp_timer_get_time()`, microseconds since boot. |

## AI usage

The policy for AI-assisted work on this package is in `AI_USAGE.md`, next to this README.

## License

MIT.

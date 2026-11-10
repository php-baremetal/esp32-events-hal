# Changelog

## 1.2.0

- Added: bm_hal_task_start(), bm_hal_alloc() and bm_hal_free() (internal RAM, PSRAM when the board has it, or PSRAM only, through heap_caps_malloc()); the package now provides php-baremetal/events-hal 1.5.0.

## 1.1.0

- Added: bm_hal_timer_start_us(), microsecond timer periods.
- Added: bm_hal_lock_create(), bm_hal_lock_enter(), bm_hal_lock_exit(), a portMUX spinlock usable from tasks and ISRs, for the event slot pools of ext-events.
- Added: bm_hal_gpio_off(), which detaches a pin's interrupt; the package now provides php-baremetal/events-hal 1.4.0.
- Fixed: bm_hal_now_us() read esp_timer_get_time(), about 3 ms behind PHP's hrtime(), so an event's $time looked 3 ms older than it was; it now reads esp_system_get_time(), the clock of hrtime().

## 1.0.2

- Added: `bm_hal_request_scoped()`, from the SDK's `phpbm_runtime_request_scoped()`. The package now
  provides `php-baremetal/events-hal` 1.1.0.

## 1.0.1

- Changed: the author's contact email.

## 1.0.0

Initial release.

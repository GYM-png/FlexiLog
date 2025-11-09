# FlexiLog - Flexible & High-Performance Embedded Logging Library

![FlexiLog Logo](https://img.shields.io/badge/FlexiLog-v1.0-brightgreen)
[![GitHub stars](https://img.shields.io/github/stars/GYM-png/FlexiLog?style=social)](https://github.com/GYM-png/FlexiLog/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/GYM-png/FlexiLog?style=social)](https://github.com/GYM-png/FlexiLog/network/members)
![License](https://img.shields.io/badge/License-Custom-blue)
![Platform](https://img.shields.io/badge/Platform-Embedded%20%7C%20Windows-orange)


[中文](https://github.com/GYM-png/FlexiLog/blob/master/README.md)|[English](https://github.com/GYM-png/FlexiLog/blob/master/README_EN.md)

**FlexiLog** is a **high-performance, highly configurable, multi-buffer logging system** designed for embedded systems and resource-constrained environments. It offers rich log formatting control, level filtering, tag filtering, ring buffer caching, event logging, color output, and a hardware abstraction layer (HAL), suitable for bare-metal, RTOS, and Windows development & debugging.

> Author: GYM  
> Email: 48060945@qq.com  
> Date: 2025-11-06  
> Version: v1.0.0  
> Repository: [https://github.com/GYM-png/FlexiLog](https://github.com/GYM-png/FlexiLog)

---

## Feature Highlights

| Feature               | Description                                                                 |
|-----------------------|-----------------------------------------------------------------------------|
| **Multi-Level Logging** | `DEBUG`, `INFO`, `WARN`, `ERROR`, `RECORD`, `ASSERT`                        |
| **Flexib[README_EN.md](..%2F..%2FMiddleWare%2Flwcli%2FREADME_EN.md)le Formatting** | Combine time, level, file, function, line, thread, tag, colors freely       |
| **ANSI Color Support** | Terminal foreground & background color support                              |
| **Ring Buffers**       | 4 independent ring buffers to prevent log loss                              |
| **Event Logging**      | Dedicated storage and retrieval for critical events                         |
| **Tag Filtering**      | Up to 5 tag-based filtering rules                                           |
| **Dynamic Memory**     | Optional `FLEXILOG_AUTO_MALLOC` for automatic allocation                    |
| **Hardware Abstraction** | Customizable UART, lock, time, and thread ID interfaces                     |
| **Hex Dump**           | Byte/half-word/word aligned memory printing                                 |
| **Efficient Output**   | High-performance `printf`-style interface                                   |

---

## Directory Structure

```text
FlexiLog/
├── inc/         # Header files (API definitions)
├── port/        # Hardware abstraction layer (user-modifiable)
├── src/         # Core implementation
├── example/     # Example code
└── README.md
```

---

## Quick Start

### 1. Add Files to Your Project

Include the following files in your project:

- `inc/*.h`
- `port/flexi_log_port.c` (hardware interface)
- `src/*.c` (core logic)

### 2. Configure Macros (in `flexi_log.h` or compiler options)

```c
/* Basic configuration */
#define FLEXILOG_LINE_MAX_LENGTH 1024        /* Max length of a single log line */
#define FLEXILOG_FILE_NAME_MAX_LENGTH 20     /* Max filename length */
#define FLEXILOG_FUNCTION_NAME_MAX_LENGTH 40 /* Max function name length */
#define FLEXILOG_TAG_MAX_LENGTH 16           /* Max tag length */
#define FLEXILOG_TAG_FILTER_NUM  5           /* Number of tag filters (0 = disable) */
#define FLEXILOG_USE_RING_BUFFER             /* Enable ring buffer logging */

/* Ring buffer options */
#ifdef FLEXILOG_USE_RING_BUFFER
#define FLEXILOG_AUTO_MALLOC                    /* Enable dynamic memory allocation */
#define FLEXILOG_USE_ALL_LOG_RING_BUFFER        /* Log all entries (unfiltered) */
#define FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER     /* Log all hardware output */
#define FLEXILOG_USE_RECOD_LOG_RING_BUFFER      /* Log RECORD level and above */
#define FLEXILOG_USE_EVENT_LOG_RING_BUFFER      /* Event-specific logging */
#endif
```

> Tip: To use static allocation, comment out `FLEXILOG_AUTO_MALLOC` and pass `FLOG_RingBuffer_Init_Paremeter` to `flog_init()`.

### 3. Initialize the Logging System

```c
#include "flexi_log.h"
#define FLOG_TAG "MAIN"

int main(void)
{
#ifdef FLEXILOG_AUTO_MALLOC
    /* Auto memory allocation */
    flog_init();
#else
    /* Static memory allocation */
    char all_buffer[1024];
    char output_buffer[1024];
    char record_buffer[1024];
    char event_buffer[1024];
    FLOG_RingBuffer_Init_Paremeter ring_buffer_param = {
        .all_log_buffer = all_buffer,
        .all_buffer_size = sizeof(all_buffer),
        .event_log_buffer = event_buffer,
        .event_buffer_size = sizeof(event_buffer),
        .output_log_buffer = output_buffer,
        .output_buffer_size = sizeof(output_buffer),
        .recod_log_buffer = record_buffer,
        .recod_buffer_size = sizeof(record_buffer),
    };
    flog_init(&ring_buffer_param);
#endif

    logi("FlexiLog initialized successfully!");
    return 0;
}
```

### 4. Use Logging Macros

```c
logd("This is debug message");                    // Visible only in development
logi("System started, version: %s", "1.0.0");
logw("Low battery warning: %d%%", 15);
loge("Failed to open device: %s", "COM3");
logr("Data saved to flash: %d bytes", 1024);
loga("Critical assertion failed!");               // Triggers assertion

// Event logging
log_event(FLOG_EVENT_0, "Sensor triggered! Value = %d", 998);

// Hex Dump
uint8_t data[] = {0x01, 0x02, 0x03, 0xAA, 0xFF};
flog_hex_dump("SENSOR Raw Data", data, 5, FLOG_DATA_TYPE_BYTE);
```

### 5. Customize Log Format

```c
flog_set_tag_filter(FLOG_TAG, FLOG_LEVEL_WARN); // Set tag filter to WARN
flog_disable_fmt(FLOG_LEVEL_RECORD, FLOG_FMT_FILE | FLOG_FMT_FUNC); // Disable file/function in RECORD logs
```

---

## Ring Buffer Reading
**Returns complete lines only — no truncation, ideal for printing or transmission**

```c
char buffer[2048];
uint32_t len;

// Read all logs
len = flog_read_all(buffer, sizeof(buffer));
printf("All logs (%u bytes):\n%s", len, buffer);

// Read hardware output logs
len = flog_read_output(buffer, sizeof(buffer));
printf("Output logs (%u bytes):\n%s", len, buffer);

// Read FLOG_EVENT_0 logs
len = flog_read_event(FLOG_EVENT_0, buffer, sizeof(buffer));
printf("FLOG_EVENT_0 logs (%u bytes):\n%s", len, buffer);
```

---

## Hardware Abstraction Layer (`flexi_log_port.c`)

You must implement the following functions based on your target platform:

| Function                                    | Purpose                                      |
|-------------------------------------------|----------------------------------------------|
| `flog_port_init()`                        | Initialize UART, USB CDC, etc.               |
| `flog_port_output(buf, size)`             | Output log data to hardware                  |
| `flog_port_lock()` / `flog_port_unlock()` | Thread-safe locking                          |
| `flog_port_get_time()`                    | Return formatted time string                 |
| `flog_port_get_thread()`                  | Return thread ID string                      |
| `flog_port_malloc()` / `flog_port_free()` | Dynamic memory (only with `AUTO_MALLOC`)     |

> Current example uses **Windows COM2 (115200 8N1)** — ready for PC debugging.

---

## Macro Configuration Details

| Macro                                    | Description                                                                 | Default |
|----------------------------------------|-----------------------------------------------------------------------------|---------|
| `FLEXILOG_LINE_MAX_LENGTH`             | Max length of a single log line                                             | 1024    |
| `FLEXILOG_USE_RING_BUFFER`             | Enable ring buffer logging                                                  | Enabled |
| `FLEXILOG_AUTO_MALLOC`                 | Enable dynamic memory allocation                                            | Enabled |
| `FLEXILOG_USE_ALL_LOG_RING_BUFFER`     | Cache all logs (unfiltered, including non-output)                           | 5KB     |
| `FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER`  | Cache logs sent to hardware                                                 | 2KB     |
| `FLEXILOG_USE_RECOD_LOG_RING_BUFFER`   | Cache RECORD level and above (configurable)                                 | 1KB     |
| `FLEXILOG_USE_EVENT_LOG_RING_BUFFER`   | Dedicated event buffer                                                      | 1KB     |
| `FLEXILOG_TAG_FILTER_NUM`              | Number of tag filters (0 = disable). **Tag filters override global level**  | 5       |

---

## Event System (Optional)

```c
typedef enum {
    FLOG_EVENT_0 = 0,
    FLOG_EVENT_1,
    // Add your custom events here
    FLOG_EVENT_NUM
} FLOG_EVENT;
```

Usage:
```c
log_event(FLOG_EVENT_0, "Door opened by user %d", uid);
```

---

## Color Support

```c
flog_set_font_color(FLOG_LEVEL_ERROR, FLOG_COLOR_LIGHT_RED);
flog_set_bg_color(FLOG_LEVEL_WARN, FLOG_COLOR_YELLOW);
```

> Supported on Windows Console and ANSI-enabled terminals (PuTTY, Tera Term, etc.)

---

## License

```
Copyright (c) 2025 GYM. All Rights Reserved.
```

This project is under a **proprietary license** and is intended for **learning, research, and personal projects only**.  
For commercial use, please contact the author for authorization: 48060945@qq.com

---

## Contributing

Issues and Pull Requests are welcome!

```bash
git clone https://github.com/GYM-png/FlexiLog.git
```

---

## Star This Project

If you find this project helpful, please give it a **Star** to support ongoing development!

---

> **FlexiLog - Take Full Control of Your Logs!**
# FlexiLog - 灵活高效的嵌入式日志库

![FlexiLog Logo](https://img.shields.io/badge/FlexiLog-v1.0-brightgreen)
[![GitHub stars](https://img.shields.io/github/stars/GYM-png/FlexiLog?style=social)](https://github.com/GYM-png/FlexiLog/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/GYM-png/FlexiLog?style=social)](https://github.com/GYM-png/FlexiLog/network/members)
![License](https://img.shields.io/badge/License-Custom-blue)
![Platform](https://img.shields.io/badge/Platform-Embedded%20%7C%20Windows-orange)

[中文](https://github.com/GYM-png/FlexiLog/blob/master/README.md)|[English](https://github.com/GYM-png/FlexiLog/blob/master/README_EN.md)

**FlexiLog** 是一款专为嵌入式系统和资源受限环境设计的**高性能、可配置、支持多缓冲区日志系统**。它提供丰富的日志格式控制、等级过滤、标签过滤、环形缓冲区缓存、事件日志、颜色输出以及硬件抽象层（HAL）接口，适用于裸机、RTOS 以及 Windows 开发调试。

> 作者：GYM  
> 邮箱：48060945@qq.com  
> 日期：2025-11-06  
> 版本：v1.0.0  
> 仓库：[https://github.com/GYM-png/FlexiLog](https://github.com/GYM-png/FlexiLog)

---

## 特性一览

| 功能            | 说明                                                   |
|---------------|------------------------------------------------------|
| **多等级日志**     | `DEBUG`, `INFO`, `WARN`, `ERROR`, `RECORD`, `ASSERT` |
| **灵活格式化**     | 时间、级别、文件、函数、行号、线程、标签、颜色等自由组合                         |
| **ANSI 颜色支持** | 支持终端字体颜色与背景色                                         |
| **环形缓冲区**     | 支持 4 种独立环形缓冲区，防止日志丢失                                 |
| **事件日志**      | 关键事件触发独立存储与读取                                        |
| **Tag 过滤**    | 多支持 Tag 过滤规则                                         |
| **自动内存管理**    | 可选 `FLEXILOG_AUTO_MALLOC` 动态分配                       |
| **硬件抽象层**     | 串口、锁、时间、线程 ID 均可自定义                                  |
| **Hex Dump**  | 支持字节/半字/字对齐的内存打印                                     |
| **灵活输出**      | 提供高效 `printf` 风格接口                                   |

---

## 目录结构

```text
FlexiLog/
├── inc/         # 头文件（API 定义）
├── port/        # 硬件抽象层接口（需用户实现或修改）
├── src/         # 核心实现
├── example/     # 示例代码
└── README.md
```

---

## 快速开始

### 1. 添加文件到项目

将以下文件加入你的工程：

- `inc/*h`
- `port/flexi_log.c`（硬件接口）
- `src/*.c`（核心逻辑）

### 2. 配置宏定义（在 `flexi_log.h` 或编译选项中）

```c
/* 基本参数配置 */
#define FLEXILOG_LINE_MAX_LENGTH 1024        /* 单行日志最大长度 */
#define FLEXILOG_FILE_NAME_MAX_LENGTH 20     /* 文件名最大长度 */
#define FLEXILOG_FUNCTION_NAME_MAX_LENGTH 40 /* 函数名最大长度 */
#define FLEXILOG_TAG_MAX_LENGTH 16           /* 标签最大长度 */
#define FLEXILOG_TAG_FILTER_NUM  5           /* tag过滤数量 @note 0表示关闭tag过滤 */
#define FLEXILOG_USE_RING_BUFFER             /* 是否使用环形缓冲区来记录日志 */

/* 多种环形缓冲区定义 */
#ifdef FLEXILOG_USE_RING_BUFFER
#define FLEXILOG_AUTO_MALLOC                    /* 使用自动分配内存 */
#define FLEXILOG_USE_ALL_LOG_RING_BUFFER        /* 使用全部环形缓冲区    @note 会对所有日志进行记录，不受任何过滤影响 */
#define FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER     /* 使用输出环形缓冲区    @note 会记录所有向硬件输出的日志 */
#define FLEXILOG_USE_RECOD_LOG_RING_BUFFER      /* 使用记录环形缓冲区    @note 会记录特定等级以上的日志，默认为FLOG_LEVEL_RECORD */
#define FLEXILOG_USE_EVENT_LOG_RING_BUFFER      /* 使用事件环形缓冲区    @note 会记录相关事件触发的日志 */
#endif
```

> 提示：如需静态分配内存，注释掉 `FLEXILOG_AUTO_MALLOC`，并使用 `FLOG_RingBuffer_Init_Paremeter` 初始化。

### 3. 初始化日志系统

```c
#include "flexi_log.h" // 引入头文件
#define FLOG_TAG "MAIN" // 定义本文件的标签

int main(void)
{     
 #ifdef FLEXILOG_AUTO_MALLOC
    /* 自动分配内存 */
    flog_init();
 #else   
    /* 静态分配内存 */
    char all_buffer[1024];  
    char output_buffer[1024];
    char record_buffer[1024];
    char event_buffer[1024];
    FLOG_RingBuffer_Init_Paremeter ring_buffer_paremeter = {
            .all_log_buffer = all_buffer,
            .all_buffer_size = sizeof(all_buffer),
            .event_log_buffer = event_buffer,
            .event_buffer_size = sizeof(event_buffer),
            .output_log_buffer = output_buffer,
            .output_buffer_size = sizeof(output_buffer),
            .recod_log_buffer = record_buffer,
            .recod_buffer_size = sizeof(record_buffer),
    };
    flog_init(&ring_buffer_paremeter);
 #endif
    
    logi("FlexiLog initialized successfully!");
    return 0;
}
```

### 4. 使用日志宏

```c
logd("This is debug message");                    // 仅开发时可见
logi("System started, version: %s", "1.0.0");
logw("Low battery warning: %d%%", 15);
loge("Failed to open device: %s", "COM3");
logr("Data saved to flash: %d bytes", 1024);
loga("Critical assertion failed!");               // 触发断言

// 事件日志
log_event(FLOG_EVENT_0, "Sensor triggered! Value = %d", 998);

// Hex Dump
uint8_t data[] = {0x01, 0x02, 0x03, 0xAA, 0xFF};
flog_hex_dump("SENSOR Raw Data", data, 5, FLOG_DATA_TYPE_BYTE);
```

### 5. 修改日志格式日志
```c
    flog_set_tag_filter(FLOG_TAG, FLOG_LEVEL_WARN); // 设置当前标签过滤等级为Warn
    flog_disable_fmt(FLOG_LEVEL_RECORD, FLOG_FMT_FILE | FLOG_FMT_FUNC); // 禁用记录等级的文件名和函数名
```

---

## 环形缓冲区读取
**读取返回一定为完整的行，不会被截断，便于打印或对外传输**
```c
char buffer[2048];
uint32_t len;

// 读取所有日志
len = flog_read_all(buffer, sizeof(buffer));
printf("All logs (%u bytes):\n%s", len, buffer);

// 读取硬件输出的日志
len = flog_read_output(buffer, sizeof(buffer));
printf("Output logs (%u bytes):\n%s", len, buffer);

// 读取FLOG_EVENT_0事件日志
len = flog_read_event(FLOG_EVENT_0, buffer, sizeof(buffer));
printf("FLOG_EVENT_0 logs (%u bytes):\n%s", len, buffer);
```

---

## 硬件抽象层接口（`flexi_log_port.c`）

你需要根据目标平台实现以下函数：

| 函数                                        | 功能                      |
|-------------------------------------------|-------------------------|
| `flog_port_init()`                        | 初始化硬件：UART、USB CDC 等    |
| `flog_port_output(buf, size)`             | 输出日志到硬件                 |
| `flog_port_lock()` / `flog_port_unlock()` | 线程安全锁                   |
| `flog_port_get_time()`                    | 返回格式化时间字符串              |
| `flog_port_get_thread()`                  | 返回线程 ID 字符串             |
| `flog_port_malloc()` / `flog_port_free()` | 动态内存（仅 `AUTO_MALLOC` 时） |

> 当前示例为 **Windows COM2 串口（115200 8N1）**，可直接用于 PC 端调试。

---

## 宏配置详解

| 宏                                     | 说明                            | 默认   |
|---------------------------------------|-------------------------------|------|
| `FLEXILOG_LINE_MAX_LENGTH`            | 单行最大长度                        | 1024 |
| `FLEXILOG_USE_RING_BUFFER`            | 启用环形缓冲区                       | 启用   |
| `FLEXILOG_AUTO_MALLOC`                | 自动分配内存                        | 启用   |
| `FLEXILOG_USE_ALL_LOG_RING_BUFFER`    | 缓存所有日志(不受任何过滤，未输出到硬件的日志也会记录)  | 5KB  |
| `FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER` | 缓存输出日志                        | 2KB  |
| `FLEXILOG_USE_RECOD_LOG_RING_BUFFER`  | 缓存 RECORD 及以上(等级可调整)          | 1KB  |
| `FLEXILOG_USE_EVENT_LOG_RING_BUFFER`  | 事件专用缓冲区                       | 1KB  |
| `FLEXILOG_TAG_FILTER_NUM`             | Tag 过滤数量（0=关闭），Tag过滤优先级高于全局过滤 | 5    |

---

## 事件系统（可选）

```c
typedef enum {
    FLOG_EVENT_0 = 0,
    FLOG_EVENT_1,
    // 在这里添加你的自定义事件
    FLOG_EVENT_NUM
} FLOG_EVENT;
```

使用：
```c
log_event(FLOG_EVENT_0, "Door opened by user %d", uid);
```

---

## 颜色支持

```c
flog_set_font_color(FLOG_LEVEL_ERROR, FLOG_COLOR_LIGHT_RED);
flog_set_bg_color(FLOG_LEVEL_WARN, FLOG_COLOR_YELLOW);
```

> 支持 Windows 控制台、支持 ANSI 转义码的终端（如 PuTTY、Tera Term）

---

## 许可证

```
Copyright (c) 2025 GYM. All Rights Reserved.
```

本项目采用 **专有许可证**，仅限学习、研究及个人项目使用。  
商业使用请联系作者授权：48060945@qq.com

---

## 贡献代码

欢迎提交 Issue 和 Pull Request！

```bash
git clone https://github.com/GYM-png/FlexiLog.git
```

---

## 星标支持 ⭐

如果你觉得这个项目有用，请给仓库点个 **Star**，鼓励作者持续更新！

---

> **FlexiLog - 日志，从此灵活掌控！**

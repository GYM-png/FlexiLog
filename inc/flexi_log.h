/**
 * ==================================================
 *  @file flexi_log.h
 *  @brief flexi log 日志库头文件
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-06 下午10:10
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */
#ifndef FLEXILOG_FLEXI_LOG_H
#define FLEXILOG_FLEXI_LOG_H

#include "stdint.h"
#include "stdbool.h"

#ifndef __FILE_NAME__
#include <string.h>
#define __FILE_NAME__   (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : \
                        (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))
#endif // __FILE_NAME__

#define flexlog_assert(expr)    do                  \
                                {                   \
                                    if (!(expr))    \
                                    {               \
                                        flog_printf(false, "[%s:%d] %s\r\n", __FILE_NAME__, __LINE__, #expr);\
                                        while (1);  \
                                    }               \
                                }while(0);


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
// #define FLEXILOG_USE_ALL_LOG_RING_BUFFER        /* 使用全部环形缓冲区    @note 会对所有日志进行记录，不受任何过滤影响 */
#define FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER     /* 使用输出环形缓冲区    @note 会记录所有向硬件输出的日志 */
#define FLEXILOG_USE_RECOD_LOG_RING_BUFFER      /* 使用记录环形缓冲区    @note 会记录特定等级以上的日志，默认为FLOG_LEVEL_RECORD */
// #define FLEXILOG_USE_EVENT_LOG_RING_BUFFER      /* 使用事件环形缓冲区    @note 会记录相关事件触发的日志 */

/* 启用自动分配内存后会在flog_init函数中分配内存 */
#ifdef FLEXILOG_AUTO_MALLOC
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
#define FLEXILOG_ALL_RING_BUFFER_SIZE (5 * 1024)    /* 全部环形缓冲区 的大小 */
#endif
#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
#define FLEXILOG_OUTPUT_RING_BUFFER_SIZE (2 * 1024) /* 输出环形缓冲区 的大小 */
#endif
#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
#define FLEXILOG_RECOD_RING_BUFFER_SIZE (1 * 1024) /* 记录环形缓冲区 的大小 */
#endif
#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
#define FLEXILOG_EVENT_RING_BUFFER_SIZE (1 * 1024) /* 事件环形缓冲区 的大小 */
#endif
#else
/* 环形缓冲区的初始化参数 */
typedef struct
{
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    uint32_t all_buffer_size;    /* 全部环形缓冲区 的大小 */
    char *all_log_buffer;        /* 存储全部日志的缓冲区 */
#endif
#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    uint32_t output_buffer_size;    /* 输出环形缓冲区 的大小 */
    char *output_log_buffer;        /* 存储输出日志的缓冲区 */
#endif
#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
    uint32_t recod_buffer_size;    /* 记录环形缓冲区 的大小 */
    char *recod_log_buffer;       /* 存储记录日志的缓冲区 */
#endif
#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
    uint32_t event_buffer_size;    /* 事件环形缓冲区 的大小 */
    char *event_log_buffer;        /* 存储事件日志的缓冲区 */
#endif
}FLOG_RingBuffer_Init_Paremeter;
#endif // FLEXILOG_AUTO_MALLOC

/* 事件定义 */
#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
typedef enum
{
    FLOG_EVENT_0 = 0,    /* 可以修改事件的命名, 可以增加和减少事件的数量, 但是不可以修改事件的值 */
    FLOG_EVENT_1,
    /* add your event began */

    /* add your event end */
    FLOG_EVENT_NUM   /* 事件数量 这个定义不可修改 */
}FLOG_EVENT;
#endif  // FLEXILOG_USE_EVENT_LOG_RING_BUFFER
#endif // FLEXILOG_USE_RING_BUFFER

/**
 * @brief 日志等级, 优先级依次递增
 */
typedef enum
{
    FLOG_LEVEL_DEBUG = 0,   /* 调试信息 */
    FLOG_LEVEL_INFO,        /* 提示信息 */
    FLOG_LEVEL_WARN,        /* 警告信息 */
    FLOG_LEVEL_ERROR,       /* 错误信息 */
    FLOG_LEVEL_RECORD,      /* 记录信息 */
    FLOG_LEVEL_ASSERT,      /* 断言信息 */
    FLOG_LEVEL_UNVALID      /* 无效等级 */
}FLOG_LEVEL;

/**
 * @brief 日志格式
 */
typedef enum
{
    FLOG_FMT_NONE       = 0x00,      /* 无格式 */
    FLOG_FMT_TIME       = 0x01 << 0, /* 时间信息 */
    FLOG_FMT_LEVEL      = 0x01 << 1, /* 等级信息 */
    FLOG_FMT_FILE       = 0x01 << 2, /* 文件信息 */
    FLOG_FMT_FUNC       = 0x01 << 3, /* 函数信息 */
    FLOG_FMT_LINE       = 0x01 << 4, /* 行号信息 */
    FLOG_FMT_THREAD     = 0x01 << 5, /* 线程信息 */
    FLOG_FMT_TAG        = 0x01 << 6, /* 标签信息 */
    FLOG_FMT_FONT_COLOR = 0x01 << 7, /* 字体颜色信息 */
    FLOG_FMT_BG_COLOR   = 0x01 << 8, /* 背景颜色信息 */
    FLOG_FMT_ALL        = 0x1FFF     /* 所有格式 */
}FLOG_FMT;

/**
 * @brief 字体颜色
 */
typedef enum
{
    FLOG_COLOR_BLACK = 0,       /* 黑色 */
    FLOG_COLOR_RED,             /* 红色 */
    FLOG_COLOR_GREEN,           /* 绿色 */
    FLOG_COLOR_YELLOW,          /* 黄色 */
    FLOG_COLOR_BLUE,            /* 蓝色 */
    FLOG_COLOR_PURPLE,          /* 紫色 */
    FLOG_COLOR_CYAN,            /* 青色 */
    FLOG_COLOR_WHITE,           /* 白色 */
    FLOG_COLOR_LIGHT_BLACK,     /* 亮黑色 */
    FLOG_COLOR_LIGHT_RED,       /* 亮红色 */
    FLOG_COLOR_LIGHT_GREEN,     /* 亮绿色 */
    FLOG_COLOR_LIGHT_YELLOW,    /* 亮黄色 */
    FLOG_COLOR_LIGHT_BLUE,      /* 亮蓝色 */
    FLOG_COLOR_LIGHT_PURPLE,    /* 亮紫色 */
    FLOG_COLOR_LIGHT_CYAN,      /* 亮青色 */
    FLOG_COLOR_LIGHT_WHITE,     /* 亮白色 */
    FLOG_COLOR_UNVALID     /* 无效颜色 */
}FLOG_COLOR;

/**
 * @brief 数据类型
 */
typedef enum
{
    FLOG_DATA_TYPE_BYTE = 0,        /* 单字节 uint8_t */
    FLOG_DATA_TYPE_HALF_WORD,       /* 半字  uint16_t */
    FLOG_DATA_TYPE_WORD,            /* 单字  uint32_t */
}FLOG_DATA_TYPE;


#if defined(FLEXILOG_USE_RING_BUFFER) && !defined(FLEXILOG_AUTO_MALLOC)
void flog_init(FLOG_RingBuffer_Init_Paremeter *parameter);
#else
void flog_init(void);
#endif

void flog_enable_fmt(FLOG_LEVEL level, uint16_t fmt);
void flog_disable_fmt(FLOG_LEVEL level, uint16_t fmt);
void flog_set_level_fmt(FLOG_LEVEL level, uint16_t fmt);
void flog_set_font_color(FLOG_LEVEL level, FLOG_COLOR color);
void flog_set_bg_color(FLOG_LEVEL level, FLOG_COLOR color);

void flog_hardware_output_enable(bool enable);
void flog_lock_enable(bool enable);

void flog_set_global_filter(FLOG_LEVEL level);
#if (FLEXILOG_TAG_FILTER_NUM > 0)
void flog_set_tag_filter(const char *tag, FLOG_LEVEL level);
#endif

void flog_printf(bool write_ring_buffer, const char *fmt, ...);
void flog_output(FLOG_LEVEL level, const char *tag, const char *file, const char *func, uint32_t line, const char *fmt, ...);
void flog_hex_dump(char *tag, void *title, uint32_t size, FLOG_DATA_TYPE type);
#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
void flog_output_event(FLOG_EVENT event, const char *file, const char *func, uint32_t line, const char *fmt, ...);
#endif

/* 环形缓冲区读取接口 */
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
uint32_t flog_read_all(char *data, uint32_t size);
#endif // FLEXILOG_USE_ALL_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
uint32_t flog_read_output(char *data, uint32_t size);
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
uint32_t flog_read_record(char *data, uint32_t size);
#endif // FLEXILOG_USE_RECOD_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
uint32_t flog_read_event(FLOG_EVENT event, char *data, uint32_t size);
#endif // FLEXILOG_USE_EVENT_LOG_RING_BUFFER

/* 日志接口输出 */
#define log_printf(...) flog_printf(true, __VA_ARGS__); /* 全功能printf 函数, 不受任何配置影响 */
#define logd(...) flog_output(FLOG_LEVEL_DEBUG, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__)  /* 调试日志 */
#define logi(...) flog_output(FLOG_LEVEL_INFO, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__)   /* 提示日志 */
#define logw(...) flog_output(FLOG_LEVEL_WARN, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__)   /* 警告日志 */
#define loge(...) flog_output(FLOG_LEVEL_ERROR, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__)  /* 错误日志 */
#define logr(...) flog_output(FLOG_LEVEL_RECORD, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__) /* 记录日志 */
#define loga(...) flog_output(FLOG_LEVEL_ASSERT, FLOG_TAG, __FILE_NAME__, __FUNCTION__,  __LINE__, __VA_ARGS__) /* 断言日志 */
#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
#define log_event(event, ...) flog_output_event(event, __FILE_NAME__, __FUNCTION__, __LINE__, __VA_ARGS__)      /* 事件日志 */
#endif
#endif //FLEXILOG_FLEXI_LOG_H

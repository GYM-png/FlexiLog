/**
 * ==================================================
 *  @file flexi_log.c
 *  @brief flexi log核心实现文件
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-06 下午10:10
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */
#include "flexi_log.h"
#include "flexi_log_until.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#ifdef FLEXILOG_USE_RING_BUFFER
#include "flexi_log_rb.h"
#endif

#define FLOG_TAG "FLOG"
#define FLOG_VERSION "1.0.0"

/* flexi_log_port */
extern void flog_port_init(void);
extern void flog_port_output(const char *buf, size_t size);
extern void flog_port_lock(void);
extern void flog_port_unlock(void);
extern const char *flog_port_get_time(void);
extern const char *flog_port_get_thread(void);
#ifdef FLEXILOG_AUTO_MALLOC
extern void *flog_port_malloc(size_t size);
extern void flog_port_free(void *ptr);
#endif

/**
 * @brief 颜色格式
 */
#define FLOG_COLOR_START "\x1B["
#define FLOG_COLOR_ADD   ";"
#define FLOG_COLOR_END   "m"

#define FLOG_COLOR_REST  "\x1B[0m"

#define FLOG_FONT_COLOR_BLACK    "30"
#define FLOG_FONT_COLOR_RED      "31"
#define FLOG_FONT_COLOR_GREEN    "32"
#define FLOG_FONT_COLOR_YELLOW   "33"
#define FLOG_FONT_COLOR_BLUE     "34"
#define FLOG_FONT_COLOR_PURPLE   "35"
#define FLOG_FONT_COLOR_CYAN     "36"
#define FLOG_FONT_COLOR_WHITE    "37"
#define FLOG_FONT_COLOR_LIGHT_BLACK    "100"
#define FLOG_FONT_COLOR_LIGHT_RED      "91"
#define FLOG_FONT_COLOR_LIGHT_GREEN    "92"
#define FLOG_FONT_COLOR_LIGHT_YELLOW   "93"
#define FLOG_FONT_COLOR_LIGHT_BLUE     "94"
#define FLOG_FONT_COLOR_LIGHT_PURPLE   "95"
#define FLOG_FONT_COLOR_LIGHT_CYAN     "96"
#define FLOG_FONT_COLOR_LIGHT_WHITE    "97"

#define FLOG_BG_COLOR_BLACK    "40"
#define FLOG_BG_COLOR_RED      "41"
#define FLOG_BG_COLOR_GREEN    "42"
#define FLOG_BG_COLOR_YELLOW   "43"
#define FLOG_BG_COLOR_BLUE     "44"
#define FLOG_BG_COLOR_PURPLE   "45"
#define FLOG_BG_COLOR_CYAN     "46"
#define FLOG_BG_COLOR_WHITE    "47"
#define FLOG_BG_COLOR_LIGHT_BLACK    "100"
#define FLOG_BG_COLOR_LIGHT_RED      "101"
#define FLOG_BG_COLOR_LIGHT_GREEN    "102"
#define FLOG_BG_COLOR_LIGHT_YELLOW   "103"
#define FLOG_BG_COLOR_LIGHT_BLUE     "104"
#define FLOG_BG_COLOR_LIGHT_PURPLE   "105"
#define FLOG_BG_COLOR_LIGHT_CYAN     "106"
#define FLOG_BG_COLOR_LIGHT_WHITE    "107"

/**
 * @brief 等级提示
 */
#define FLOG_LEVLE_STR_DEBUG    "-D"
#define FLOG_LEVLE_STR_INFO     "-I"
#define FLOG_LEVLE_STR_WARN     "-W"
#define FLOG_LEVLE_STR_ERROR    "-E"
#define FLOG_LEVLE_STR_RECORD   "-R"
#define FLOG_LEVLE_STR_ASSERT   "-A"

#define FLOG_NEW_LINE "\r\n"

/**
 * @brief 默认格式
 */
#define FLOG_FMT_DEFAULT (FLOG_FMT_TIME | FLOG_FMT_TAG | FLOG_FMT_FONT_COLOR)


#define FLOG_LOCK()     do                               \
                        {                                \
                            if (flog.output_lock_enbale) \
                            {                            \
                                flog_port_lock();        \
                            }                            \
                        }while(0)

#define FLOG_UNLOCK()   do                               \
                        {                                \
                            if (flog.output_lock_enbale) \
                            {                            \
                                flog_port_unlock();      \
                            }                            \
                        }while(0)

/**
 * @brief 文本颜色表
 */
static const char *flog_font_color_table[FLOG_COLOR_UNVALID] =
{
        FLOG_FONT_COLOR_BLACK,
        FLOG_FONT_COLOR_RED,
        FLOG_FONT_COLOR_GREEN,
        FLOG_FONT_COLOR_YELLOW,
        FLOG_FONT_COLOR_BLUE,
        FLOG_FONT_COLOR_PURPLE,
        FLOG_FONT_COLOR_CYAN,
        FLOG_FONT_COLOR_WHITE,
        FLOG_FONT_COLOR_LIGHT_BLACK,
        FLOG_FONT_COLOR_LIGHT_RED,
        FLOG_FONT_COLOR_LIGHT_GREEN,
        FLOG_FONT_COLOR_LIGHT_YELLOW,
        FLOG_FONT_COLOR_LIGHT_BLUE,
        FLOG_FONT_COLOR_LIGHT_PURPLE,
        FLOG_FONT_COLOR_LIGHT_CYAN,
        FLOG_FONT_COLOR_LIGHT_WHITE,
};

/**
 * @brief 背景颜色表
 */
static const char *flog_bg_color_table[FLOG_COLOR_UNVALID] =
{
        FLOG_BG_COLOR_BLACK,
        FLOG_BG_COLOR_RED,
        FLOG_BG_COLOR_GREEN,
        FLOG_BG_COLOR_YELLOW,
        FLOG_BG_COLOR_BLUE,
        FLOG_BG_COLOR_PURPLE,
        FLOG_BG_COLOR_CYAN,
        FLOG_BG_COLOR_WHITE,
        FLOG_BG_COLOR_LIGHT_BLACK,
        FLOG_BG_COLOR_LIGHT_RED,
        FLOG_BG_COLOR_LIGHT_GREEN,
        FLOG_BG_COLOR_LIGHT_YELLOW,
        FLOG_BG_COLOR_LIGHT_BLUE,
        FLOG_BG_COLOR_LIGHT_PURPLE,
        FLOG_BG_COLOR_LIGHT_CYAN,
        FLOG_BG_COLOR_LIGHT_WHITE
};

/**
 * @brief 等级提示表
 */
static const char *flog_level_str_table[FLOG_LEVEL_UNVALID] =
{
    FLOG_LEVLE_STR_DEBUG,
    FLOG_LEVLE_STR_INFO,
    FLOG_LEVLE_STR_WARN,
    FLOG_LEVLE_STR_ERROR,
    FLOG_LEVLE_STR_RECORD,
    FLOG_LEVLE_STR_ASSERT
};


/**
 * @brief FLOG 结构体
 */
typedef struct
{
    char line_buffer[FLEXILOG_LINE_MAX_LENGTH];
    uint16_t level_fmt[FLOG_LEVEL_UNVALID];          // 每个日志等级对应的格式
    FLOG_COLOR font_color[FLOG_LEVEL_UNVALID]; // 每个日志等级对应的字体颜色
    FLOG_COLOR bg_color[FLOG_LEVEL_UNVALID];     // 每个日志等级对应的背景颜色
    bool hardware_output_enable;
    bool output_lock_enbale;
    bool output_color_enable;
    FLOG_LEVEL global_filter_level;

#if (FLEXILOG_TAG_FILTER_NUM > 0)
    struct flog_filter_t/* tag 过滤 */
    {
        char tag[FLEXILOG_TAG_MAX_LENGTH + 1];
        FLOG_LEVEL level;
    }tag_filters[FLEXILOG_TAG_FILTER_NUM];
#endif // FLEXILOG_TAG_FILTER_NUM > 0

#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    flog_ring_buffer_t ring_buffer_all;
#endif  // FLEXILOG_USE_ALL_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_ring_buffer_t ring_buffer_output;
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
    flog_ring_buffer_t ring_buffer_recod;
    FLOG_LEVEL recod_level;
#endif // FLEXILOG_USE_RECOD_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
    struct flog_event_ring_buffer_t{
        FLOG_EVENT event;
        flog_ring_buffer_t ring_bufer;
    }event_ring_buffer[FLOG_EVENT_NUM];
#endif // FLEXILOG_USE_EVENT_LOG_RING_BUFFER
}flog_t;
static flog_t flog;


#if defined(FLEXILOG_USE_RING_BUFFER) && !defined(FLEXILOG_AUTO_MALLOC)
/**
 * @brief 静态初始化
 * @param parameter 静态初始化参数
 */
void flog_init(FLOG_RingBuffer_Init_Paremeter *parameter)
#else
/**
 * @brief 初始化
 */
void flog_init(void)
#endif
{
    flog_port_init();
    flog.font_color[FLOG_LEVEL_DEBUG] = FLOG_COLOR_LIGHT_WHITE;
    flog.font_color[FLOG_LEVEL_INFO] = FLOG_COLOR_GREEN;
    flog.font_color[FLOG_LEVEL_WARN] = FLOG_COLOR_YELLOW;
    flog.font_color[FLOG_LEVEL_ERROR] = FLOG_COLOR_RED;
    flog.font_color[FLOG_LEVEL_RECORD] = FLOG_COLOR_PURPLE;
    flog.font_color[FLOG_LEVEL_ASSERT] = FLOG_COLOR_WHITE;
    flog.bg_color[FLOG_LEVEL_DEBUG] = FLOG_COLOR_UNVALID;
    flog.bg_color[FLOG_LEVEL_INFO] = FLOG_COLOR_UNVALID;
    flog.bg_color[FLOG_LEVEL_WARN] = FLOG_COLOR_UNVALID;
    flog.bg_color[FLOG_LEVEL_ERROR] = FLOG_COLOR_UNVALID;
    flog.bg_color[FLOG_LEVEL_RECORD] = FLOG_COLOR_UNVALID;
    flog.bg_color[FLOG_LEVEL_ASSERT] = FLOG_COLOR_UNVALID;

    flog.level_fmt[FLOG_LEVEL_DEBUG] = FLOG_FMT_DEFAULT;
    flog.level_fmt[FLOG_LEVEL_INFO] = FLOG_FMT_DEFAULT;
    flog.level_fmt[FLOG_LEVEL_WARN] = FLOG_FMT_DEFAULT;
    flog.level_fmt[FLOG_LEVEL_ERROR] = FLOG_FMT_DEFAULT;
    flog.level_fmt[FLOG_LEVEL_RECORD] = FLOG_FMT_DEFAULT | FLOG_FMT_THREAD | FLOG_FMT_FUNC;
    flog.level_fmt[FLOG_LEVEL_ASSERT] = FLOG_FMT_DEFAULT | FLOG_FMT_THREAD | FLOG_FMT_FUNC;

    flog.global_filter_level = FLOG_LEVEL_INFO;
    flog.hardware_output_enable = true;
    flog.output_lock_enbale = true;
    flog.output_color_enable = true;


#if (FLEXILOG_TAG_FILTER_NUM > 0)
    memset(flog.tag_filters, 0, sizeof(flog.tag_filters));
#endif // FLEXILOG_TAG_FILTER_NUM > 0

#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    memset(&flog.ring_buffer_all, 0, sizeof(flog_ring_buffer_t));
    #ifdef FLEXILOG_AUTO_MALLOC
    flog_rb_buffer_create(&flog.ring_buffer_all, FLEXILOG_ALL_RING_BUFFER_SIZE);
    #else
    flexlog_assert(parameter->all_log_buffer != NULL);
    flog_rb_init(&flog.ring_buffer_all, parameter->all_log_buffer, parameter->all_buffer_size);
    #endif
#endif  // FLEXILOG_USE_ALL_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    memset(&flog.ring_buffer_output, 0, sizeof(flog_ring_buffer_t));
    #ifdef FLEXILOG_AUTO_MALLOC
    flog_rb_buffer_create(&flog.ring_buffer_output, FLEXILOG_OUTPUT_RING_BUFFER_SIZE);
    #else
    flexlog_assert(parameter->output_log_buffer != NULL);
    flog_rb_init(&flog.ring_buffer_output, parameter->output_log_buffer, parameter->output_buffer_size);
    #endif
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
    memset(&flog.ring_buffer_recod, 0, sizeof(flog_ring_buffer_t));
    flog.recod_level = FLOG_LEVEL_RECORD;
    #ifdef FLEXILOG_AUTO_MALLOC
    flog_rb_buffer_create(&flog.ring_buffer_recod, FLEXILOG_RECOD_RING_BUFFER_SIZE);
    #else
    flexlog_assert(parameter->recod_log_buffer != NULL);
    flog_rb_init(&flog.ring_buffer_recod, parameter->recod_log_buffer, parameter->recod_buffer_size);
    #endif
#endif // FLEXILOG_USE_RECOD_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
    for (int i = 0; i < FLOG_EVENT_NUM; ++i)
    {
        flog.event_ring_buffer[i].event = i;
        #ifdef FLEXILOG_AUTO_MALLOC
        flog_rb_buffer_create(&flog.event_ring_buffer[i].ring_bufer, FLEXILOG_EVENT_RING_BUFFER_SIZE / FLOG_EVENT_NUM);
        #else
        flexlog_assert(parameter->event_log_buffer != NULL);
        uint32_t offset = i * parameter->event_buffer_size / FLOG_EVENT_NUM;
        flog_rb_init(&flog.event_ring_buffer[i].ring_bufer, parameter->event_log_buffer + offset, parameter->event_buffer_size / FLOG_EVENT_NUM);
        #endif
    }
#endif // FLEXILOG_USE_EVENT_LOG_RING_BUFFER
    flog_printf(false, "Flexi Log init ok, version: %s\r\n", FLOG_VERSION);
}


#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
#ifndef FLEXILOG_AUTO_MALLOC
void flog_set_ringbuffer_all(char *buffer, uint32_t size)
{
    flog_rb_init(&flog.ring_buffer_all, buffer, size);
}
#endif // FLEXILOG_AUTO_MALLOC
/**
 * @brief 读取所有日志
 * @param data 输出缓冲区
 * @param size 缓冲区长度
 * @return 读取长度
 */
uint32_t flog_read_all(char *data, uint32_t size)
{
    return flog_rb_read_lines(&flog.ring_buffer_all, data, size);
}
#endif // FLEXILOG_USE_ALL_LOG_RING_BUFFER
#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
#ifndef FLEXILOG_AUTO_MALLOC
void flog_set_ringbuffer_output(char *buffer, uint32_t size)
{
    flog_rb_init(&flog.ring_buffer_all, buffer, size);
}
#endif // FLEXILOG_AUTO_MALLOC
/**
 * @brief 读取输出日志
 * @param data 输出缓冲区
 * @param size 缓冲区长度
 * @return 读取长度
 */
uint32_t flog_read_output(char *data, uint32_t size)
{
    return flog_rb_read_lines(&flog.ring_buffer_output, data, size);
}
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
#ifndef FLEXILOG_AUTO_MALLOC
void flog_set_ringbuffer_recod(char *buffer, uint32_t size)
{
    flog_rb_init(&flog.ring_buffer_recod, buffer, size);
}
#endif // FLEXILOG_AUTO_MALLOC
/**
 * @brief 读取记录日志
 * @param data 输出缓冲区
 * @param size 缓冲区长度
 * @return 读取长度
 */
uint32_t flog_read_record(char *data, uint32_t size)
{
    return flog_rb_read_lines(&flog.ring_buffer_recod, data, size);
}
#endif  // FLEXILOG_USE_RECOD_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
#ifndef FLEXILOG_AUTO_MALLOC
void flog_set_ringbuffer_event(char *buffer, uint32_t size)
{
    uint32_t offset = 0;
    if (buffer)
    {
        for (int i = 0; i < FLOG_EVENT_NUM; ++i)
        {
            offset = i * size / FLOG_EVENT_NUM;
            flog.event_ring_buffer[i].event = i;
            flog_rb_init(&flog.event_ring_buffer[i].ring_bufer, buffer + offset, size / FLOG_EVENT_NUM);
        }
    }
}
#endif // FLEXILOG_AUTO_MALLOC
/**
 * @brief 读取事件日志
 * @param event  事件
 * @param data 输出缓冲区
 * @param size 缓冲区长度
 * @return 读取长度
 */
uint32_t flog_read_event(FLOG_EVENT event, char *data, uint32_t size)
{
    for (int i = 0; i < FLOG_EVENT_NUM; ++i)
    {
        if (flog.event_ring_buffer[i].event == event)
        {
            return flog_rb_read_lines(&flog.event_ring_buffer[i].ring_bufer, data, size);
        }
    }
    return 0;
}

/**
 * @brief 写入事件日志
 * @param event  事件
 * @param data 日志
 * @param size 日志长度
 */
static void flog_write_event_ring_buffer(FLOG_EVENT event, char *data, uint32_t size)
{
    for (int i = 0; i < FLOG_EVENT_NUM; ++i)
    {
        if (flog.event_ring_buffer[i].event == event)
        {
            flog_rb_write_force(&flog.event_ring_buffer[i].ring_bufer, data, size);
        }
    }
}
#endif // FLEXILOG_USE_EVENT_LOG_RING_BUFFER

/**
 * @brief 设置全局过滤等级
 * @param level 过滤等级
 */
void flog_set_global_filter(FLOG_LEVEL level)
{
    flog.global_filter_level = level;
}

#if (FLEXILOG_TAG_FILTER_NUM > 0)
/**
 * @brief 设置tag过滤等级
 * @param tag  tag
 * @param level 过滤等级
 */
void flog_set_tag_filter(const char *tag, FLOG_LEVEL level)
{
    for (int i = 0; i < FLEXILOG_TAG_FILTER_NUM; ++i)
    {
        if (flog.tag_filters[i].tag[0] == '\0')
        {
            strcpy(flog.tag_filters[i].tag, tag);
            flog.tag_filters[i].level = level;
            return;
        }
    }
}

/**
 * @brief 判断tag是否在过滤列表中
 * @param tag  tag
 * @return true     在过滤列表中
 * @return false    不在过滤列表中
 */
static bool flog_is_tag_in_filter(const char *tag)
{
    for (int i = 0; i < FLEXILOG_TAG_FILTER_NUM; ++i)
    {
        if (flog.tag_filters[i].tag[0] != '\0')
        {
            if (flog_strcmp(tag, flog.tag_filters[i].tag))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 获取tag过滤等级
 * @param tag  tag
 * @return FLOG_LEVEL  tag过滤等级
 */
static FLOG_LEVEL flog_get_tag_filter_level(const char *tag)
{
    for (int i = 0; i < FLEXILOG_TAG_FILTER_NUM; ++i)
    {
        if (flog.tag_filters[i].tag[0] != '\0')
        {
            if (flog_strcmp(tag, flog.tag_filters[i].tag))
            {
                return flog.tag_filters[i].level;
            }
        }
    }
    return FLOG_LEVEL_UNVALID;
}

#endif // (FLEXILOG_TAG_FILTER_NUM > 0)

/**
 * @brief 设置等级格式
 * @param level 等级
 * @param fmt 格式
 */
void flog_set_level_fmt(FLOG_LEVEL level, uint16_t fmt)
{
    flog.level_fmt[level] = fmt;
}

/**
 * @brief 启用格式
 * @note  可以通过或运算传入多个格式, 格式参考@ref FLOG_FMT
 * @param level 等级
 * @param fmt 需要启用的格式
 */
void flog_enable_fmt(FLOG_LEVEL level, uint16_t fmt)
{
    flog_set_level_fmt(level, fmt | flog.level_fmt[level]);
}

/**
 * @brief 禁用格式
 * @note  可以通过或运算传入多个格式, 格式参考@ref FLOG_FMT
 * @param level 等级
 * @param fmt 需要禁用的格式
 */
void flog_disable_fmt(FLOG_LEVEL level, uint16_t fmt)
{
    flog_set_level_fmt(level, flog.level_fmt[level] & (~fmt));
}

/**
 * @brief 设置字体颜色
 * @param level 等级
 * @param color 颜色
 */
void flog_set_font_color(FLOG_LEVEL level, FLOG_COLOR color)
{
    flog_enable_fmt(level, FLOG_FMT_FONT_COLOR);
    flog.font_color[level] = color;
}

/**
 * @brief 设置背景颜色
 * @param level 等级
 * @param color 颜色
 */
void flog_set_bg_color(FLOG_LEVEL level, FLOG_COLOR color)
{
    flog_enable_fmt(level, FLOG_FMT_BG_COLOR);
    flog.bg_color[level] = color;
}

/**
 * @brief 硬件输出使能
 * @param enable  是否使能
 */
void flog_hardware_output_enable(bool enable)
{
    flog.hardware_output_enable = enable;
}

/**
 * @brief 锁使能
 * @param enable  是否使能
 */
void flog_lock_enable(bool enable)
{
    flog.output_lock_enbale = enable;
}

/**
 * @brief printf
 * @param write_ring_buffer  是否写入ring buffer
 * @param fmt  格式
 * @param ...  参数
 */
void flog_printf(bool write_ring_buffer, const char *fmt, ...)
{
    uint32_t output_size = 0;
    va_list args;
    va_start(args, fmt);
    output_size = vsnprintf(flog.line_buffer, FLEXILOG_LINE_MAX_LENGTH, fmt, args);
    va_end(args);
    FLOG_LOCK();
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    if (write_ring_buffer)
        flog_rb_write_force(&flog.ring_buffer_all, flog.line_buffer, output_size);
#endif // FLEXILOG_USE_ALL_LOG_RING_BUFFER
#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    if (write_ring_buffer)
        flog_rb_write_force(&flog.ring_buffer_output, flog.line_buffer, output_size);
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_port_output(flog.line_buffer, output_size);
    FLOG_UNLOCK();
}

/**
 * @brief 输出日志
 * @param level 等级
 * @param tag  tag
 * @param file 文件名
 * @param func 函数名
 * @param line 行号
 * @param fmt  格式
 * @param ...  参数
 */
void flog_output(FLOG_LEVEL level, const char *tag, const char *file, const char *func, uint32_t line, const char *fmt, ...)
{
#ifndef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    if (!flog.hardware_output_enable)
        return;
#else
    if (flog.ring_buffer_all.buffer == NULL)
        return;
#endif // FLEXILOG_USE_ALL_LOG_RING_BUFFER
    uint32_t log_size = 0;

    /* TAG过滤器 */
#if (FLEXILOG_TAG_FILTER_NUM > 0)
    if (flog_is_tag_in_filter(tag))
    {
        if (flog.level_fmt[level] & FLOG_FMT_TAG)
        {
            FLOG_LEVEL filter_level = flog_get_tag_filter_level(tag);
            if (filter_level != FLOG_LEVEL_UNVALID && filter_level > level)
                return;
        }
    }
    else
    {
        if (level < flog.global_filter_level)
            return;
    }
#endif

    /* 添加颜色 */
    if (flog.output_color_enable && (flog.level_fmt[level] & (FLOG_FMT_FONT_COLOR | FLOG_FMT_BG_COLOR)))
    {
        log_size += flog_strcat(flog.line_buffer + log_size, FLOG_COLOR_START, FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, flog_font_color_table[flog.font_color[level]], FLEXILOG_LINE_MAX_LENGTH);
        if (flog.level_fmt[level] & FLOG_FMT_BG_COLOR && flog.bg_color[level] != FLOG_COLOR_UNVALID)
        {
            log_size += flog_strcat(flog.line_buffer + log_size, FLOG_COLOR_ADD, FLEXILOG_LINE_MAX_LENGTH);
            log_size += flog_strcat(flog.line_buffer + log_size, flog_bg_color_table[flog.bg_color[level]], FLEXILOG_LINE_MAX_LENGTH);
        }
        log_size += flog_strcat(flog.line_buffer + log_size, FLOG_COLOR_END, FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加时间 */
    if (flog.level_fmt[level] & FLOG_FMT_TIME)
    {
        log_size += flog_strcat(flog.line_buffer + log_size, "[", FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, flog_port_get_time(), FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, "]", FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加等级 */
    if (flog.level_fmt[level] & FLOG_FMT_LEVEL)
    {
        log_size += flog_strcat(flog.line_buffer + log_size, flog_level_str_table[level], FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加标签 */
    if (flog.level_fmt[level] & FLOG_FMT_TAG)
    {
        log_size += flog_strcat(flog.line_buffer + log_size, "[", FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, tag, FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, "]", FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加括号 */
    if (flog.level_fmt[level] & (FLOG_FMT_FILE | FLOG_FMT_FUNC | FLOG_FMT_LINE))
    {
        log_size += flog_strcat(flog.line_buffer + log_size, "(", FLEXILOG_LINE_MAX_LENGTH);
        /* 添加文件 */
        if (flog.level_fmt[level] & FLOG_FMT_FILE)
            log_size += flog_strcat(flog.line_buffer + log_size, file, FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加行号 */
    if (flog.level_fmt[level] & FLOG_FMT_LINE)
    {
        char line_str[6] = {0};
        log_size += flog_strcat(flog.line_buffer + log_size, ":", FLEXILOG_LINE_MAX_LENGTH);
        snprintf(line_str, sizeof(line_str), "%d", line);
        log_size += flog_strcat(flog.line_buffer + log_size, line_str, FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加函数 */
    if (flog.level_fmt[level] & FLOG_FMT_FUNC)
    {
        if (flog.level_fmt[level] & FLOG_FMT_LINE)
        {
            log_size += flog_strcat(flog.line_buffer + log_size, ",", FLEXILOG_LINE_MAX_LENGTH);
        }
        log_size += flog_strcat(flog.line_buffer + log_size, func, FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, "()", FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 括号结尾 */
    if (flog.level_fmt[level] & (FLOG_FMT_FILE | FLOG_FMT_FUNC | FLOG_FMT_LINE))
    {
        log_size += flog_strcat(flog.line_buffer + log_size, ")", FLEXILOG_LINE_MAX_LENGTH);
    }

    /* 添加线程 */
    if (flog.level_fmt[level] & FLOG_FMT_THREAD)
    {
        log_size += flog_strcat(flog.line_buffer + log_size, "(theard:", FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, flog_port_get_thread(), FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, ")", FLEXILOG_LINE_MAX_LENGTH);
    }
    log_size += flog_strcat(flog.line_buffer + log_size, ": ", FLEXILOG_LINE_MAX_LENGTH);
    /* 格式化日志 */
    va_list args;
    va_start(args, fmt);
    log_size += vsnprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, fmt, args);
    va_end(args);

    /* 重置颜色 */
    if (flog.output_color_enable && (flog.level_fmt[level] & (FLOG_FMT_FONT_COLOR | FLOG_FMT_BG_COLOR)))
    {
        log_size += flog_strcat(flog.line_buffer + log_size, FLOG_COLOR_REST, FLEXILOG_LINE_MAX_LENGTH);
    }

    log_size += flog_strcat(flog.line_buffer + log_size, FLOG_NEW_LINE, FLEXILOG_LINE_MAX_LENGTH);
    FLOG_LOCK();
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_all, flog.line_buffer, log_size);
    if (!flog.hardware_output_enable)
        return;
#endif  // FLEXILOG_USE_ALL_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_output, flog.line_buffer, log_size);
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_RECOD_LOG_RING_BUFFER
    if (level >= flog.recod_level)
    {
        flog_rb_write_force(&flog.ring_buffer_recod, flog.line_buffer, log_size);
    }
#endif  // FLEXILOG_USE_RECOD_LOG_RING_BUFFER

    flog_port_output(flog.line_buffer, log_size);
    FLOG_UNLOCK();
}

#ifdef FLEXILOG_USE_EVENT_LOG_RING_BUFFER
void flog_output_event(FLOG_EVENT event, const char *file, const char *func, uint32_t line, const char *fmt, ...)
{
    uint32_t log_size = 0;
    static char temp_str[FLEXILOG_FILE_NAME_MAX_LENGTH + FLEXILOG_FUNCTION_NAME_MAX_LENGTH + 12] = {0};
    memset(temp_str, 0, sizeof(temp_str));
    /* 时间 */
    log_size += flog_strcat(flog.line_buffer + log_size, "[", FLEXILOG_LINE_MAX_LENGTH);
    log_size += flog_strcat(flog.line_buffer + log_size, flog_port_get_time(), FLEXILOG_LINE_MAX_LENGTH);
    log_size += flog_strcat(flog.line_buffer + log_size, "]", FLEXILOG_LINE_MAX_LENGTH);

    /* 事件 */
    log_size += flog_strcat(flog.line_buffer + log_size, "[", FLEXILOG_LINE_MAX_LENGTH);
    log_size += flog_strcat(flog.line_buffer + log_size, "event:", FLEXILOG_LINE_MAX_LENGTH);
    snprintf(temp_str, sizeof(temp_str), "%d", event);
    log_size += flog_strcat(flog.line_buffer + log_size, temp_str, FLEXILOG_LINE_MAX_LENGTH);
    log_size += flog_strcat(flog.line_buffer + log_size, "]", FLEXILOG_LINE_MAX_LENGTH);

    memset(temp_str, 0, sizeof(temp_str));

    /* 函数 */
    snprintf(temp_str, sizeof(temp_str), "(%s:%d,%s()): ", file, line, func);
    log_size += flog_strcat(flog.line_buffer + log_size, temp_str, FLEXILOG_LINE_MAX_LENGTH);

    /* 格式化日志 */
    va_list args;
    va_start(args, fmt);
    log_size += vsnprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, fmt, args);
    va_end(args);

    log_size += flog_strcat(flog.line_buffer + log_size, FLOG_NEW_LINE, FLEXILOG_LINE_MAX_LENGTH);
    FLOG_LOCK();
    flog_write_event_ring_buffer(event, flog.line_buffer, log_size);
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_all, flog.line_buffer, log_size);
    if (!flog.hardware_output_enable)
        return;
#endif  // FLEXILOG_USE_ALL_LOG_RING_BUFFER

#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_output, flog.line_buffer, log_size);
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER

    flog_port_output(flog.line_buffer, log_size);
    FLOG_UNLOCK();
}
#endif // FLEXILOG_USE_EVENT_LOG_RING_BUFFER

/**
 * @brief 十六进制输出
 * @param title 标题信息
 * @param data  数据
 * @param size 数据大小
 * @param type 数据类型
 */
void flog_hex_dump(char *title, void *data, uint32_t size, FLOG_DATA_TYPE type)
{
    uint32_t log_size = 0;
    uint32_t pos = 0;
    flexlog_assert(data != NULL);
    if (title)
    {
        log_size += flog_strcat(flog.line_buffer + log_size, title, FLEXILOG_LINE_MAX_LENGTH);
        log_size += flog_strcat(flog.line_buffer + log_size, ":\r\n", FLEXILOG_LINE_MAX_LENGTH);
    }
    switch (type)
    {
        case FLOG_DATA_TYPE_BYTE:
            for (pos = 0; pos < size; pos++)
            {
                if (pos % 16 == 0)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "0x%08X ", pos);
                }
                log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "%02X ", ((uint8_t *)data)[pos]);
                if (pos % 16 == 15)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "\r\n");
                }
            }
            break;
        case FLOG_DATA_TYPE_HALF_WORD:
            if (size % 2 != 0)
                return;
            for (pos = 0; pos < size - 1; pos += 2)
            {
                if (pos % 16 == 0)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "0x%08X ", pos);
                }
                log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "%02X%02X ", ((uint8_t *)data)[pos + 1], ((uint8_t *)data)[pos]);
                if (pos % 16 == 14)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, FLOG_NEW_LINE);
                }
            }
            break;
        case FLOG_DATA_TYPE_WORD:
            if (size % 4 != 0)
                return;
            for (pos = 0; pos < size - 3; pos += 4)
            {
                if (pos % 16 == 0)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "0x%08X ", pos);
                }
                log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, "%02X%02X%02X%02X ", ((uint8_t *)data)[pos + 3], ((uint8_t *)data)[pos + 2], ((uint8_t *)data)[pos + 1], ((uint8_t *)data)[pos]);
                if (pos % 16 == 12)
                {
                    log_size += snprintf(flog.line_buffer + log_size, FLEXILOG_LINE_MAX_LENGTH, FLOG_NEW_LINE);
                }
            }
            break;
        default:
            break;
    }
    FLOG_LOCK();
#ifdef FLEXILOG_USE_ALL_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_all, flog.line_buffer, log_size);
#endif // FLEXILOG_USE_ALL_LOG_RING_BUFFER
#ifdef FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_rb_write_force(&flog.ring_buffer_output, flog.line_buffer, log_size);
#endif // FLEXILOG_USE_OUTPUT_LOG_RING_BUFFER
    flog_port_output(flog.line_buffer, log_size);
    FLOG_UNLOCK();
}

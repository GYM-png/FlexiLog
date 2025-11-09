/**
 * ==================================================
 *  @file main.c
 *  @brief 示例文件
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-09 下午7:58
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */
#include "flexi_log.h"
#include "stdio.h"
#include "string.h"
#define FLOG_TAG "example"

int main(void)
{
#ifdef FLEXILOG_AUTO_MALLOC
    flog_init();
#else
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
    /* 设置过滤等级 */
    flog_set_global_filter(FLOG_LEVEL_DEBUG); // 设置全局过滤等级
    log_printf("\r\nbefore set filter\r\n");
    logd("debug levle");
    logi("info levle");
    logw("warn levle");
    loge("error levle");
    logr("record levle");
    loga("assert levle");
    flog_set_tag_filter(FLOG_TAG, FLOG_LEVEL_WARN); // 设置当前标签过滤等级为Warn
    flog_disable_fmt(FLOG_LEVEL_RECORD, FLOG_FMT_FILE | FLOG_FMT_FUNC); // 禁用记录等级的文件名和函数名
    log_printf("after set filter warn\r\n");
    logd("debug levle");
    logi("info levle");
    logw("warn levle");
    loge("error levle");
    logr("record levle");
    loga("assert levle");
    log_event(FLOG_EVENT_0, "event 0 log tigger");
    log_event(FLOG_EVENT_1, "event 1 log tigger");
    /* 设置过滤等级 */
    
    
    /* hex dump */
    uint8_t hex_data[] = {0x48, 0x04, 0x01, 0x20, 0x95, 0x82, 0x00, 0x08, 0x2D, 0xCA, 0x00, 0x08, 0xB9, 0x82, 0x00, 0x08, 0x48, 0x04, 0x01, 0x20, 0x95, 0x82, 0x00, 0x08, 0x2D, 0xCA, 0x00, 0x08, 0xB9, 0x82, 0x00, 0x08};
    flog_hex_dump("hex dump:", hex_data, sizeof(hex_data), FLOG_DATA_TYPE_WORD);
    /* hex dump */

    /* 读取日志并打印 */
    char log_read[2048];
    uint32_t read_size = flog_read_record(log_read, 2048);
    printf("read_size : %d\r\n", read_size);
    printf("%s\r\n", log_read); // 打印读取的日志

    /* 读取事件日志并打印 */
    printf("\r\nread event log\r\n");
    memset(log_read, 0, 2048);
    read_size = flog_read_event(FLOG_EVENT_0, log_read, 2048);
    printf("event 0 read_size : %d\r\n", read_size);
    printf("%s\r\n", log_read);

    memset(log_read, 0, 2048);
    read_size = flog_read_event(FLOG_EVENT_1, log_read, 2048);
    printf("event 1 read_size : %d\r\n", read_size);
    printf("%s", log_read);


    return 0;
}
/**
 * ==================================================
 *  @file flexi_log_rb.h
 *  @brief flexi log 环形缓冲区
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-06 下午11:11
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */


#ifndef FLEXILOG_FLEXI_LOG_RB_H
#define FLEXILOG_FLEXI_LOG_RB_H

#include "flexi_log.h"
#ifdef FLEXILOG_USE_RING_BUFFER
#include "stdint.h"

/* 环形缓冲区 */
typedef struct
{
    char *buffer;
    uint32_t size;
    uint32_t read_pos : 31;
    uint32_t read_pos_mirror : 1;
    uint32_t write_pos : 31;
    uint32_t write_pos_mirror : 1;
}flog_ring_buffer_t;

void flog_rb_init(flog_ring_buffer_t *rb, char *buffer, uint32_t size);
#ifdef FLEXILOG_AUTO_MALLOC
void flog_rb_buffer_create(flog_ring_buffer_t *rb, uint32_t size);
#endif //FLEXILOG_AUTO_MALLOC
uint32_t flog_rb_read_lines(flog_ring_buffer_t *rb, char *data, uint32_t size);
void flog_rb_write_force(flog_ring_buffer_t *rb, const char *data, uint32_t size);
#endif
#endif //FLEXILOG_FLEXI_LOG_RB_H

/**
 * ==================================================
 *  @file flexi_log_rb.c
 *  @brief TODO 描述该文件的功能
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-06 下午11:11
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */


#include "flexi_log_rb.h"
#include "flexi_log.h"
#include "stdbool.h"
#include "string.h"
#include "stdint.h"


void flog_rb_init(flog_ring_buffer_t *rb, char *buffer, uint32_t size)
{
    flexlog_assert(rb);
    flexlog_assert(rb->buffer);
    flexlog_assert(buffer);
    memset(buffer, 0, size);
    rb->buffer = buffer;
    rb->size = size;
    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->read_pos_mirror = 0;
    rb->write_pos_mirror = 0;
}

#ifdef FLEXILOG_AUTO_MALLOC
extern void *flog_port_malloc(size_t size);
extern void flog_port_free(void *ptr);
/**
 * @brief 创建一个缓冲区
 * @param rb 环形缓冲区
 * @param size 缓冲区大小
 */
void flog_rb_buffer_create(flog_ring_buffer_t *rb, uint32_t size)
{
    flexlog_assert(rb != NULL)
    rb->buffer = flog_port_malloc(size);
    if (rb->buffer != NULL)
    {
        memset(rb->buffer, 0, size);
        rb->size = size;
        rb->read_pos = 0;
        rb->write_pos = 0;
        rb->read_pos_mirror = 0;
        rb->write_pos_mirror = 0;
    }
}
#endif //FLEXILOG_AUTO_MALLOC
static bool flog_rb_is_empty(flog_ring_buffer_t *rb)
{
    return (rb->read_pos == rb->write_pos && rb->read_pos_mirror == rb->write_pos_mirror);
}

static bool flog_rb_is_full(flog_ring_buffer_t *rb)
{
    return (rb->read_pos == rb->write_pos && rb->read_pos_mirror != rb->write_pos_mirror);
}

uint32_t flog_rb_read(flog_ring_buffer_t *rb, char *data, uint32_t size)
{
    if (flog_rb_is_empty(rb))
    {
        return 0;
    }

    for (uint32_t i = 0; i < size; ++i)
    {
        if(flog_rb_is_empty(rb))
        {
            return i;
        }
        else
        {
            data[i] = rb->buffer[rb->read_pos];
            rb->read_pos = (rb->read_pos + 1) % rb->size;
            if (rb->read_pos == rb->write_pos)
            {
                rb->read_pos_mirror = !rb->read_pos_mirror;
            }
        }
    }
    return size;
}

/**
 * @brief 读取整行数据
 * @param rb 环形缓冲区
 * @param data 数据缓冲区
 * @param size 数据缓冲区大小
 * @return 读取的字节大小
 */
uint32_t flog_rb_read_lines(flog_ring_buffer_t *rb, char *data, uint32_t size)
{
    flexlog_assert(rb)
    flexlog_assert(rb->buffer)
    flexlog_assert(data);
    if (flog_rb_is_empty(rb))
        return 0;
    const uint32_t write_pos_cur = rb->write_pos;//写入指针 做判断
    const uint32_t rb_size = rb->size;          //缓冲区大小
    uint32_t try_read_pos = rb->read_pos;      //临时读取指针
    uint32_t read_szie = 0;     //读取大小
    uint32_t try_read_size = 0; //尝试读取大小
    char rChar = 0;             //读取字符
    while (1)
    {
        /*读取字符*/
        rChar = rb->buffer[try_read_pos];
        try_read_size++;
        try_read_pos = (try_read_pos + 1) % rb_size;

        /*判断是否超大小*/
        if(try_read_size >= size)
        {
            break;
        }

        /*缓冲区是否读取完毕*/
        if (try_read_pos == write_pos_cur)
        {
            read_szie = try_read_size;          //进这里说明缓冲区已经读取完最后一行并且没有超大小
            break;
        }

        if (rChar == '\n')//完整读取一行并且没有超大小再才能进行读取
        {
            read_szie = try_read_size;
        }
    }
    read_szie = flog_rb_read(rb, data, read_szie);
    return read_szie;
}


void flog_rb_write_force(flog_ring_buffer_t *rb, const char *data, uint32_t size)
{
    flexlog_assert(rb);
    flexlog_assert(rb->buffer);
    flexlog_assert(data);
    for (uint32_t i = 0; i < size; ++i)
    {
        if (flog_rb_is_full(rb))
        {
            rb->read_pos = (rb->read_pos + 1) % rb->size;
            rb->buffer[rb->write_pos] = data[i];
            rb->write_pos = (rb->write_pos + 1) % rb->size;
        }
        else
        {
            rb->buffer[rb->write_pos] = data[i];
            rb->write_pos = (rb->write_pos + 1) % rb->size;
        }
        if (rb->read_pos == rb->write_pos)
        {
            rb->write_pos_mirror = !rb->write_pos_mirror;
        }
    }
}




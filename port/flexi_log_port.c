/**
 * ==================================================
 *  @file flexi_log_port.c
 *  @brief flexi log 外部接口
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-06 下午9:49
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */

#include "flexi_log.h"

/* your library */
#include "stdlib.h"

/**
 * @brief 硬件外设初始化
 */
void flog_port_init(void)
{
    /* TODO: 添加初始化代码 */
}

/**
 * @brief 硬件外设输出
 * @param buf 输出数据
 * @param size 输出数据长度
 */
void flog_port_output(const char *buf, size_t size)
{
    /* TODO: 添加写入代码 */
}

/**
 * @brief 加锁
 */
void flog_port_lock(void)
{
    /* TODO: 添加锁代码 */
}

/**
 * @brief 解锁
 */
void flog_port_unlock(void)
{
    /* TODO: 添加解锁代码 */
}

/**
 * @brief 获取时间
 */
const char *flog_port_get_time(void)
{
    /* TODO: 添加时间代码 */
    return "";
}

/**
 * @brief 获取线程ID
 */
const char *flog_port_get_thread(void)
{
    /* TODO: 添加线程ID代码 */
    return "";
}

#ifdef FLEXILOG_AUTO_MALLOC
/**
 * @brief 内存分配
 */
void *flog_port_malloc(size_t size)
{
    /* TODO: 添加内存分配代码 */
    return malloc(size);
}

/**
 * @brief 内存释放
 */
void flog_port_free(void *ptr)
{
    /* TODO: 添加内存释放代码 */
    free(ptr);
}
#endif

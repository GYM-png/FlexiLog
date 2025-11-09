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
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
static HANDLE hSerial = INVALID_HANDLE_VALUE;
static int is_initialized = 0;

/**
 * @brief 硬件外设初始化
 */
void flog_port_init(void)
{
    /* TODO: 添加初始化代码 */

    DCB dcb = {0};
    COMMTIMEOUTS timeouts = {0};

    if (is_initialized) {
        return; // 已初始化
    }

    // 1. 打开串口
    hSerial = CreateFileA("COM2",
                          GENERIC_READ | GENERIC_WRITE,
                          0, NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[Serial] open %s filed，errorcode: %lu\n", "COM2", GetLastError());
        return;
    }

    // 2. 设置缓冲区
    SetupComm(hSerial, 4096, 4096);

    // 3. 获取并配置 DCB
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial, &dcb)) {
        fprintf(stderr, "[Serial] GetCommState file\n");
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return;
    }

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcb)) {
        fprintf(stderr, "[Serial] SetCommState fail，errorcode: %lu\n", GetLastError());
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return;
    }

    // 4. 设置写超时
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(hSerial, &timeouts);

    is_initialized = 1;
    printf("[Serial] %s init success (115200 8N1)\n", "COM2");
}

/**
 * @brief 硬件外设输出
 * @param buf 输出数据
 * @param size 输出数据长度
 */
void flog_port_output(const char *buf, size_t size)
{
    /* TODO: 添加写入代码 */
    DWORD written = 0;

    if (!is_initialized || hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[Serial] 串口未初始化\n");
        return ;
    }

    if (!WriteFile(hSerial, buf, (DWORD)size, &written, NULL)) {
        fprintf(stderr, "[Serial] WriteFile 失败，错误码: %lu\n", GetLastError());
        return;
    }
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
    /* return ""; */
    static char time_str[] = "2025-11-06 17:05:05.000";
    return time_str;
}

/**
 * @brief 获取线程ID
 */
const char *flog_port_get_thread(void)
{
    /* TODO: 添加线程ID代码 */
    /* return ""; */

    static char thread_id_str[] = "0x00000000";
    return thread_id_str;
}

#ifdef FLEXILOG_AUTO_MALLOC
/**
 * @brief 内存分配
 */
void *flog_port_malloc(size_t size)
{
    return malloc(size);
}

/**
 * @brief 内存释放
 */
void flog_port_free(void *ptr)
{
    free(ptr);
}
#endif

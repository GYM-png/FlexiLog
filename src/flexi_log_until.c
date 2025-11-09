/**
 * ==================================================
 *  @file flexi_log_until.c
 *  @brief flexi log功能性函数实现文件
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-07 下午9:44
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */
#include "flexi_log_until.h"
#include "flexi_log.h"

/**
 * @brief 字符串拼接
 * @param dest 源字符串
 * @param src 拼接字符串
 * @param max_size 最大长度
 * @return 拼接的字符串长度
 */
uint32_t flog_strcat(char *dest, const char *src, uint32_t max_size)
{
    flexlog_assert(dest != NULL)
    flexlog_assert(src != NULL)
    uint32_t pos = 0;
    while (pos < max_size && *src)
    {
        dest[pos++] = *src++;
    }
    return pos;
}

/**
 * @brief 计算字符串长度
 * @param str 字符串
 * @return 字符串长度
 */
uint32_t flog_strlen(const char *str)
{
    flexlog_assert(str != NULL)
    uint32_t len = 0;
    while (*str++)
    {
        len++;
    }
    return len;
}

/**
 * @brief 字符串比较
 * @param str1 字符串1
 * @param str2 字符串2
 * @return true 相等
 * @return false 不相等
 */
bool flog_strcmp(const char *str1, const char *str2)
{
    flexlog_assert(str1 != NULL)
    flexlog_assert(str2 != NULL)
    if (flog_strlen(str1) != flog_strlen(str2))
        return false;

    while (*str1 && *str2)
    {
        if (*str1++ != *str2++)
        {
            return false;
        }
    }
    return true;
}

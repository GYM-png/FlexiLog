/**
 * ==================================================
 *  @file flexi_log_until.h
 *  @brief TODO 描述该文件的功能
 *  @author GYM (48060945@qq.com)
 *  @date 2025-11-07 下午9:44
 *  @version 1.0
 *  @copyright Copyright (c) 2025 GYM. All Rights Reserved.
 * ==================================================
 */


#ifndef FLEXILOG_FLEXI_LOG_UNTIL_H
#define FLEXILOG_FLEXI_LOG_UNTIL_H

#include "stdint.h"
#include "stdbool.h"

uint32_t flog_strcat(char *dest, const char *src, uint32_t max_size);
uint32_t flog_strlen(const char *str);
bool flog_strcmp(const char *str1, const char *str2);

#endif //FLEXILOG_FLEXI_LOG_UNTIL_H

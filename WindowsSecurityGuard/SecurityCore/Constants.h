#pragma once

// 通用宏定义头文件，请勿在其中添加除宏定义之外的内容

// 字符串长度定义
#define LENGTH_MINUS_ONE               -1
#define LENGTH_ZERO                     0
#define LENGTH_WINDOWS_ROOT_DIR         3

// 架构值
#define ARCHITECTURE_32  32
#define ARCHITECTURE_64  32

// 正确返回值
#define ERROR_SUCCESS                1

// WINDOWS API 错误返回值
#define ERROR_GET_FULL_PATH_NAME     10000   // GetFullPathName函数报错
#define ERROR_OPEN_PROCESS           10001   // OpenProcess函数报错
#define ERROR_IS_WOW64_PROCESS       10002   // IsWow64Process函数报错
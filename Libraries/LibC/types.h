#ifndef TYPES_H
#define TYPES_H

#define NULL 0
#define UINT32_MAX 0xFFFFFFFF
#define INT_MAX 2147483647
#define INT_MIN -2147483648
#define UINT_MAX (INT_MAX * 2U + 1U)

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef unsigned int uintptr_t;

typedef long unsigned int size_t;

#ifndef __cplusplus
typedef char bool;
#endif

#endif

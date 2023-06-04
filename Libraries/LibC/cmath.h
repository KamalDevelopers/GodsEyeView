#ifndef CMATH_H
#define CMATH_H

#define M_E 2.71828182845904523536
#define M_LOG2E 1.44269504088896340736
#define M_LOG10E 0.434294481903251827651
#define M_LN2 0.693147180559945309417
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.785398163397448309616
#define M_1_PI 0.318309886183790671538
#define M_2_PI 0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.707106781186547524401

#ifdef __cplusplus
extern "C" {
#endif

inline double pow(double x, double y)
{
    int temp = x;
    for (int i = 0; i < y - 1; i++)
        temp = temp * x;
    return temp;
}

inline double sqrt(double x)
{
    double result;
    asm("fsqrt"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline double floor(double x)
{
    return (double)(x <= x ? x : x - 1);
}

inline double ceil(double x)
{
    return (double)(x < x ? x + 1 : x);
}

inline double fabs(double x)
{
    return x >= 0 ? x : -1 * x;
}

inline double modf(double x, int* integer)
{
    *integer = x;
    return x - *integer;
}

inline double sin(double x)
{
    double result;
    asm("fsin"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline int abs(int num)
{
    if (num < 0)
        num = (-1) * num;
    return num;
}

inline double truncate(double x)
{
    return x < 0 ? -floor(-x) : floor(x);
}

inline double fmod(double x, double y)
{
    return x - truncate(x / y) * y;
}

#ifdef __cplusplus
}
#endif

#endif

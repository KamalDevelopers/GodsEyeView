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

inline double pow(double x, int y)
{
    double temp;
    if (y == 0)
        return 1;
    temp = pow(x, y / 2);
    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0) return x * temp * temp;
        else return (temp * temp) / x;
    }
}

inline double sqrt(double x)
{
    double result;
    asm("fsqrt"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline float sqrtf(float x)
{
    double y = (double)x;
    return (float)sqrt(y);
}

inline double floor(double x)
{
    if (x < 0)
        return (int)x - 1;
    else
        return (int)x;
}

inline float floorf(float x)
{
    return (float)floor((double)x);
}

inline double ceil(double x)
{
    float f = (float)x;
    unsigned input;
    ((char*)&input)[0] = ((char*)&f)[0];
    ((char*)&input)[1] = ((char*)&f)[1];
    ((char*)&input)[2] = ((char*)&f)[2];
    ((char*)&input)[3] = ((char*)&f)[3];

    int exponent = ((input >> 23) & 255) - 127;
    if (exponent < 0) return (f > 0);

    int fractional_bits = 23 - exponent;
    if (fractional_bits <= 0) return f;

    unsigned integral_mask = 0xffffffff << fractional_bits;
    unsigned output = input & integral_mask;

    ((char*)&f)[0] = ((char*)&output)[0];
    ((char*)&f)[1] = ((char*)&output)[1];
    ((char*)&f)[2] = ((char*)&output)[2];
    ((char*)&f)[3] = ((char*)&output)[3];
    if (f > 0 && output != input) ++f;

    return f;
}

inline float ceilf(float x)
{
    return (float)ceil((double)x);
}

inline double fabs(double x)
{
    return x >= 0 ? x : -1 * x;
}

inline float fabsf(float x)
{
    return (float)fabs((float)x);
}

inline double modf(double x, int* integer)
{
    *integer = x;
    return x - *integer;
}

inline float cosf(float x)
{
    double result;
    asm("fcos"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline double tan(double x)
{
    double result;
    asm("fptan"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline float tanf(float x)
{
    double y = (double)x;
    return (float)tan(y);
}

inline double sin(double x)
{
    double result;
    asm("fsin"
        : "=t"(result)
        : "0"(x));
    return result;
}

inline float sinf(float x)
{
    double y = (double)x;
    return (float)sin(y);
}

inline int abs(int num)
{
    if (num < 0)
        num = (-1) * num;
    return num;
}

inline float acosf(float x)
{
    float nate = (float)(x < 0);
    if (x < 0) x = (-1) * x;
    float ret = -0.0187293;
    ret = ret * x;
    ret = ret + 0.0742610;
    ret = ret * x;
    ret = ret - 0.2121144;
    ret = ret * x;
    ret = ret + 1.5707288;
    ret = ret * sqrt(1.0 - x);
    ret = ret - 2 * nate * ret;
    return nate * 3.14159265358979 + ret;
}

inline float atan2f(float y, float x)
{
    const float piby2 =  1.5707963f;
    const float pi = 3.14159265f;
	if (x == 0.0f) {
		if (y > 0.0f) return piby2;
		if (y == 0.0f) return 0.0f;
		return -piby2;
	}

	float atan;
	float z = y/x;
	if (fabs(z) < 1.0f) {
		atan = z / (1.0f + 0.28f * z * z);
		if (x < 0.0f) {
            if (y < 0.0f)
                return atan - pi;
            return atan + pi;
		}
	}
	else {
		atan = piby2 - z / (z * z + 0.28f);
		if (y < 0.0f)
            return atan - pi;
	}
	return atan;
}

inline double truncate(double x)
{
    return x < 0 ? -floor(-x) : floor(x);
}

inline double fmod(double x, double y)
{
    return x - truncate(x / y) * y;
}

inline float fmodf(float x, float y)
{
    return (float)fmod((float)x, (float)y);
}

inline double round(double arg)
{
    if (arg < 0.0)
        return ceil((float)(arg - 0.5));
    else
        return floor((double)(arg + 0.5));
}

inline float roundf(float arg)
{
    double d = (double)arg;
    return round(d);
}

#ifdef __cplusplus
}
#endif

#endif

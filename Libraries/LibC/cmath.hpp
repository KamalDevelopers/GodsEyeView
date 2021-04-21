#ifndef CMATH_HPP
#define CMATH_HPP

#if defined(_USE_MATH_DEFINES) && !defined(_MATH_DEFINES_DEFINED)
#    define _MATH_DEFINES_DEFINED
#    define M_E 2.71828182845904523536
#    define M_LOG2E 1.44269504088896340736
#    define M_LOG10E 0.434294481903251827651
#    define M_LN2 0.693147180559945309417
#    define M_LN10 2.30258509299404568402
#    define M_PI 3.14159265358979323846
#    define M_PI_2 1.57079632679489661923
#    define M_PI_4 0.785398163397448309616
#    define M_1_PI 0.318309886183790671538
#    define M_2_PI 0.636619772367581343076
#    define M_2_SQRTPI 1.12837916709551257390
#    define M_SQRT2 1.41421356237309504880
#    define M_SQRT1_2 0.707106781186547524401
#endif

/*Not a C function, but still usefull*/
//template <int N> struct Factorial { enum { value = N * Factorial<N - 1>::value }; };
//template <> struct Factorial<0> { enum { value = 1 }; };

static double pow(double x, double y)
{
    int temp = x;
    for (int i = 0; i < y - 1; i++)
        temp = temp * x;
    return temp;
}

static int sqrt(int x)
{
    int z = 1;
    while (z != 10000) {
        if (x / z == z) {
            break;
        }
        z += 1;
    }
    return z;
}

static double floor(double x)
{
    return double(int(x) <= x ? int(x) : int(x) - 1);
}

static double ceil(double x)
{
    return double(int(x) < x ? int(x) + 1 : int(x));
}

static double fabs(double x)
{
    return x >= 0 ? x : -1 * x;
}

static double modf(double x, int* integer)
{
    *integer = x;
    return x - *integer;
}

static double sin(double x)
{
    double result;
    asm("fsin"
        : "=t"(result)
        : "0"(x));
    return result;
}

#endif
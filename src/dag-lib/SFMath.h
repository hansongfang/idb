#pragma once

#include <math.h>
#include <limits>
#include <string>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char ubyte;

#define ZCHECK(x)
#define ZRCHECK_LT(x, y) 
#define ZCHECK_NE(x, y)
#define ZCHECK_EQ(x, y)

using namespace std;

//Constant variables
const double EPS=1e-7;
const double EPSILON5=1e-5;
const double EPSILON6=1e-6;
const double EPSILON7=1e-7;
const double EPSILON8=1e-8;
const double EPSILON9=1e-9;
const double EPSILON=EPSILON7;
const double SMALL_EPSILON=EPSILON8;
const double BIG_EPSILON=EPSILON6;
const double TINY_EPSILON=EPSILON9;
const double HUGE_EPSILON=EPSILON5;
const double C_EPS=1e-7;
const double C_E=2.7182818284590452354;
const double C_LOG2E=1.4426950408889634074;
const double C_LOG10E=0.43429448190325182765;
const double C_LN2=0.69314718055994530942;
const double C_LN10=2.30258509299404568402;

const double PI=3.1415926535897932384626433832795028841971693993751058209749445;
const double C_PI=PI;
const double C_2PI=PI*2.0;
const double C_PI_2=C_PI/2.0;
const double C_PI_4=C_PI/4.0;
const double C_1_PI=1.0/C_PI;
const double C_2_PI=2.0/C_PI;
const double C_1_2PI=0.5/C_PI;
const double C_PI_180=C_PI/180.0;
const double C_180_PI=180.0/C_PI;
const double C_D2R=C_PI_180;
const double C_R2D=C_180_PI;

const double C_SQRTPI=sqrt(C_PI);
const double C_2_SQRTPI=2.0/C_SQRTPI;
const double C_SQRT2=1.4142135623730950488016887242097;
const double C_SQRT3=1.73205080756888;

const double C_SQRT2_2=C_SQRT2/2.0;
const double C_LNPI=1.14472988584940017414;
const double C_EULER=0.57721566490153286061;

#undef min
#undef max

const char MIN_CHAR=numeric_limits<char>::min();
const char MAX_CHAR=numeric_limits<char>::max();
const uchar MIN_UCHAR=numeric_limits<uchar>::min();
const uchar MAX_UCHAR=numeric_limits<uchar>::max();
const short MIN_SHORT=numeric_limits<short>::min();
const short MAX_SHORT=numeric_limits<short>::max();
const ushort MIN_USHORT=numeric_limits<ushort>::min();
const ushort MAX_USHORT=numeric_limits<ushort>::max();
const int MIN_INT=numeric_limits<int>::min();
const int MAX_INT=numeric_limits<int>::max();
const uint MIN_UINT=numeric_limits<uint>::min();
const uint MAX_UINT=numeric_limits<uint>::max();
const long MIN_LONG=numeric_limits<long>::min();
const long MAX_LONG=numeric_limits<long>::max();
const ulong MIN_ULONG=numeric_limits<ulong>::min();
const ulong MAX_ULONG=numeric_limits<ulong>::max();
const float MIN_FLOAT=numeric_limits<float>::min();
const float MAX_FLOAT=numeric_limits<float>::max();
const double MIN_DOUBLE=numeric_limits<double>::min();
const double MAX_DOUBLE=numeric_limits<double>::max();

//////////////////////////////////////////////////////////////////////////
template <typename T> inline const T& Min(const T &a, const T &b) {
    return (a<b)?a:b;
}

template <typename T> inline const T& Max(const T &a, const T &b) {
    return (a>b)?a:b;
}

template <typename T> inline T& Min(T& a, T& b) {
    return (a<b)?a:b;
}

template <typename T> inline T& Max(T& a, T& b) {
    return (a>b)?a:b;
}

//ATTENTION TO PARAMETER ORDER: A X B
template <typename T> inline const T& Clamp(const T &a, const T &x, const T &b) {
    if (x<a) return a;
    return (x>b)?b:x;
}

//ATTENTION TO PARAMETER ORDER: A X B
template <typename T> inline bool Within(const T &a, const T &x, const T &b) {
    return (x>=a && x<=b);
}

template <typename T> inline T Sign(const T &x) {
    return (x>=T(0))?T(1):T(-1);
}

//ATTENTION TO PARAMETER ORDER: T0 T1 A
template <typename T> inline T Lerp(const T &t0, const T &t1, const float a) {
    return t0*(1.0f-a)+t1*a;
}

template <typename T> inline T Lerp(const T &t0, const T &t1, const double a) {
    return t0*(1.0f-a)+t1*a;
}

template <typename T>
inline int Round( const T &x ) {
    if (x>=0)
        return (int)(x+T(0.5));
    else
        return (int)(x-T(0.5));
}

inline double Round(const double value, const int precision) {
    if (precision>=0) {
        const int p = Clamp(precision,-15,15);
        // Lookup table for pwr(10.0,i)
        const static double pwr[] = {
            1e0,  1e1,  1e2,  1e3,  1e4,
            1e5,  1e6,  1e7,  1e8,  1e9,
            1e10, 1e11, 1e12, 1e13, 1e14 };

            // Table of inverses
            const static double invpwr[] = {
                1e0,   1e-1,  1e-2,  1e-3,  1e-4,
                1e-5,  1e-6,  1e-7,  1e-8,  1e-9,
                1e-10, 1e-11, 1e-12, 1e-13, 1e-14 };
                double val = value;
                if (value<0.0) val = ceil(val*pwr[p]-0.5);
                if (value>0.0) val = floor(val*pwr[p]+0.5);
                return val*invpwr[p];
    }
    return value;
}

inline float Round(const float value, const int precision) {
    return (float) Round(double(value),precision);
}

template<int M,int N>
struct StaticPow {
    enum{RESULT = M * StaticPow<M,N-1>::RESULT};
};
template<int M>
struct StaticPow<M,0> {
    enum{RESULT = 1};
};

inline int FlowToPower2(int w) {
    return  1 << (int)floor((log((double)w)/log(2.0f)) + 0.5f);
}

inline int Log2(int x) {
    int i = 0;
    while(x = x >> 1) ++i;
    return i;
}

inline float Log2(float x) {
    return static_cast<float>(::log(x)/C_LN2);
}

inline double Log2(double x) {
    return ::log(x)/C_LN2;
}

inline int Factorial(int n) {
    int ret=1;
    for (int i=2;i<=n;i++) ret*=i;
    return ret;
}

template<typename T1, typename T2>
double Pow(const T1 x, const T2 y) {
    return pow(static_cast<double>(x),static_cast<double>(y));
}

template<typename T>
T Pow(const T &x, const int y) {
    T ret=1;
    for (int i=0;i<y;i++) ret*=x;
    return ret;
}

inline double WrapPI(double theta) {
    theta += PI;
    theta -= floor(theta * C_1_2PI) * C_2PI;;
    theta -= PI;
    return theta;
}

inline double SafeACos(double x) {
    // Check limit conditions
    if (x <= -1.0f) return PI;
    if (x >= 1.0f) return 0.0;
    // Value is in the domain - use standard C function
    return acos(x);
}

template <typename T> 
inline T Sqr(const T &x) {
    return x*x;
}
template <typename T> 
inline T Cube(const T &x) {
    return x*x*x;
}
template <typename T> 
inline T Quart(const T &x) {
    return x*x*x*x;
}
template <typename T> 
inline T Quint(const T &x) {
    return x*x*x*x*x;
}

template <typename T> inline void Swap(T &x1, T &x2) {
    T tmp=x1; x1=x2; x2=tmp;
}

template <typename T> inline T Abs(const T x) {
    return std::abs(x);
}

template <typename T> inline void KeepAbs(T& x) {
    x=std::abs(x);
}

template <typename T> inline T Sqrt(const T x) {
    return static_cast<T>(std::sqrt(static_cast<double>(x)));
}

inline uchar FromHex(const char ch) {
    if (ch>='0' && ch<='9') return ch-'0';
    if (ch>='a' && ch<='f') return ch-'a'+10;
    if (ch>='A' && ch<='F') return ch-'A'+10;
    ZCHECK((ch>='0' && ch<='9') || (ch>='a' && ch<='f') || (ch>='A' && ch<='F'));
    return 0;
}

inline int FromHex(const string &a) {
    int ret=0;
    for (size_t i=0;i<a.size();i++)
        ret=(ret<<4)+FromHex(a[i]);
    return ret;
}

inline char ToHex(const uchar val) {
    const char table[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    // Check sanity of input
    ZCHECK(/*val>=0 && */val<=15);
    return table[val&15];
}

inline char ToHex(const char val) {
    return ToHex(uchar(val));
}

// Determine highest n, given 2^n <= x
template<typename T> T LowerPowerOf2(const T x) {
    uint tmp = 1<<(sizeof(T)*8-1);
    while ((tmp&x)==0 && tmp) tmp>>=1;
    return tmp;
}

//Determine lowest n, given 2^n >= x
template<typename T>T UpperPowerOf2(const T x) {
    uint tmp = 1<<(sizeof(T)*8-1);
    while ((tmp&x)==0 && tmp) tmp>>=1;
    return x==tmp ? tmp : tmp<<1;
}

//Test if two numbers have same sign
template <typename T>bool IsSameSign(const T &a, const T &b) {
    return (a<0 && b<0) || (a>0 && b>0);
}

// Convert an integer to a float between 0 and 1
// IEEE 4 byte real
// 
// 31 30    23 22                0
// |-------------------------------------.
// |s | 8 bits |msb   23 bit mantissa  lsb|
// `-------------------------------------'
// |  |        `----------------  mantissa
// |  `--------------------------------  biased exponent (7fh)
// `-------------------------------------  sign bit
inline void Fraction(float &frac,const uint bits) {
    ZCHECK(sizeof(float)==4 && sizeof(float)==sizeof(uint));
    union { float f; uint i; } tmp;
    // Place fractional bits in mantissa
    tmp.i = (0x7f<<23) | (bits>>9);
    // Adjust to [0,1) range
    frac = tmp.f-1.0f;
}

//Convert an integer to a double between 0 and 1
// IEEE 8 byte real
// 63 62      52 51                                 0
// |-------------------------------------------------.
// |s |11 bits| msb    52 bit mantissa            lsb|
// `-------------------------------------------------'
// |  |        `----------------  mantissa
// |  `--------------------------------  biased exponent (3FFh)
// `-------------------------------------  sign bit
inline void Fraction(double &frac,const uint bits) {
    ZCHECK(sizeof(double)==8 && sizeof(double)==sizeof(uint)*2);
    union { double f; uint i[2]; } tmp;
    // Place fractional bits in mantissa
    tmp.i[1] = (0x3FF<<20) | (bits>>12);
    tmp.i[0] = (bits<<12);
    // Adjust to [0,1) range
    frac = tmp.f-1.0;
}

//magic square root in Quake III
// 1.0/sqrt(number)
inline float MagicInvSqrt( float number ) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;  // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 ); // what the fuck?
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
    // y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

    return y;
}

// Another implementation, better.
inline float MagicInvSqrt2(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x; // get bits for floating value
    i = 0x5f375a86- (i>>1); // gives initial guess y0
    x = *(float*)&i; // convert bits back to float
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    return x;
}


inline float MagicSqrt(float number) {
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}

inline bool IsInf(double x) {
    return x==numeric_limits<double>::infinity() || x==-numeric_limits<double>::infinity();
}

inline bool IsInf(float x) {
    return x==numeric_limits<float>::infinity() || x==-numeric_limits<float>::infinity();
}

template< typename T >
inline bool IsNaN( const T &r ) {
    return r != r;
}

inline bool IsBad(double x) {
    return IsInf(x) || IsNaN(x);
}

inline bool IsBad(float x) {
    return IsInf(x) || IsNaN(x);
}
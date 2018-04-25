#pragma once
#include "SFMath.h"

// Misc
template <typename InputIterator1, typename InputIterator2>
bool SafeEqual(const InputIterator1 &first1, const InputIterator1 &last1,
               const InputIterator2 &first2, const InputIterator2 &last2)
{
    InputIterator1 cur1 = first1;
    InputIterator2 cur2 = first2;
    while ( (cur1 != last1) ) {
        if (cur2 == last2)
            return false;
        if (*cur1 != *cur2)   // or: if (!pred(*first1,*first2)), for pred version
            return false;
        cur1++; cur2++;
    }
    return true;
}

template<typename T>
class IOObject
{
public:
    static void CopyData(T* dst, const T* src, uint size) {
        for (uint i=0;i<size;i++) dst[i]=src[i];
    }
};

//////////////////////////////////////////////////////////////////////////
// Class VectorBase
//////////////////////////////////////////////////////////////////////////

template <unsigned int N, typename T>
class VectorBase
{
protected:
    T v[N];

public:
    // STL support
    // type definitions
    typedef T               value_type;
    typedef T*              iterator;
    typedef const T*        const_iterator;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef uint           size_type;
    typedef uint           difference_type;

    // iterator support
    iterator begin() { return v; }
    const_iterator begin() const { return v; }
    iterator end() { return v+N; }
    const_iterator end() const { return v+N; }

    // reverse iterator support
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end());}
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const {return const_reverse_iterator(begin());}

    // at()
    reference at(size_type i) {
        ZRCHECK_LT(i, N);
        return v[i]; 
    }
    const_reference at(size_type i) const {
        ZRCHECK_LT(i, N);
        return v[i]; 
    }

    // front() and back()
    reference front() { return v[0]; }
    const_reference front() const {  return v[0];}
    reference back() {   return v[N-1]; }
    const_reference back() const {   return v[N-1]; }

    // size is constant
    static size_type size() { return N; }
    static bool empty() { return false; }
    static size_type max_size() { return N; }
    enum { static_size = N };

    // swap (note: linear complexity)
    void swap (VectorBase<N,T>& y) {  std::swap_ranges(begin(),end(),y.begin());}

    // check range (may be private because it is static)
    bool CheckIndex (size_type i) { return (i>=0 && i<size());}

    //subscript
    inline T& operator [](int a) {return at(a);}
    inline const T& operator [](int a) const {return at(a);}

    //copy
    inline const VectorBase<N,T> &operator=(const T &a){for (unsigned int i=0;i<N;i++) v[i]=a; return *this;}
    inline const VectorBase<N,T> &operator=(const VectorBase<N,T>& a){IOObject<T>::CopyData(v,a.v,N); return *this;}
    template <typename T1>
    inline const VectorBase<N,T> &operator=(const VectorBase<N,T1>& a){for (unsigned int i=0;i<N;i++) v[i]=(T)a[i]; return *this;}

    //raw data access
    inline T* Data(){return v;}
    inline const T* Data() const {return v;}

    //rawcopy
    inline void RawCopy(const VectorBase<N,T>& a){memcpy(v,a.v,sizeof(T)*N);}
    inline void RawCopy(const T *a){memcpy(v,a.v,sizeof(T)*N);}
    inline void Zero(int x=0){memset(v,x,sizeof(T)*N);}
    inline static VectorBase<N,T> GetZero(int x=0){VectorBase<N,T> v; v.Zero(x); return v;}
    inline void Fill(const T &a){for (uint i=0;i<N;i++) v[i]=a;}
    inline static VectorBase<N,T> GetFill(const T &x){VectorBase<N,T> v; v.Fill(x); return v;}

    //positive and negative
    inline const VectorBase<N,T> operator+() const {return *this;}
    inline const VectorBase<N,T> operator-() const {VectorBase<N,T> ret; for (unsigned int i=0;i<N;i++) ret.v[i]=-v[i]; return ret;}

    //addition
    inline const VectorBase<N,T> operator+(const VectorBase<N,T> &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]+a.v[i];
        return ret;
    }
    inline const VectorBase<N,T> operator+(const T &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]+a;
        return ret;
    }
    inline void operator+=(const VectorBase<N,T> &a) {
        for (unsigned int i=0;i<N;i++)  v[i]+=a.v[i];
    }
    inline void operator+=(const T &a) {
        for (unsigned int i=0;i<N;i++)  v[i]+=a;
    }

    //subtraction
    inline const VectorBase<N,T> operator-(const VectorBase<N,T> &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]-a.v[i];
        return ret;
    }
    inline const VectorBase<N,T> operator-(const T &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]-a;
        return ret;
    }
    inline void operator-=(const VectorBase<N,T> &a) {
        for (unsigned int i=0;i<N;i++)  v[i]-=a.v[i];
    }
    inline void operator-=(const T &a) {
        for (unsigned int i=0;i<N;i++)  v[i]-=a;
    }

    //multiplication
    inline const VectorBase<N,T> operator*(const VectorBase<N,T> &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]*a.v[i];
        return ret;
    }
    inline const VectorBase<N,T> operator*(const T &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]*a;
        return ret;
    }
    inline void operator*=(const VectorBase<N,T> &a) {
        for (unsigned int i=0;i<N;i++)  v[i]*=a.v[i];
    }
    inline void operator*=(const T &a) {
        for (unsigned int i=0;i<N;i++)  v[i]*=a;
    }

    //division
    inline const VectorBase<N,T> operator/(const VectorBase<N,T> &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]/a.v[i];
        return ret;
    }
    inline const VectorBase<N,T> operator/(const T &a) const {
        VectorBase<N,T> ret;
        for (unsigned int i=0;i<N;i++)  ret.v[i]=v[i]/a;
        return ret;
    }
    inline void operator/=(const VectorBase<N,T> &a) {
        for (unsigned int i=0;i<N;i++)  v[i]/=a.v[i];
    }
    inline void operator/=(const T &a) {
        for (unsigned int i=0;i<N;i++)  v[i]/=a;
    }

    //math
    inline T Dot(const VectorBase<N,T> &a) const {T ret=0;for (unsigned int i=0;i<N;i++) ret+=v[i]*a.v[i]; return ret;}
    inline T Sum() const{T ret=0;for (unsigned int i=0;i<N;i++)  ret+=v[i];return ret;}
    inline void Negative() {for (uint i=0;i<N;i++) v[i]=-v[i];}
    inline T Len() const {return Sqrt<T>((double)LenSqr());}
    inline T LenSqr() const {return FastDot(*this, *this);}

    inline T SafeNormalize() {T norm=Len(); if (norm!=0) *this/=norm; return norm;}
    inline VectorBase<N,T> SafeNormalized() {T norm=Len(); if (norm!=0) return *this/norm; return *this;}
    inline T Normalize() {T norm=Len(); *this/=norm; return norm;}
    inline VectorBase<N,T> Normalized() const {T norm=Len(); return *this/norm;}

    inline T DistTo(const VectorBase<N,T> &a) const {return (T)sqrt((double)DistToSqr(a));}
    inline T DistToSqr(const VectorBase<N,T> &a) const {VectorBase<N,T> diff(*this-a); return diff.Dot(diff);}

    VectorBase<N,T> RandomPerpendicularUnitVec() const {
        VectorBase<N,T> ret((T)0);
        ret[v[0]>(T)(1-EPS)?1:0]=(T)1;
        ret-=*this*ret.Dot(*this)/LenSqr();
        ret.Normalize();
        return ret;
    }

    //Min and Max
    inline T Max() const {
        T maxv=v[0];
        for (unsigned int i=1;i<N;i++)  if (maxv<v[i]) maxv=v[i];
        return maxv;
    }
    inline int MaxPos() const {
        T maxv=v[0];
        int pos=0;
        for (unsigned int i=1;i<N;i++) if (maxv<v[i]) {maxv=v[i];pos=i;}
        return pos;
    }
    inline T Min() const {
        T minv=v[0];
        for (unsigned int i=1;i<N;i++) if (minv>v[i]) minv=v[i];
        return minv;
    }
    inline int MinPos() const {
        T minv=v[0];
        int pos=0;
        for (unsigned int i=1;i<N;i++) if (minv>v[i]) {minv=v[i];pos=i;}
        return pos;
    }
    inline T AbsMax() const {
        T maxv=Abs(v[0]);
        for (unsigned int i=1;i<N;i++)  if (maxv<Abs(v[i])) maxv=Abs(v[i]);
        return maxv;
    }
    inline int AbsMaxPos() const {
        T maxv=Abs(v[0]);
        int pos=0;
        for (unsigned int i=1;i<N;i++) if (maxv<Abs(v[i])) {maxv=Abs(v[i]);pos=i;}
        return pos;
    }
    inline T AbsMin() const {
        T minv=Abs(v[0]);
        for (unsigned int i=1;i<N;i++) if (minv>Abs(v[i])) minv=Abs(v[i]);
        return minv;
    }
    inline int AbsMinPos() const {
        T minv=Abs(v[0]);
        int pos=0;
        for (unsigned int i=1;i<N;i++) if (minv>Abs(v[i])) {minv=Abs(v[i]);pos=i;}
        return pos;
    }
    inline void RotateToLeft() {
        T tmp=v[0];
        for (uint i=0;i<N-1;i++) v[i]=v[i+1];
        v[N-1]=tmp;
    }
    inline void RotateToRight() {
        T tmp=v[N-1];
        for (uint i=N-1;i>=1;i--) v[i]=v[i-1];
        v[0]=tmp;
    }
    inline void KeepMin(const VectorBase<N,T> &y)
    {
        for (uint i=0;i<N;i++) v[i]=min(v[i],y[i]);
    }
    inline void KeepMax(const VectorBase<N,T> &y)
    {
        for (uint i=0;i<N;i++) v[i]=max(v[i],y[i]);
    }
    // comparisons
    bool operator== (const VectorBase<N,T>& y) const {return SafeEqual(begin(), end(), y.begin(), y.end());}
    bool operator< (const VectorBase<N,T>& y) const {return std::lexicographical_compare(begin(),end(),y.begin(),y.end());}
    bool operator!= (const VectorBase<N,T>& y) const {return !(*this==y);}
    bool operator> (const VectorBase<N,T>& y) const {return y<*this;}
    bool operator<= (const VectorBase<N,T>& y) const {return !(y<*this);}
    bool operator>= (const VectorBase<N,T>& y) const {return !(*this<y);}  

    //IO
    friend inline ostream& operator<<(ostream& os,const VectorBase<N,T> &me)
    {
        for (int i=0;i<N;i++)
            os<<me.v[i]<<' ';
        return os;
    }
    friend inline istream& operator>>(istream& is,VectorBase<N,T> &me)
    {
        for (int i=0;i<N;i++)
            is>>me.v[i];
        return is;
    }

    //constructor
    VectorBase(void){}
    VectorBase(const VectorBase<N,T>& a) {*this=a;}
    explicit VectorBase(const T &a){*this=a;}
    template<typename T1>
    explicit VectorBase(const T1 *p) {for (uint i=0;i<N;i++) v[i]=p[i];}
    template<typename T1>
    explicit VectorBase(const VectorBase<N,T1>& a) {*this=a;}
    template<typename T1>
    explicit VectorBase(const VectorBase<N-1,T1>& a, const T1 &b) {for (uint i=0;i<N-1;i++) v[i]=a[i]; v[N-1]=b;}
    template<typename T1>
    explicit VectorBase(const VectorBase<N+1,T1>& a)  {for (uint i=0;i<N;i++) v[i]=a[i];}
};

//scale at front
template <uint N, typename T>
inline const VectorBase<N,T> operator+(const T &v,const VectorBase<N,T> &vec)
{
    return vec+v;
}
template <uint N,typename T>
inline const VectorBase<N,T> operator-(const T &v,const VectorBase<N,T> &vec)
{
    return -vec+v;
}
template <uint N,typename T>
inline const VectorBase<N,T> operator*(const T &v,const VectorBase<N,T> &vec)
{
    return vec*v;
}
template <uint N,typename T>
inline const VectorBase<N,T> operator/(const T &v,const VectorBase<N,T> &vec)
{
    VectorBase<N,T> res;
    for (uint i=0;i<N;i++) res[i]=v/vec[i];
    return res;
}

//////////////////////////////////////////////////////////////////////////
// Class Vector
//////////////////////////////////////////////////////////////////////////

//write all these stuff just to avoid direct memory copy when construct and copy
template <unsigned int N,typename T>
class Vector : public VectorBase<N,T>
{
protected:
    using VectorBase<N,T>::v;
public:
    //constructor
    Vector(void){}
    Vector(const Vector<N,T>& a):VectorBase<N,T>(a) {}
    Vector(const VectorBase<N,T>& a):VectorBase<N,T>(a) {}
    explicit Vector(const T &a):VectorBase<N,T>(a){}
    explicit Vector(const VectorBase<N-1,T> &a, const T &b):VectorBase<N,T>(a,b){}
    explicit Vector(const VectorBase<N+1,T> &a):VectorBase<N,T>(a){}
    template<typename T1> explicit Vector(const T1 *p):VectorBase<N,T>(p) {}
    template<typename T1> explicit Vector(const Vector<N,T1>& a):VectorBase<N,T>(a) {}
    template<typename T1> explicit Vector(const VectorBase<N,T1>& a):VectorBase<N,T>(a) {}

    //assign
    inline const Vector<N,T> &operator=(const Vector<N,T>& a){VectorBase<N,T>::operator =(a); return *this;}
    using VectorBase<N,T>::operator=;

};

//meta-programming...
// primary template
template <unsigned int N, typename T>
struct DotProductHelper 
{
    static T Result (const T* a, const T* b) 
    {
        return *a * *b  +  DotProductHelper<N-1,T>::Result(a+1,b+1);
    }
};

// partial specialization as end criteria
template <typename T>
struct DotProductHelper<1,T> 
{
    static T Result (const T* a, const T* b) {return *a * *b;}
};

// convenience function
template <unsigned int N, typename T>
inline T FastDot(const VectorBase<N,T> &a, const VectorBase<N,T> &b)
{
    return DotProductHelper<N,T>::Result(a.Data(),b.Data());
}

//////////////////////////////////////////////////////////////////////////
// Class Vector2
//////////////////////////////////////////////////////////////////////////

template <typename T>
class Vector<2,T> : public VectorBase<2,T>
{
protected:
    using VectorBase<2,T>::v;
public:
    //constructor
    Vector(void){}
    Vector(const Vector<2,T>& a):VectorBase<2,T>(a){}
    Vector(const T &a, const T &b){v[0]=a;v[1]=b;}
    Vector(const VectorBase<2,T>& a):VectorBase<2,T>(a){}
    explicit Vector(const T &a):VectorBase<2,T>(a){}
    explicit Vector(const VectorBase<1,T> &a, const T &b):VectorBase<2,T>(a,b){}
    explicit Vector(const VectorBase<3,T> &a):VectorBase<2,T>(a){}
    template<typename T1> explicit Vector(const T1 *p):VectorBase<2,T>(p){}
    template<typename T1> explicit Vector(const Vector<2,T1>& a):VectorBase<2,T>(a){}
    template<typename T1> explicit Vector(const VectorBase<2,T1>& a):VectorBase<2,T>(a){}

    //assign
    inline const Vector<2,T> &operator=(const Vector<2,T>& a){VectorBase<2,T>::operator =(a); return *this;}
    inline void Set(const T &a, const T &b){v[0]=a; v[1]=b;}
    //using VectorBase<2,T>::operator=;

    // alias
    inline T& x(){return v[0];}
    inline T& y(){return v[1];}
    inline const T& x() const {return v[0];}
    inline const T& y() const {return v[1];}
};

typedef Vector<2, float> Vector2;
typedef Vector<2, double> Vector2d;

template<typename T>
inline T Cross(const VectorBase<2,T> &a, const VectorBase<2,T> &b)
{
    return a[0]*b[1]-a[1]*b[0];
}

//////////////////////////////////////////////////////////////////////////
// Class Vector3
//////////////////////////////////////////////////////////////////////////

template <typename T>
class Vector<3,T> : public VectorBase<3,T>
{
protected:
    using VectorBase<3,T>::v;
public:
    //constructor
    Vector(void){}
    Vector(const Vector<3,T>& a):VectorBase<3,T>(a){}
    Vector(const T &a,const T &b,const T &c){v[0]=a;v[1]=b;v[2]=c;}
    Vector(const VectorBase<3,T>& a):VectorBase<3,T>(a){}
    explicit Vector(const T &a):VectorBase<3,T>(a){}
    explicit Vector(const VectorBase<2,T> &a, const T &b):VectorBase<3,T>(a,b){}
    explicit Vector(const VectorBase<4,T> &a):VectorBase<3,T>(a){}
    template<typename T1> explicit Vector(const T1 *p):VectorBase<3,T>(p){}
    template<typename T1> explicit Vector(const Vector<3,T1>& a):VectorBase<3,T>(a){}
    template<typename T1> explicit Vector(const VectorBase<3,T1>& a):VectorBase<3,T>(a){}

    inline const Vector<3,T> &operator=(const Vector<3,T>& a){VectorBase<3,T>::operator =(a); return *this;}
    inline void Set(const T &a, const T &b, const T &c){v[0]=a; v[1]=b; v[2]=c;}
    //using VectorBase<3,T>::operator=;

    // Vector3 only cross
    inline Vector<3,T> Cross(const Vector<3,T> &a) const 
    {
        return Vector<3,T>(v[1]*a.v[2]-v[2]*a.v[1],v[2]*a.v[0]-v[0]*a.v[2],v[0]*a.v[1]-v[1]*a.v[0]);
    }

    // alias
    inline T& x(){return v[0];}
    inline T& y(){return v[1];}
    inline T& z(){return v[2];}
    inline T& r(){return v[0];}
    inline T& g(){return v[1];}
    inline T& b(){return v[2];}
    inline T& i(){return v[0];}
    inline T& j(){return v[1];}
    inline T& k(){return v[2];}
    inline const T& x() const {return v[0];}
    inline const T& y() const {return v[1];}
    inline const T& z() const {return v[2];}
    inline const T& r() const {return v[0];}
    inline const T& g() const {return v[1];}
    inline const T& b() const {return v[2];}
    inline const T& i() const {return v[0];}
    inline const T& j() const {return v[1];}
    inline const T& k() const {return v[2];}

};

template<typename T>
inline Vector<3,T> Cross(const VectorBase<3,T> &a, const VectorBase<3,T> &b)
{
    return Vector<3,T>(a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]);
}

template<typename T>
inline void Cross(const VectorBase<3,T> &a, const VectorBase<3,T> &b, VectorBase<3,T> &res)
{
    res[0]=a[1]*b[2]-a[2]*b[1];
    res[1]=a[2]*b[0]-a[0]*b[2];
    res[2]=a[0]*b[1]-a[1]*b[0];
}

typedef Vector<3, int> Vector3i;
typedef Vector<3, float> Vector3;
typedef Vector<3, double> Vector3d;

//////////////////////////////////////////////////////////////////////////
// Class Vector4
//////////////////////////////////////////////////////////////////////////

template <typename T>
class Vector<4,T> : public VectorBase<4,T>
{
protected:
    using VectorBase<4,T>::v;
public:
    //constructor
    Vector(void){}
    Vector(const Vector<4,T>& a):VectorBase<4,T>(a){}
    Vector(const T &a1,const T &b,const T &c,const T &d){v[0]=a1;v[1]=b;v[2]=c;v[3]=d;}
    Vector(const VectorBase<4,T>& a):VectorBase<4,T>(a){}
    explicit Vector(const T &a1):VectorBase<4,T>(a1){}
    explicit Vector(const VectorBase<3,T> &a, const T &b):VectorBase<4,T>(a,b){}
    explicit Vector(const VectorBase<5,T> &a):VectorBase<4,T>(a){}
    template<typename T1> explicit Vector(const T1 *p):VectorBase<4,T>(p) {}
    template<typename T1> explicit Vector(const Vector<4,T1>& a1):VectorBase<4,T>(a1) {}
    template<typename T1> explicit Vector(const VectorBase<4,T1>& a1):VectorBase<4,T>(a1) {}

    //assign
    inline const Vector<4,T> &operator=(const Vector<4,T>& a){VectorBase<4,T>::operator =(a); return *this;}
    inline void Set(const T &a, const T &b, const T &c, const T &d){v[0]=a; v[1]=b; v[2]=c; v[3]=d;}
    //using VectorBase<4,T>::operator=;

    // alias
    inline T& x(){return v[0];}
    inline T& y(){return v[1];}
    inline T& z(){return v[2];}
    inline T& w(){return v[3];}
    inline T& r(){return v[0];}
    inline T& g(){return v[1];}
    inline T& b(){return v[2];}
    inline T& a(){return v[3];}
    inline const T& x()const {return v[0];}
    inline const T& y()const {return v[1];}
    inline const T& z()const {return v[2];}
    inline const T& w()const {return v[3];}
    inline const T& r()const {return v[0];}
    inline const T& g()const {return v[1];}
    inline const T& b()const {return v[2];}
    inline const T& a()const {return v[3];}

};

typedef Vector<4, float> Vector4;
typedef Vector<4, double> Vector4d;
typedef Vector<4, int> Vector4i;

///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_VECTOR_H_
#define _CORE_GUARD_VECTOR_H_

// TODO 3: rotation from one vector to another
// TODO 3: extract axis from quaternion
// TODO 3: extract angle from quaternion
// TODO 5: quaternion spline, with spherically interpolation of quaternions using a smooth cubic spline
// TODO 2: fix flipped rotation in 2d-space (Rotated90 etc. goes into the wrong direction, normal rotation is CCW)

// NOTE: normalization needs to be safe by default, because an accidental null-normalization is hard to predict or detect in gameplay-code and can be fatal


// ****************************************************************
/* 2d-vector class */
class coreVector2 final
{
public:
    coreFloat x, y;


public:
    coreVector2() = default;
    constexpr FORCE_INLINE coreVector2(const coreFloat fx, const coreFloat fy)noexcept : x (fx), y (fy) {}

    ENABLE_COPY(coreVector2)
    ENABLE_COMPARISON(coreVector2)

    /* comparison operations */
    constexpr coreBool operator <  (const coreVector2 v)const {return (x <  v.x) && (y <  v.y);}
    constexpr coreBool operator <= (const coreVector2 v)const {return (x <= v.x) && (y <= v.y);}
    constexpr coreBool operator >  (const coreVector2 v)const {return (x >  v.x) && (y >  v.y);}
    constexpr coreBool operator >= (const coreVector2 v)const {return (x >= v.x) && (y >= v.y);}

    /* vector operations */
    constexpr coreVector2 operator +  (const coreVector2 v)const {return coreVector2(x+v.x, y+v.y);}
    constexpr coreVector2 operator -  (const coreVector2 v)const {return coreVector2(x-v.x, y-v.y);}
    constexpr coreVector2 operator *  (const coreVector2 v)const {return coreVector2(x*v.x, y*v.y);}
    constexpr coreVector2 operator /  (const coreVector2 v)const {return coreVector2(x*RCP(v.x), y*RCP(v.y));}
    inline    void        operator += (const coreVector2 v)      {*this = *this + v;}
    inline    void        operator -= (const coreVector2 v)      {*this = *this - v;}
    inline    void        operator *= (const coreVector2 v)      {*this = *this * v;}
    inline    void        operator /= (const coreVector2 v)      {*this = *this / v;}

    /* scalar operations */
    constexpr coreVector2 operator +  (const coreFloat f)const {return coreVector2(x+f, y+f);}
    constexpr coreVector2 operator -  (const coreFloat f)const {return coreVector2(x-f, y-f);}
    constexpr coreVector2 operator *  (const coreFloat f)const {return coreVector2(x*f, y*f);}
    constexpr coreVector2 operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    void        operator += (const coreFloat f)      {*this = *this + f;}
    inline    void        operator -= (const coreFloat f)      {*this = *this - f;}
    inline    void        operator *= (const coreFloat f)      {*this = *this * f;}
    inline    void        operator /= (const coreFloat f)      {*this = *this / f;}

    /* matrix operations */
    constexpr coreVector2 operator *  (const coreMatrix2& m)const;
    constexpr coreVector2 operator *  (const coreMatrix3& m)const;
    constexpr coreVector2 operator *  (const coreMatrix4& m)const;
    inline    void        operator *= (const coreMatrix2& m) {*this = *this * m;}
    inline    void        operator *= (const coreMatrix3& m) {*this = *this * m;}
    inline    void        operator *= (const coreMatrix4& m) {*this = *this * m;}

    /* convert vector */
    constexpr explicit operator coreBool ()const           {return (x != 0.0f) || (y != 0.0f);}
    inline          coreFloat& arr(const coreUintW i)      {ASSERT(i < 2u) return (&x)[i];}
    constexpr const coreFloat& arr(const coreUintW i)const {ASSERT(i < 2u) return (&x)[i];}
    constexpr const coreFloat* ptr()const                  {return (&x);}
    constexpr coreVector2      yx ()const                  {return coreVector2(y, x);}

    /* invert vector */
    constexpr coreVector2 operator - ()const {return coreVector2(-x, -y);}
    constexpr coreVector2 InvertedX  ()const {return coreVector2(-x,  y);}
    constexpr coreVector2 InvertedY  ()const {return coreVector2( x, -y);}

    /* rotate vector */
    constexpr coreVector2 Rotated90 ()const {return coreVector2(y,   -x);}
    constexpr coreVector2 Rotated45 ()const {return coreVector2(y+x, -x+y) * (1.0f / SQRT2);}
    constexpr coreVector2 Rotated135()const {return coreVector2(y-x, -x-y) * (1.0f / SQRT2);}
    constexpr coreVector2 Rotated30 ()const {return coreVector2( y, -x) * 0.5f + coreVector2( x,  y) * 0.8660254037844386467637231707529f;}
    constexpr coreVector2 Rotated60 ()const {return coreVector2( x,  y) * 0.5f + coreVector2( y, -x) * 0.8660254037844386467637231707529f;}
    constexpr coreVector2 Rotated120()const {return coreVector2(-x, -y) * 0.5f + coreVector2( y, -x) * 0.8660254037844386467637231707529f;}
    constexpr coreVector2 Rotated150()const {return coreVector2( y, -x) * 0.5f + coreVector2(-x, -y) * 0.8660254037844386467637231707529f;}

    /* normalize vector */
    constexpr coreVector2 Normalized      (const coreVector2 vFallback = coreVector2(0.0f,1.0f))const {ASSERT(vFallback.IsNormalized()) WARN_IF(this->IsNull()) return vFallback; return this->NormalizedUnsafe();}
    constexpr coreVector2 NormalizedUnsafe()const                                                     {ASSERT(!this->IsNull())          return coreVector2(x, y) * RSQRT(this->LengthSq());}

    /* process vector */
    template <typename F, typename... A> inline coreVector2 Processed(F&& nFunction, A&&... vArgs)const                                                      {return coreVector2(nFunction(x, std::forward<A>(vArgs)...), nFunction(y, std::forward<A>(vArgs)...));}
    inline coreVector2 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {return coreVector2(nFunction(x),                            nFunction(y));}
    inline coreVector2 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {return coreVector2(nFunction(x, f1),                        nFunction(y, f1));}
    inline coreVector2 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {return coreVector2(nFunction(x, f1, f2),                    nFunction(y, f1, f2));}

    /* direct functions */
    constexpr coreFloat   Length      ()const {return SQRT(this->LengthSq());}
    constexpr coreFloat   LengthSq    ()const {return (x*x + y*y);}
    constexpr coreFloat   Min         ()const {return MIN(x, y);}
    constexpr coreFloat   Max         ()const {return MAX(x, y);}
    constexpr coreUintW   MinDimension()const {return (x < y) ? 0u : 1u;}
    constexpr coreUintW   MaxDimension()const {return (x > y) ? 0u : 1u;}
    constexpr coreVector2 LowRatio    ()const {return ((*this) * RCP(this->Processed(ABS).Max()));}
    constexpr coreVector2 HighRatio   ()const {return ((*this) * RCP(this->Processed(ABS).Min()));}
    constexpr coreFloat   AspectRatio ()const {return (x * RCP(y));}
    inline    coreFloat   Angle       ()const {return (-std::atan2(x, y));}
    constexpr coreBool    IsNormalized()const {return (coreMath::IsNear(this->LengthSq(), 1.0f));}
    constexpr coreBool    IsAligned   ()const {return (x*y == 0.0f);}
    constexpr coreBool    IsNull      ()const {return (this->LengthSq() == 0.0f);}
    constexpr coreBool    IsUnorm     ()const {return (x >=  0.0f) && (x <= 1.0f) && (y >=  0.0f) && (y <= 1.0f);}
    constexpr coreBool    IsSnorm     ()const {return (x >= -1.0f) && (x <= 1.0f) && (y >= -1.0f) && (y <= 1.0f);}

    /* static functions */
    static constexpr coreFloat   Dot      (const coreVector2 v1, const coreVector2 v2);
    static inline    coreFloat   Angle    (const coreVector2 v1, const coreVector2 v2);
    static inline    coreVector2 Direction(const coreFloat fAngle);
    static inline    coreVector2 Rand     (coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector2 Rand     (const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector2 Rand     (const coreFloat fMin,  const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector2 Rand     (const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, coreRand* OUTPUT pRand = Core::Rand);
    static constexpr coreVector2 Reflect  (const coreVector2 vVelocity, const coreVector2 vNormal);

    /* packing functions */
    constexpr        coreUint16  PackUnorm2x8   ()const;
    constexpr        coreUint16  PackSnorm2x8   ()const;
    constexpr        coreUint32  PackUnorm2x16  ()const;
    constexpr        coreUint32  PackSnorm2x16  ()const;
    constexpr        coreUint32  PackFloat2x16  ()const;
    constexpr        coreUint64  PackFloat2x32  ()const;
    static constexpr coreVector2 UnpackUnorm2x8 (const coreUint16 iNumber);
    static constexpr coreVector2 UnpackSnorm2x8 (const coreUint16 iNumber);
    static constexpr coreVector2 UnpackUnorm2x16(const coreUint32 iNumber);
    static constexpr coreVector2 UnpackSnorm2x16(const coreUint32 iNumber);
    static constexpr coreVector2 UnpackFloat2x16(const coreUint32 iNumber);
    static constexpr coreVector2 UnpackFloat2x32(const coreUint64 iNumber);
};


// ****************************************************************
/* 3d-vector class */
class coreVector3 final
{
public:
    coreFloat x, y, z;


public:
    coreVector3() = default;
    constexpr FORCE_INLINE coreVector3(const coreVector2 v, const coreFloat fz)noexcept                    : x (v.x), y (v.y), z (fz)  {}
    constexpr FORCE_INLINE coreVector3(const coreFloat fx, const coreVector2 v)noexcept                    : x (fx),  y (v.x), z (v.y) {}
    constexpr FORCE_INLINE coreVector3(const coreFloat fx, const coreFloat fy, const coreFloat fz)noexcept : x (fx),  y (fy),  z (fz)  {}

    ENABLE_COPY(coreVector3)
    ENABLE_COMPARISON(coreVector3)

    /* comparison operations */
    constexpr coreBool operator <  (const coreVector3 v)const {return (x <  v.x) && (y <  v.y) && (z <  v.z);}
    constexpr coreBool operator <= (const coreVector3 v)const {return (x <= v.x) && (y <= v.y) && (z <= v.z);}
    constexpr coreBool operator >  (const coreVector3 v)const {return (x >  v.x) && (y >  v.y) && (z >  v.z);}
    constexpr coreBool operator >= (const coreVector3 v)const {return (x >= v.x) && (y >= v.y) && (z >= v.z);}

    /* vector operations */
    constexpr coreVector3 operator +  (const coreVector3 v)const {return coreVector3(x+v.x, y+v.y, z+v.z);}
    constexpr coreVector3 operator -  (const coreVector3 v)const {return coreVector3(x-v.x, y-v.y, z-v.z);}
    constexpr coreVector3 operator *  (const coreVector3 v)const {return coreVector3(x*v.x, y*v.y, z*v.z);}
    constexpr coreVector3 operator /  (const coreVector3 v)const {return coreVector3(x*RCP(v.x), y*RCP(v.y), z*RCP(v.z));}
    inline    void        operator += (const coreVector3 v)      {*this = *this + v;}
    inline    void        operator -= (const coreVector3 v)      {*this = *this - v;}
    inline    void        operator *= (const coreVector3 v)      {*this = *this * v;}
    inline    void        operator /= (const coreVector3 v)      {*this = *this / v;}

    /* scalar operations */
    constexpr coreVector3 operator +  (const coreFloat f)const {return coreVector3(x+f, y+f, z+f);}
    constexpr coreVector3 operator -  (const coreFloat f)const {return coreVector3(x-f, y-f, z-f);}
    constexpr coreVector3 operator *  (const coreFloat f)const {return coreVector3(x*f, y*f, z*f);}
    constexpr coreVector3 operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    void        operator += (const coreFloat f)      {*this = *this + f;}
    inline    void        operator -= (const coreFloat f)      {*this = *this - f;}
    inline    void        operator *= (const coreFloat f)      {*this = *this * f;}
    inline    void        operator /= (const coreFloat f)      {*this = *this / f;}

    /* matrix operations */
    constexpr coreVector3 operator *  (const coreMatrix3& m)const;
    constexpr coreVector3 operator *  (const coreMatrix4& m)const;
    inline    void        operator *= (const coreMatrix3& m) {*this = *this * m;}
    inline    void        operator *= (const coreMatrix4& m) {*this = *this * m;}

    /* convert vector */
    constexpr explicit operator coreBool ()const           {return (x != 0.0f) || (y != 0.0f) || (z != 0.0f);}
    inline          coreFloat& arr(const coreUintW i)      {ASSERT(i < 3u) return (&x)[i];}
    constexpr const coreFloat& arr(const coreUintW i)const {ASSERT(i < 3u) return (&x)[i];}
    constexpr const coreFloat* ptr()const                  {return (&x);}
    constexpr coreVector3      xzy()const                  {return coreVector3(x, z, y);}
    constexpr coreVector3      yxz()const                  {return coreVector3(y, x, z);}
    constexpr coreVector3      yzx()const                  {return coreVector3(y, z, x);}
    constexpr coreVector3      zxy()const                  {return coreVector3(z, x, y);}
    constexpr coreVector3      zyx()const                  {return coreVector3(z, y, x);}
    constexpr coreVector2      xy ()const                  {return coreVector2(x, y);}
    constexpr coreVector2      xz ()const                  {return coreVector2(x, z);}
    constexpr coreVector2      yx ()const                  {return coreVector2(y, x);}
    constexpr coreVector2      yz ()const                  {return coreVector2(y, z);}
    constexpr coreVector2      zx ()const                  {return coreVector2(z, x);}
    constexpr coreVector2      zy ()const                  {return coreVector2(z, y);}
    inline    void             xy (const coreVector2 v)    {x = v.x; y = v.y;}
    inline    void             xz (const coreVector2 v)    {x = v.x; z = v.y;}
    inline    void             yx (const coreVector2 v)    {y = v.x; x = v.y;}
    inline    void             yz (const coreVector2 v)    {y = v.x; z = v.y;}
    inline    void             zx (const coreVector2 v)    {z = v.x; x = v.y;}
    inline    void             zy (const coreVector2 v)    {z = v.x; y = v.y;}

    /* invert vector */
    constexpr coreVector3 operator - ()const {return coreVector3(-x, -y, -z);}
    constexpr coreVector3 InvertedX  ()const {return coreVector3(-x,  y,  z);}
    constexpr coreVector3 InvertedY  ()const {return coreVector3( x, -y,  z);}
    constexpr coreVector3 InvertedZ  ()const {return coreVector3( x,  y, -z);}

    /* rotate vector */
    constexpr coreVector3 RotatedX90 ()const {return coreVector3(x, this->yz().Rotated90 ());}
    constexpr coreVector3 RotatedX45 ()const {return coreVector3(x, this->yz().Rotated45 ());}
    constexpr coreVector3 RotatedX135()const {return coreVector3(x, this->yz().Rotated135());}
    constexpr coreVector3 RotatedY90 ()const {return coreVector3(this->xz().Rotated90 (), y).xzy();}
    constexpr coreVector3 RotatedY45 ()const {return coreVector3(this->xz().Rotated45 (), y).xzy();}
    constexpr coreVector3 RotatedY135()const {return coreVector3(this->xz().Rotated135(), y).xzy();}
    constexpr coreVector3 RotatedZ90 ()const {return coreVector3(this->xy().Rotated90 (), z);}
    constexpr coreVector3 RotatedZ45 ()const {return coreVector3(this->xy().Rotated45 (), z);}
    constexpr coreVector3 RotatedZ135()const {return coreVector3(this->xy().Rotated135(), z);}

    /* normalize vector */
    constexpr coreVector3 Normalized      (const coreVector3 vFallback = coreVector3(0.0f,0.0f,1.0f))const {ASSERT(vFallback.IsNormalized()) WARN_IF(this->IsNull()) return vFallback; return this->NormalizedUnsafe();}
    constexpr coreVector3 NormalizedUnsafe()const                                                          {ASSERT(!this->IsNull())          return coreVector3(x, y, z) * RSQRT(this->LengthSq());}

    /* process vector */
    template <typename F, typename... A> inline coreVector3 Processed(F&& nFunction, A&&... vArgs)const                                                      {return coreVector3(nFunction(x, std::forward<A>(vArgs)...), nFunction(y, std::forward<A>(vArgs)...), nFunction(z, std::forward<A>(vArgs)...));}
    inline coreVector3 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {return coreVector3(nFunction(x),                            nFunction(y),                            nFunction(z));}
    inline coreVector3 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {return coreVector3(nFunction(x, f1),                        nFunction(y, f1),                        nFunction(z, f1));}
    inline coreVector3 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {return coreVector3(nFunction(x, f1, f2),                    nFunction(y, f1, f2),                    nFunction(z, f1, f2));}

    /* direct functions */
    constexpr coreFloat   Length      ()const {return SQRT(this->LengthSq());}
    constexpr coreFloat   LengthSq    ()const {return (x*x + y*y + z*z);}
    constexpr coreFloat   Min         ()const {return MIN(x, y, z);}
    constexpr coreFloat   Max         ()const {return MAX(x, y, z);}
    constexpr coreUintW   MinDimension()const {return (x < y) ? ((x < z) ? 0u : 2u) : ((y < z) ? 1u : 2u);}
    constexpr coreUintW   MaxDimension()const {return (x > y) ? ((x > z) ? 0u : 2u) : ((y > z) ? 1u : 2u);}
    constexpr coreVector3 LowRatio    ()const {return ((*this) * RCP(this->Processed(ABS).Max()));}
    constexpr coreVector3 HighRatio   ()const {return ((*this) * RCP(this->Processed(ABS).Min()));}
    constexpr coreBool    IsNormalized()const {return (coreMath::IsNear(this->LengthSq(), 1.0f));}
    constexpr coreBool    IsAligned   ()const {return (x*y == 0.0f) && (x*z == 0.0f) && (y*z == 0.0f);}
    constexpr coreBool    IsNull      ()const {return (this->LengthSq() == 0.0f);}
    constexpr coreBool    IsUnorm     ()const {return (x >=  0.0f) && (x <= 1.0f) && (y >=  0.0f) && (y <= 1.0f) && (z >=  0.0f) && (z <= 1.0f);}
    constexpr coreBool    IsSnorm     ()const {return (x >= -1.0f) && (x <= 1.0f) && (y >= -1.0f) && (y <= 1.0f) && (z >= -1.0f) && (z <= 1.0f);}

    /* static functions */
    static constexpr coreFloat   Dot    (const coreVector3 v1, const coreVector3 v2);
    static constexpr coreVector3 Cross  (const coreVector3 v1, const coreVector3 v2);
    static inline    coreFloat   Angle  (const coreVector3 v1, const coreVector3 v2);
    static inline    coreVector3 Rand   (coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector3 Rand   (const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector3 Rand   (const coreFloat fMin,  const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector3 Rand   (const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, const coreFloat fMinZ, const coreFloat fMaxZ, coreRand* OUTPUT pRand = Core::Rand);
    static constexpr coreVector3 Reflect(const coreVector3 vVelocity, const coreVector3 vNormal);
    static inline    coreBool    Visible(const coreVector3 vPosition, const coreFloat fFOV, const coreVector3 vViewPosition, const coreVector3 vViewDirection);

    /* packing functions */
    constexpr        coreUint16  PackUnorm565   ()const;
    constexpr        coreUint16  PackSnorm565   ()const;
    constexpr        coreUint32  PackUnorm011   ()const;
    constexpr        coreUint32  PackSnorm011   ()const;
    inline           coreVector2 PackSnormOcta  ()const;
    static constexpr coreVector3 UnpackUnorm565 (const coreUint16  iNumber);
    static constexpr coreVector3 UnpackSnorm565 (const coreUint16  iNumber);
    static constexpr coreVector3 UnpackUnorm011 (const coreUint32  iNumber);
    static constexpr coreVector3 UnpackSnorm011 (const coreUint32  iNumber);
    static inline    coreVector3 UnpackSnormOcta(const coreVector2 vVector);

    /* color functions */
    constexpr coreVector3 RgbToHsv  ()const;
    constexpr coreVector3 HsvToRgb  ()const;
    constexpr coreVector3 RgbToYiq  ()const;
    constexpr coreVector3 YiqToRgb  ()const;
    constexpr coreVector3 RgbToYuv  ()const;
    constexpr coreVector3 YuvToRgb  ()const;
    constexpr coreVector3 RgbToYcbcr()const;
    constexpr coreVector3 YcbcrToRgb()const;
    constexpr coreFloat   Luminance ()const {return coreVector3::Dot(*this, coreVector3(0.2126f, 0.7152f, 0.0722f));}   // BT.709
};


// ****************************************************************
/* 4d-vector and quaternion class */
class coreVector4 final
{
public:
    coreFloat x, y, z, w;


public:
    coreVector4() = default;
    constexpr FORCE_INLINE coreVector4(const coreVector3 v, const coreFloat fw)noexcept                                        : x (v.x),  y (v.y),  z (v.z),  w (fw)   {}
    constexpr FORCE_INLINE coreVector4(const coreFloat fx, const coreVector3 v)noexcept                                        : x (fx),   y (v.x),  z (v.y),  w (v.z)  {}
    constexpr FORCE_INLINE coreVector4(const coreVector2 v, const coreFloat fz, const coreFloat fw)noexcept                    : x (v.x),  y (v.y),  z (fz),   w (fw)   {}
    constexpr FORCE_INLINE coreVector4(const coreFloat fx, const coreVector2 v, const coreFloat fw)noexcept                    : x (fx),   y (v.x),  z (v.y),  w (fw)   {}
    constexpr FORCE_INLINE coreVector4(const coreFloat fx, const coreFloat fy, const coreVector2 v)noexcept                    : x (fx),   y (fy),   z (v.x),  w (v.y)  {}
    constexpr FORCE_INLINE coreVector4(const coreVector2 v1, const coreVector2 v2)noexcept                                     : x (v1.x), y (v1.y), z (v2.x), w (v2.y) {}
    constexpr FORCE_INLINE coreVector4(const coreFloat fx, const coreFloat fy, const coreFloat fz, const coreFloat fw)noexcept : x (fx),   y (fy),   z (fz),   w (fw)   {}

    ENABLE_COPY(coreVector4)
    ENABLE_COMPARISON(coreVector4)

    /* comparison operations */
    constexpr coreBool operator <  (const coreVector4 v)const {return (x <  v.x) && (y <  v.y) && (z <  v.z) && (w <  v.w);}
    constexpr coreBool operator <= (const coreVector4 v)const {return (x <= v.x) && (y <= v.y) && (z <= v.z) && (w <= v.w);}
    constexpr coreBool operator >  (const coreVector4 v)const {return (x >  v.x) && (y >  v.y) && (z >  v.z) && (w >  v.w);}
    constexpr coreBool operator >= (const coreVector4 v)const {return (x >= v.x) && (y >= v.y) && (z >= v.z) && (w >= v.w);}

    /* vector operations */
    constexpr coreVector4 operator +  (const coreVector4 v)const {return coreVector4(x+v.x, y+v.y, z+v.z, w+v.w);}
    constexpr coreVector4 operator -  (const coreVector4 v)const {return coreVector4(x-v.x, y-v.y, z-v.z, w-v.w);}
    constexpr coreVector4 operator *  (const coreVector4 v)const {return coreVector4(x*v.x, y*v.y, z*v.z, w*v.w);}
    constexpr coreVector4 operator /  (const coreVector4 v)const {return coreVector4(x*RCP(v.x), y*RCP(v.y), z*RCP(v.z), w*RCP(v.w));}
    inline    void        operator += (const coreVector4 v)      {*this = *this + v;}
    inline    void        operator -= (const coreVector4 v)      {*this = *this - v;}
    inline    void        operator *= (const coreVector4 v)      {*this = *this * v;}
    inline    void        operator /= (const coreVector4 v)      {*this = *this / v;}

    /* scalar operations */
    constexpr coreVector4 operator +  (const coreFloat f)const {return coreVector4(x+f, y+f, z+f, w+f);}
    constexpr coreVector4 operator -  (const coreFloat f)const {return coreVector4(x-f, y-f, z-f, w-f);}
    constexpr coreVector4 operator *  (const coreFloat f)const {return coreVector4(x*f, y*f, z*f, w*f);}
    constexpr coreVector4 operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    void        operator += (const coreFloat f)      {*this = *this + f;}
    inline    void        operator -= (const coreFloat f)      {*this = *this - f;}
    inline    void        operator *= (const coreFloat f)      {*this = *this * f;}
    inline    void        operator /= (const coreFloat f)      {*this = *this / f;}

    /* matrix operations */
    constexpr coreVector4 operator *  (const coreMatrix4& m)const;
    inline    void        operator *= (const coreMatrix4& m) {*this = *this * m;}

    /* convert vector */
    constexpr explicit operator coreBool ()const            {return (x != 0.0f) || (y != 0.0f) || (z != 0.0f) || (w != 0.0f);}
    inline          coreFloat& arr (const coreUintW i)      {ASSERT(i < 4u) return (&x)[i];}
    constexpr const coreFloat& arr (const coreUintW i)const {ASSERT(i < 4u) return (&x)[i];}
    constexpr const coreFloat* ptr ()const                  {return (&x);}
    constexpr coreVector3      xyzw()const                  {return coreVector3(x, y, z) * w;}
    constexpr coreVector3      xyz ()const                  {return coreVector3(x, y, z);}
    constexpr coreVector2      xy  ()const                  {return coreVector2(x, y);}
    constexpr coreVector2      yz  ()const                  {return coreVector2(y, z);}
    constexpr coreVector2      zw  ()const                  {return coreVector2(z, w);}
    inline    void             xyz (const coreVector3 v)    {x = v.x; y = v.y; z = v.z;}
    inline    void             xy  (const coreVector2 v)    {x = v.x; y = v.y;}
    inline    void             yz  (const coreVector2 v)    {y = v.x; z = v.y;}
    inline    void             zw  (const coreVector2 v)    {z = v.x; w = v.y;}

    /* invert vector */
    constexpr coreVector4 operator - ()const {return coreVector4(-x, -y, -z, -w);}
    constexpr coreVector4 InvertedX  ()const {return coreVector4(-x,  y,  z,  w);}
    constexpr coreVector4 InvertedY  ()const {return coreVector4( x, -y,  z,  w);}
    constexpr coreVector4 InvertedZ  ()const {return coreVector4( x,  y, -z,  w);}
    constexpr coreVector4 InvertedW  ()const {return coreVector4( x,  y,  z, -w);}

    /* normalize vector */
    constexpr coreVector4 Normalized      (const coreVector4 vFallback = coreVector4(0.0f,0.0f,0.0f,1.0f))const {ASSERT(vFallback.IsNormalized()) WARN_IF(this->IsNull()) return vFallback; return this->NormalizedUnsafe();}
    constexpr coreVector4 NormalizedUnsafe()const                                                               {ASSERT(!this->IsNull())          return coreVector4(x, y, z, w) * RSQRT(this->LengthSq());}

    /* process vector */
    template <typename F, typename... A> inline coreVector4 Processed(F&& nFunction, A&&... vArgs)const                                                      {return coreVector4(nFunction(x, std::forward<A>(vArgs)...), nFunction(y, std::forward<A>(vArgs)...), nFunction(z, std::forward<A>(vArgs)...), nFunction(w, std::forward<A>(vArgs)...));}
    inline coreVector4 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {return coreVector4(nFunction(x),                            nFunction(y),                            nFunction(z),                            nFunction(w));}
    inline coreVector4 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {return coreVector4(nFunction(x, f1),                        nFunction(y, f1),                        nFunction(z, f1),                        nFunction(w, f1));}
    inline coreVector4 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {return coreVector4(nFunction(x, f1, f2),                    nFunction(y, f1, f2),                    nFunction(z, f1, f2),                    nFunction(w, f1, f2));}

    /* direct functions */
    constexpr coreFloat   Length      ()const {return SQRT(this->LengthSq());}
    constexpr coreFloat   LengthSq    ()const {return (x*x + y*y + z*z + w*w);}
    constexpr coreFloat   Min         ()const {return MIN(x, y, z, w);}
    constexpr coreFloat   Max         ()const {return MAX(x, y, z, w);}
    constexpr coreUintW   MinDimension()const {return (x < y) ? ((x < z) ? ((x < w) ? 0u : 3u) : ((z < w) ? 2u : 3u)) : ((y < z) ? ((y < w) ? 1u : 3u) : ((z < w) ? 2u : 3u));}
    constexpr coreUintW   MaxDimension()const {return (x > y) ? ((x > z) ? ((x > w) ? 0u : 3u) : ((z > w) ? 2u : 3u)) : ((y > z) ? ((y > w) ? 1u : 3u) : ((z > w) ? 2u : 3u));}
    constexpr coreVector4 LowRatio    ()const {return ((*this) * RCP(this->Processed(ABS).Max()));}
    constexpr coreVector4 HighRatio   ()const {return ((*this) * RCP(this->Processed(ABS).Min()));}
    constexpr coreBool    IsNormalized()const {return (coreMath::IsNear(this->LengthSq(), 1.0f));}
    constexpr coreBool    IsAligned   ()const {return (x*y == 0.0f) && (x*z == 0.0f) && (x*w == 0.0f) && (y*z == 0.0f) && (y*w == 0.0f) && (z*w == 0.0f);}
    constexpr coreBool    IsNull      ()const {return (this->LengthSq() == 0.0f);}
    constexpr coreBool    IsUnorm     ()const {return (x >=  0.0f) && (x <= 1.0f) && (y >=  0.0f) && (y <= 1.0f) && (z >=  0.0f) && (z <= 1.0f) && (w >=  0.0f) && (w <= 1.0f);}
    constexpr coreBool    IsSnorm     ()const {return (x >= -1.0f) && (x <= 1.0f) && (y >= -1.0f) && (y <= 1.0f) && (z >= -1.0f) && (z <= 1.0f) && (w >= -1.0f) && (w <= 1.0f);}

    /* static functions */
    static constexpr coreFloat   Dot (const coreVector4 v1, const coreVector4 v2);
    static inline    coreVector4 Rand(coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector4 Rand(const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector4 Rand(const coreFloat fMin,  const coreFloat fMax, coreRand* OUTPUT pRand = Core::Rand);
    static inline    coreVector4 Rand(const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, const coreFloat fMinZ, const coreFloat fMaxZ, const coreFloat fMinW, const coreFloat fMaxW, coreRand* OUTPUT pRand = Core::Rand);

    /* packing functions */
    constexpr        coreUint32  PackUnorm210   ()const;
    constexpr        coreUint32  PackSnorm210   ()const;
    constexpr        coreUint32  PackUnorm4x8   ()const;
    constexpr        coreUint32  PackSnorm4x8   ()const;
    constexpr        coreUint64  PackUnorm4x16  ()const;
    constexpr        coreUint64  PackSnorm4x16  ()const;
    constexpr        coreUint64  PackFloat4x16  ()const;
    static constexpr coreVector4 UnpackUnorm210 (const coreUint32 iNumber);
    static constexpr coreVector4 UnpackSnorm210 (const coreUint32 iNumber);
    static constexpr coreVector4 UnpackUnorm4x8 (const coreUint32 iNumber);
    static constexpr coreVector4 UnpackSnorm4x8 (const coreUint32 iNumber);
    static constexpr coreVector4 UnpackUnorm4x16(const coreUint64 iNumber);
    static constexpr coreVector4 UnpackSnorm4x16(const coreUint64 iNumber);
    static constexpr coreVector4 UnpackFloat4x16(const coreUint64 iNumber);

    /* quaternion functions */
    static constexpr coreVector4 QuatMultiply (const coreVector4 v1, const coreVector4 v2);
    static inline    coreVector4 QuatSlerp    (const coreVector4 v1, const coreVector4 v2, const coreFloat t);
    static constexpr coreVector4 QuatIdentity ()      {return coreVector4(0.0f,0.0f,0.0f,1.0f);}
    constexpr        coreVector4 QuatConjugate()const {return coreVector4(-x, -y, -z, w);}
    constexpr        coreVector4 QuatInvert   ()const {return coreVector4(-x, -y, -z, w) * RCP(this->LengthSq());}
    constexpr        coreVector3 QuatApply    (const coreVector3 v)const;
};


// ****************************************************************
/* global scalar operations */
constexpr coreVector2 operator + (const coreFloat f, const coreVector2 v) {return v + f;}
constexpr coreVector2 operator * (const coreFloat f, const coreVector2 v) {return v * f;}
constexpr coreVector3 operator + (const coreFloat f, const coreVector3 v) {return v + f;}
constexpr coreVector3 operator * (const coreFloat f, const coreVector3 v) {return v * f;}
constexpr coreVector4 operator + (const coreFloat f, const coreVector4 v) {return v + f;}
constexpr coreVector4 operator * (const coreFloat f, const coreVector4 v) {return v * f;}


// ****************************************************************
/* calculate dot product */
constexpr coreFloat coreVector2::Dot(const coreVector2 v1, const coreVector2 v2)
{
    return (v1.x*v2.x + v1.y*v2.y);
}


// ****************************************************************
/* calculate angle between two vectors */
inline coreFloat coreVector2::Angle(const coreVector2 v1, const coreVector2 v2)
{
    return ACOS(coreVector2::Dot(v1, v2));
}


// ****************************************************************
/* calculate direction vector */
inline coreVector2 coreVector2::Direction(const coreFloat fAngle)
{
    return coreVector2(-SIN(fAngle), COS(fAngle));
}


// ****************************************************************
/* generate random vector */
inline coreVector2 coreVector2::Rand(coreRand* OUTPUT pRand)
{
    return coreVector2(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized();
}

inline coreVector2 coreVector2::Rand(const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector2(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMax);
}

inline coreVector2 coreVector2::Rand(const coreFloat fMin, const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector2(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMin, fMax);
}

inline coreVector2 coreVector2::Rand(const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, coreRand* OUTPUT pRand)
{
    return coreVector2(pRand->Float(fMinX, fMaxX),
                       pRand->Float(fMinY, fMaxY));
}


// ****************************************************************
/* calculate reflected vector */
constexpr coreVector2 coreVector2::Reflect(const coreVector2 vVelocity, const coreVector2 vNormal)
{
    const coreFloat fDot = coreVector2::Dot(vVelocity, vNormal);
    return (fDot >= 0.0f) ? vVelocity : (vVelocity - vNormal * (2.0f*fDot));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into YX packed uint16 */
constexpr coreUint16 coreVector2::PackUnorm2x8()const
{
    ASSERT(this->IsUnorm())
    return (coreUint16(y * 255.0f) << 8u) |
           (coreUint16(x * 255.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into YX packed uint16 */
constexpr coreUint16 coreVector2::PackSnorm2x8()const
{
    ASSERT(this->IsSnorm())
    return (coreUint16((y < 0.0f) ? (256.0f + y*128.0f) : (y*127.0f)) << 8u) |
           (coreUint16((x < 0.0f) ? (256.0f + x*128.0f) : (x*127.0f)));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into YX packed uint32 */
constexpr coreUint32 coreVector2::PackUnorm2x16()const
{
    ASSERT(this->IsUnorm())
    return (F_TO_UI(y * 65535.0f) << 16u) |
           (F_TO_UI(x * 65535.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into YX packed uint32 */
constexpr coreUint32 coreVector2::PackSnorm2x16()const
{
    ASSERT(this->IsSnorm())
    return (F_TO_UI((y < 0.0f) ? (65536.0f + y*32768.0f) : (y*32767.0f)) << 16u) |
           (F_TO_UI((x < 0.0f) ? (65536.0f + x*32768.0f) : (x*32767.0f)));
}


// ****************************************************************
/* compress arbitrary vector into YX packed uint32 */
constexpr coreUint32 coreVector2::PackFloat2x16()const
{
    return (coreUint32(coreMath::Float32To16(y)) << 16u) |
           (coreUint32(coreMath::Float32To16(x)));
}


// ****************************************************************
/* safely convert vector into bit-representation */
constexpr coreUint64 coreVector2::PackFloat2x32()const
{
    return std::bit_cast<coreUint64>(*this);
}


// ****************************************************************
/* uncompress YX packed uint16 into 0.0 to 1.0 vector */
constexpr coreVector2 coreVector2::UnpackUnorm2x8(const coreUint16 iNumber)
{
    return coreVector2(I_TO_F( iNumber        & 0xFFu),
                       I_TO_F((iNumber >> 8u) & 0xFFu)) * 3.921568627e-3f;
}


// ****************************************************************
/* uncompress YX packed uint16 into -1.0 to 1.0 vector */
constexpr coreVector2 coreVector2::UnpackSnorm2x8(const coreUint16 iNumber)
{
    const coreVector2 A = coreVector2(I_TO_F( iNumber        & 0xFFu),
                                      I_TO_F((iNumber >> 8u) & 0xFFu));

    return coreVector2((A.x >= 128.0f) ? ((A.x - 256.0f)/128.0f) : (A.x/127.0f),
                       (A.y >= 128.0f) ? ((A.y - 256.0f)/128.0f) : (A.y/127.0f));
}


// ****************************************************************
/* uncompress YX packed uint32 into 0.0 to 1.0 vector */
constexpr coreVector2 coreVector2::UnpackUnorm2x16(const coreUint32 iNumber)
{
    return coreVector2(I_TO_F( iNumber         & 0xFFFFu),
                       I_TO_F((iNumber >> 16u) & 0xFFFFu)) * 1.525902189e-5f;
}


// ****************************************************************
/* uncompress YX packed uint32 into -1.0 to 1.0 vector */
constexpr coreVector2 coreVector2::UnpackSnorm2x16(const coreUint32 iNumber)
{
    const coreVector2 A = coreVector2(I_TO_F( iNumber         & 0xFFFFu),
                                      I_TO_F((iNumber >> 16u) & 0xFFFFu));

    return coreVector2((A.x >= 32768.0f) ? ((A.x - 65536.0f)/32768.0f) : (A.x/32767.0f),
                       (A.y >= 32768.0f) ? ((A.y - 65536.0f)/32768.0f) : (A.y/32767.0f));
}


// ****************************************************************
/* uncompress YX packed uint32 into arbitrary vector */
constexpr coreVector2 coreVector2::UnpackFloat2x16(const coreUint32 iNumber)
{
    return coreVector2(coreMath::Float16To32( iNumber         & 0xFFFFu),
                       coreMath::Float16To32((iNumber >> 16u) & 0xFFFFu));
}


// ****************************************************************
/* safely convert bit-representation into vector */
constexpr coreVector2 coreVector2::UnpackFloat2x32(const coreUint64 iNumber)
{
    return std::bit_cast<coreVector2>(iNumber);
}


// ****************************************************************
/* calculate dot product */
constexpr coreFloat coreVector3::Dot(const coreVector3 v1, const coreVector3 v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}


// ****************************************************************
/* calculate cross product */
constexpr coreVector3 coreVector3::Cross(const coreVector3 v1, const coreVector3 v2)
{
    return coreVector3(v1.y*v2.z - v1.z*v2.y,
                       v1.z*v2.x - v1.x*v2.z,
                       v1.x*v2.y - v1.y*v2.x);
}


// ****************************************************************
/* calculate angle between two vectors */
inline coreFloat coreVector3::Angle(const coreVector3 v1, const coreVector3 v2)
{
    return ACOS(coreVector3::Dot(v1, v2));
}


// ****************************************************************
/* generate random vector */
inline coreVector3 coreVector3::Rand(coreRand* OUTPUT pRand)
{
    return coreVector3(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized();
}

inline coreVector3 coreVector3::Rand(const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector3(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMax);
}

inline coreVector3 coreVector3::Rand(const coreFloat fMin, const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector3(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMin, fMax);
}

inline coreVector3 coreVector3::Rand(const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, const coreFloat fMinZ, const coreFloat fMaxZ, coreRand* OUTPUT pRand)
{
    return coreVector3(pRand->Float(fMinX, fMaxX),
                       pRand->Float(fMinY, fMaxY),
                       pRand->Float(fMinZ, fMaxZ));
}


// ****************************************************************
/* calculate reflected vector */
constexpr coreVector3 coreVector3::Reflect(const coreVector3 vVelocity, const coreVector3 vNormal)
{
    const coreFloat fDot = coreVector3::Dot(vVelocity, vNormal);
    return (fDot >= 0.0f) ? vVelocity : (vVelocity - vNormal * (2.0f*fDot));
}


// ****************************************************************
/* check if inside field-of-view */
inline coreBool coreVector3::Visible(const coreVector3 vPosition, const coreFloat fFOV, const coreVector3 vViewPosition, const coreVector3 vViewDirection)
{
    // calculate relative position and angle
    const coreVector3 vRelative = (vPosition - vViewPosition).Normalized();
    const coreFloat   fDot      = coreVector3::Dot(vRelative, vViewDirection);

    // check result
    ASSERT(fFOV <= 0.5f*PI)
    return (fDot < 0.0f) ? false : (fDot > COS(fFOV));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into 5_6_5_rev packed uint16 */
constexpr coreUint16 coreVector3::PackUnorm565()const
{
    ASSERT(this->IsUnorm())
    return (coreUint16(z * 31.0f) << 11u) |
           (coreUint16(y * 63.0f) <<  5u) |
           (coreUint16(x * 31.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into 5_6_5_rev packed uint16 */
constexpr coreUint16 coreVector3::PackSnorm565()const
{
    ASSERT(this->IsSnorm())
    return (coreUint16((z < 0.0f) ? (32.0f + z*16.0f) : (z*15.0f)) << 11u) |
           (coreUint16((y < 0.0f) ? (64.0f + y*32.0f) : (y*31.0f)) <<  5u) |
           (coreUint16((x < 0.0f) ? (32.0f + x*16.0f) : (x*15.0f)));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into (own) 10_11_11_rev packed uint32 */
constexpr coreUint32 coreVector3::PackUnorm011()const
{
    ASSERT(this->IsUnorm())
    return (F_TO_UI(z * 1023.0f) << 22u) |
           (F_TO_UI(y * 2047.0f) << 11u) |
           (F_TO_UI(x * 2047.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into (own) 10_11_11_rev packed uint32 */
constexpr coreUint32 coreVector3::PackSnorm011()const
{
    ASSERT(this->IsSnorm())
    return (F_TO_UI((z < 0.0f) ? (1024.0f + z* 512.0f) : (z* 511.0f)) << 22u) |
           (F_TO_UI((y < 0.0f) ? (2048.0f + y*1024.0f) : (y*1023.0f)) << 11u) |
           (F_TO_UI((x < 0.0f) ? (2048.0f + x*1024.0f) : (x*1023.0f)));
}


// ****************************************************************
/* compress regular normal-vector into octahedron normal-vector */
inline coreVector2 coreVector3::PackSnormOcta()const
{
    ASSERT(this->IsNormalized())

    coreVector2 A = coreVector2(x, y) / (ABS(x) + ABS(y) + ABS(z));

    if(z < 0.0f)
    {
        A = coreVector2((1.0f - ABS(A.y)) * SIGN(A.x),
                        (1.0f - ABS(A.x)) * SIGN(A.y));
    }

    return A;
}


// ****************************************************************
/* uncompress 5_6_5_rev packed uint16 into 0.0 to 1.0 vector */
constexpr coreVector3 coreVector3::UnpackUnorm565(const coreUint16 iNumber)
{
    return coreVector3(I_TO_F( iNumber         & 0x1Fu) * 3.225806452e-2f,
                       I_TO_F((iNumber >>  5u) & 0x3Fu) * 1.587301587e-2f,
                       I_TO_F((iNumber >> 11u) & 0x1Fu) * 3.225806452e-2f);
}


// ****************************************************************
/* uncompress 5_6_5_rev packed uint16 into -1.0 to 1.0 vector */
constexpr coreVector3 coreVector3::UnpackSnorm565(const coreUint16 iNumber)
{
    const coreVector3 A = coreVector3(I_TO_F( iNumber         & 0x1Fu),
                                      I_TO_F((iNumber >>  5u) & 0x3Fu),
                                      I_TO_F((iNumber >> 11u) & 0x1Fu));

    return coreVector3((A.x >= 16.0f) ? ((A.x - 32.0f)/16.0f) : (A.x/15.0f),
                       (A.y >= 32.0f) ? ((A.y - 64.0f)/32.0f) : (A.y/31.0f),
                       (A.z >= 16.0f) ? ((A.z - 32.0f)/16.0f) : (A.z/15.0f));
}


// ****************************************************************
/* uncompress (own) 10_11_11_rev packed uint32 into 0.0 to 1.0 vector */
constexpr coreVector3 coreVector3::UnpackUnorm011(const coreUint32 iNumber)
{
    return coreVector3(coreVector2(I_TO_F( iNumber         & 0x7FFu),
                                   I_TO_F((iNumber >> 11u) & 0x7FFu)) * 4.885197850e-4f,
                                   I_TO_F((iNumber >> 22u) & 0x3FFu)  * 9.775171065e-4f);
}


// ****************************************************************
/* uncompress (own) 10_11_11_rev packed uint32 into -1.0 to 1.0 vector */
constexpr coreVector3 coreVector3::UnpackSnorm011(const coreUint32 iNumber)
{
    const coreVector3 A = coreVector3(I_TO_F( iNumber         & 0x7FFu),
                                      I_TO_F((iNumber >> 11u) & 0x7FFu),
                                      I_TO_F((iNumber >> 22u) & 0x3FFu));

    return coreVector3((A.x >= 1024.0f) ? ((A.x - 2048.0f)/1024.0f) : (A.x/1023.0f),
                       (A.y >= 1024.0f) ? ((A.y - 2048.0f)/1024.0f) : (A.y/1023.0f),
                       (A.z >=  512.0f) ? ((A.z - 1024.0f)/ 512.0f) : (A.z/ 511.0f));
}


// ****************************************************************
/* uncompress octahedron normal-vector into regular normal-vector */
inline coreVector3 coreVector3::UnpackSnormOcta(const coreVector2 vVector)
{
    coreVector3 A = coreVector3(vVector, 1.0f - ABS(vVector.x) - ABS(vVector.y));

    if(A.z < 0.0f)
    {
        A.x += SIGN(A.x) * A.z;
        A.y += SIGN(A.y) * A.z;
    }

    return A.NormalizedUnsafe();
}


// ****************************************************************
/* convert RGB-color to HSV-color */
constexpr coreVector3 coreVector3::RgbToHsv()const
{
    const coreFloat R = x;
    const coreFloat G = y;
    const coreFloat B = z;

    const coreFloat v = this->Max();
    const coreFloat d = v - this->Min();

    if(!d) return coreVector3(0.0f, 0.0f, v);

    const coreFloat s = d * RCP(v);

    if(R == v) return coreVector3((0.0f + (G - B) * RCP(d)) / 6.0f, s, v);
    if(G == v) return coreVector3((2.0f + (B - R) * RCP(d)) / 6.0f, s, v);
               return coreVector3((4.0f + (R - G) * RCP(d)) / 6.0f, s, v);
}


// ****************************************************************
/* convert HSV-color to RGB-color */
constexpr coreVector3 coreVector3::HsvToRgb()const
{
    const coreFloat H = x * 6.0f;
    const coreFloat S = y;
    const coreFloat V = z;

    const coreInt32 h = F_TO_SI(H);

    const coreFloat s = V * S;
    const coreFloat t = s * (H - I_TO_F(h));
    const coreFloat p = V - s;

    switch(h)
    {
    case 1:  return coreVector3(V - t, V,     p);
    case 2:  return coreVector3(p,     V,     p + t);
    case 3:  return coreVector3(p,     V - t, V);
    case 4:  return coreVector3(p + t, p,     V);
    case 5:  return coreVector3(V,     p,     V - t);
    default: return coreVector3(V,     p + t, p);
    }
}


// ****************************************************************
/* calculate dot product */
constexpr coreFloat coreVector4::Dot(const coreVector4 v1, const coreVector4 v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
}


// ****************************************************************
/* generate random vector */
inline coreVector4 coreVector4::Rand(coreRand* OUTPUT pRand)
{
    return coreVector4(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized();
}

inline coreVector4 coreVector4::Rand(const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector4(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMax);
}

inline coreVector4 coreVector4::Rand(const coreFloat fMin, const coreFloat fMax, coreRand* OUTPUT pRand)
{
    return coreVector4(pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f,
                       pRand->Float(2.0f) - 1.0f).Normalized() * pRand->Float(fMin, fMax);
}

inline coreVector4 coreVector4::Rand(const coreFloat fMinX, const coreFloat fMaxX, const coreFloat fMinY, const coreFloat fMaxY, const coreFloat fMinZ, const coreFloat fMaxZ, const coreFloat fMinW, const coreFloat fMaxW, coreRand* OUTPUT pRand)
{
    return coreVector4(pRand->Float(fMinX, fMaxX),
                       pRand->Float(fMinY, fMaxY),
                       pRand->Float(fMinZ, fMaxZ),
                       pRand->Float(fMinW, fMaxW));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into 2_10_10_10_rev packed uint32 */
constexpr coreUint32 coreVector4::PackUnorm210()const
{
    ASSERT(this->IsUnorm())
    return (F_TO_UI(w *    3.0f) << 30u) |
           (F_TO_UI(z * 1023.0f) << 20u) |
           (F_TO_UI(y * 1023.0f) << 10u) |
           (F_TO_UI(x * 1023.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into 2_10_10_10_rev packed uint32 */
constexpr coreUint32 coreVector4::PackSnorm210()const
{
    ASSERT(this->IsSnorm())
    return (F_TO_UI((w < 0.0f) ? (   4.0f + w*  2.0f) : (w*  1.0f)) << 30u) |
           (F_TO_UI((z < 0.0f) ? (1024.0f + z*512.0f) : (z*511.0f)) << 20u) |
           (F_TO_UI((y < 0.0f) ? (1024.0f + y*512.0f) : (y*511.0f)) << 10u) |
           (F_TO_UI((x < 0.0f) ? (1024.0f + x*512.0f) : (x*511.0f)));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into WZYX packed uint32 */
constexpr coreUint32 coreVector4::PackUnorm4x8()const
{
    ASSERT(this->IsUnorm())
    return (F_TO_UI(w * 255.0f) << 24u) |
           (F_TO_UI(z * 255.0f) << 16u) |
           (F_TO_UI(y * 255.0f) <<  8u) |
           (F_TO_UI(x * 255.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into WZYX packed uint32 */
constexpr coreUint32 coreVector4::PackSnorm4x8()const
{
    ASSERT(this->IsSnorm())
    return (F_TO_UI((w < 0.0f) ? (256.0f + w*128.0f) : (w*127.0f)) << 24u) |
           (F_TO_UI((z < 0.0f) ? (256.0f + z*128.0f) : (z*127.0f)) << 16u) |
           (F_TO_UI((y < 0.0f) ? (256.0f + y*128.0f) : (y*127.0f)) <<  8u) |
           (F_TO_UI((x < 0.0f) ? (256.0f + x*128.0f) : (x*127.0f)));
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into WZYX packed uint64 */
constexpr coreUint64 coreVector4::PackUnorm4x16()const
{
    ASSERT(this->IsUnorm())
    return (coreUint64(w * 65535.0f) << 48u) |
           (coreUint64(z * 65535.0f) << 32u) |
           (coreUint64(y * 65535.0f) << 16u) |
           (coreUint64(x * 65535.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into WZYX packed uint64 */
constexpr coreUint64 coreVector4::PackSnorm4x16()const
{
    ASSERT(this->IsSnorm())
    return (coreUint64((w < 0.0f) ? (65536.0f + w*32768.0f) : (w*32767.0f)) << 48u) |
           (coreUint64((z < 0.0f) ? (65536.0f + z*32768.0f) : (z*32767.0f)) << 32u) |
           (coreUint64((y < 0.0f) ? (65536.0f + y*32768.0f) : (y*32767.0f)) << 16u) |
           (coreUint64((x < 0.0f) ? (65536.0f + x*32768.0f) : (x*32767.0f)));
}


// ****************************************************************
/* compress arbitrary vector into WZYX packed uint64 */
constexpr coreUint64 coreVector4::PackFloat4x16()const
{
#if defined(_CORE_SSE_) && defined(_CORE_MSVC_) && defined(_CORE_64BIT_)

    if(!std::is_constant_evaluated() && coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtsi128_si64(_mm_cvtps_ph(_mm_loadu_ps(&x), _MM_FROUND_CUR_DIRECTION));
    }

#endif

    // normal calculation
    return (coreUint64(coreMath::Float32To16(w)) << 48u) |
           (coreUint64(coreMath::Float32To16(z)) << 32u) |
           (coreUint64(coreMath::Float32To16(y)) << 16u) |
           (coreUint64(coreMath::Float32To16(x)));
}


// ****************************************************************
/* uncompress 2_10_10_10_rev packed uint32 into 0.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackUnorm210(const coreUint32 iNumber)
{
    return coreVector4(coreVector3(I_TO_F( iNumber         & 0x3FFu),
                                   I_TO_F((iNumber >> 10u) & 0x3FFu),
                                   I_TO_F((iNumber >> 20u) & 0x3FFu)) * 9.775171065e-4f,
                                   I_TO_F((iNumber >> 30u) & 0x003u)  * 3.333333333e-1f);
}


// ****************************************************************
/* uncompress 2_10_10_10_rev packed uint32 into -1.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackSnorm210(const coreUint32 iNumber)
{
    const coreVector4 A = coreVector4(I_TO_F( iNumber         & 0x3FFu),
                                      I_TO_F((iNumber >> 10u) & 0x3FFu),
                                      I_TO_F((iNumber >> 20u) & 0x3FFu),
                                      I_TO_F((iNumber >> 30u) & 0x003u));

    return coreVector4((A.x >= 512.0f) ? ((A.x - 1024.0f)/512.0f) : (A.x/511.0f),
                       (A.y >= 512.0f) ? ((A.y - 1024.0f)/512.0f) : (A.y/511.0f),
                       (A.z >= 512.0f) ? ((A.z - 1024.0f)/512.0f) : (A.z/511.0f),
                       (A.w >=   2.0f) ? ((A.w -    4.0f)/  2.0f) : (A.w/  1.0f));
}


// ****************************************************************
/* uncompress WZYX packed uint32 into 0.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackUnorm4x8(const coreUint32 iNumber)
{
    return coreVector4(I_TO_F( iNumber         & 0xFFu),
                       I_TO_F((iNumber >>  8u) & 0xFFu),
                       I_TO_F((iNumber >> 16u) & 0xFFu),
                       I_TO_F((iNumber >> 24u) & 0xFFu)) * 3.921568627e-3f;
}


// ****************************************************************
/* uncompress WZYX packed uint32 into -1.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackSnorm4x8(const coreUint32 iNumber)
{
    const coreVector4 A = coreVector4(I_TO_F( iNumber         & 0xFFu),
                                      I_TO_F((iNumber >>  8u) & 0xFFu),
                                      I_TO_F((iNumber >> 16u) & 0xFFu),
                                      I_TO_F((iNumber >> 24u) & 0xFFu));

    return coreVector4((A.x >= 128.0f) ? ((A.x - 256.0f)/128.0f) : (A.x/127.0f),
                       (A.y >= 128.0f) ? ((A.y - 256.0f)/128.0f) : (A.y/127.0f),
                       (A.z >= 128.0f) ? ((A.z - 256.0f)/128.0f) : (A.z/127.0f),
                       (A.w >= 128.0f) ? ((A.w - 256.0f)/128.0f) : (A.w/127.0f));
}


// ****************************************************************
/* uncompress WZYX packed uint64 into 0.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackUnorm4x16(const coreUint64 iNumber)
{
    return coreVector4(I_TO_F( iNumber         & 0xFFFFu),
                       I_TO_F((iNumber >> 16u) & 0xFFFFu),
                       I_TO_F((iNumber >> 32u) & 0xFFFFu),
                       I_TO_F((iNumber >> 48u) & 0xFFFFu)) * 1.525902190e-5f;
}


// ****************************************************************
/* uncompress WZYX packed uint64 into -1.0 to 1.0 vector */
constexpr coreVector4 coreVector4::UnpackSnorm4x16(const coreUint64 iNumber)
{
    const coreVector4 A = coreVector4(I_TO_F( iNumber         & 0xFFFFu),
                                      I_TO_F((iNumber >> 16u) & 0xFFFFu),
                                      I_TO_F((iNumber >> 32u) & 0xFFFFu),
                                      I_TO_F((iNumber >> 48u) & 0xFFFFu));

    return coreVector4((A.x >= 32768.0f) ? ((A.x - 65536.0f)/32768.0f) : (A.x/32767.0f),
                       (A.y >= 32768.0f) ? ((A.y - 65536.0f)/32768.0f) : (A.y/32767.0f),
                       (A.z >= 32768.0f) ? ((A.z - 65536.0f)/32768.0f) : (A.z/32767.0f),
                       (A.w >= 32768.0f) ? ((A.w - 65536.0f)/32768.0f) : (A.w/32767.0f));
}


// ****************************************************************
/* uncompress WZYX packed uint64 into arbitrary vector */
constexpr coreVector4 coreVector4::UnpackFloat4x16(const coreUint64 iNumber)
{
#if defined(_CORE_SSE_) && defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated() && coreCPUID::F16C())
    {
        // optimized calculation with F16C
        coreVector4 vOutput; _mm_storeu_ps(&vOutput.x, _mm_cvtph_ps(_mm_set_epi64x(0u, iNumber)));
        return vOutput;
    }

#endif

    // normal calculation
    return coreVector4(coreMath::Float16To32( iNumber         & 0xFFFFu),
                       coreMath::Float16To32((iNumber >> 16u) & 0xFFFFu),
                       coreMath::Float16To32((iNumber >> 32u) & 0xFFFFu),
                       coreMath::Float16To32((iNumber >> 48u) & 0xFFFFu));
}


// ****************************************************************
/* multiplicate two quaternions (Grassman product) */
constexpr coreVector4 coreVector4::QuatMultiply(const coreVector4 v1, const coreVector4 v2)
{
    return coreVector4(v1.x*v2.w + v1.w*v2.x + v1.y*v2.z - v1.z*v2.y,
                       v1.y*v2.w + v1.w*v2.y + v1.z*v2.x - v1.x*v2.z,
                       v1.z*v2.w + v1.w*v2.z + v1.x*v2.y - v1.y*v2.x,
                       v1.w*v2.w - v1.x*v2.x - v1.y*v2.y - v1.z*v2.z);
}


// ****************************************************************
/* spherical linear interpolate between two quaternions */
inline coreVector4 coreVector4::QuatSlerp(const coreVector4 v1, const coreVector4 v2, const coreFloat t)
{
    ASSERT(v1.IsNormalized() && v2.IsNormalized())

    const coreFloat   A = coreVector4::Dot(v1, v2);
    const coreFloat   B = ACOS(A) * t;
    const coreVector4 P = (v2 - v1 * A).Normalized();

    return v1 * COS(B) + P * SIN(B);
}


// ****************************************************************
/* apply quaternion-transformation to vector */
constexpr coreVector3 coreVector4::QuatApply(const coreVector3 v)const
{
    return v + 2.0f * coreVector3::Cross(this->xyz(), coreVector3::Cross(this->xyz(), v) + w * v);
}


// ****************************************************************
/* additional checks */
STATIC_ASSERT(std::is_trivial<coreVector2>::value)
STATIC_ASSERT(std::is_trivial<coreVector3>::value)
STATIC_ASSERT(std::is_trivial<coreVector4>::value)


#endif /* _CORE_GUARD_VECTOR_H_ */
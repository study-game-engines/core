//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATH_H_
#define _CORE_GUARD_MATH_H_

// TODO: SIN and COS with MacLaurin or Taylor series (no lookup-table, because memory access may be equally slow)
// TODO: check out _mm_ceil_ss and _mm_floor_ss (SSE4) ?
// TODO: FUNC_CONST on every function in this class ?
// TODO: add fmod, pow, integer-log


// ****************************************************************
/* math definitions */
#define CORE_MATH_PRECISION (0.001f)                               //!< default floating-point precision

#define PI (3.1415926535897932384626433832795f)                    //!< Archimedes' constant
#define EU (2.7182818284590452353602874713527f)                    //!< Euler's number

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   //!< convert degrees to radians
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   //!< convert radians to degrees
#define KM_TO_MI(x)   ((x) * 0.621371192237f)                      //!< convert kilometers to miles
#define MI_TO_KM(x)   ((x) * 1.609344000000f)                      //!< convert miles to kilometers

#define MIN   coreMath::Min
#define MAX   coreMath::Max
#define CLAMP coreMath::Clamp
#define SIGN  coreMath::Sign
#define ABS   coreMath::Abs
#define LERP  coreMath::Lerp
#define LERPS coreMath::LerpSmooth
#define LERPB coreMath::LerpBreak
#define TRUNC coreMath::Trunc
#define FRACT coreMath::Fract
#define SQRT  coreMath::Sqrt
#define RSQRT coreMath::Rsqrt
#define RCP   coreMath::Rcp
#define SIN   coreMath::Sin
#define COS   coreMath::Cos
#define TAN   coreMath::Tan
#define ASIN  coreMath::Asin
#define ACOS  coreMath::Acos
#define ATAN  coreMath::Atan
#define COT   coreMath::Cot
#define CEIL  coreMath::Ceil
#define FLOOR coreMath::Floor
#define ROUND coreMath::Round


// ****************************************************************
/* math utility collection */
class INTERFACE coreMath final
{
public:
    DISABLE_CONSTRUCTION(coreMath)

    /*! special operations */
    //! @{
    template <typename T, typename S, typename... A> static constexpr T Min  (const T& x, const S& y, A&&... vArgs) {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S, typename... A> static constexpr T Max  (const T& x, const S& y, A&&... vArgs) {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S>                static constexpr T Min  (const T& x, const S& y)               {return y ^ ((x ^ y) & -(x < y));}
    template <typename T, typename S>                static constexpr T Max  (const T& x, const S& y)               {return x ^ ((x ^ y) & -(x < y));}
    template <typename T, typename S, typename R>    static constexpr T Clamp(const T& x, const S& a, const R& b)   {return MIN(MAX(x, a), b);}
    template <typename T> static inline    T        Sign      (const T& x)                                          {return std::copysign(T(1), x);}
    template <typename T> static inline    T        Abs       (const T& x)                                          {return std::abs(x);}
    template <typename T> static constexpr T        Lerp      (const T& x, const T& y, const coreFloat s)           {return x + (y - x) * s;}
    template <typename T> static inline    T        LerpSmooth(const T& x, const T& y, const coreFloat s)           {return LERP(x, y, 0.5f - 0.5f * COS(s*PI));}
    template <typename T> static inline    T        LerpBreak (const T& x, const T& y, const coreFloat s)           {return LERP(x, y, SIN(s*PI*0.5f));}
    template <typename T> static constexpr coreBool InRange   (const T& x, const T& c, const T& r)                  {return (x - c)*(x - c) <= r*r;}
    template <typename T> static constexpr coreBool IsPOT     (const T& x)                                          {return !(x & (x - T(1)));}
    //! @}

    /*! elementary operations */
    //! @{
    template <coreUintW iBase> static inline coreFloat Log(const coreFloat fInput) {return std::log(fInput) / std::log(I_TO_F(iBase));}
    static inline coreFloat Trunc(const coreFloat fInput)                          {return std::trunc(fInput);}
    static inline coreFloat Fract(const coreFloat fInput)                          {return fInput - TRUNC(fInput);}
    static inline coreFloat Sqrt (const coreFloat fInput)                          {return fInput ? (fInput * RSQRT(fInput)) : 0.0f;}
    static inline coreFloat Rsqrt(const coreFloat fInput);
    static inline coreFloat Rcp  (const coreFloat fInput);
    //! @}

    /*! trigonometric operations */
    //! @{
    static inline coreFloat Sin (const coreFloat fInput) {return std::sin (fInput);}
    static inline coreFloat Cos (const coreFloat fInput) {return std::cos (fInput);}
    static inline coreFloat Tan (const coreFloat fInput) {return std::tan (fInput);}
    static inline coreFloat Asin(const coreFloat fInput) {return std::asin(fInput);}
    static inline coreFloat Acos(const coreFloat fInput) {return std::acos(fInput);}
    static inline coreFloat Atan(const coreFloat fInput) {return std::atan(fInput);}
    static inline coreFloat Cot (const coreFloat fInput) {return TAN(PI*0.5f - fInput);}
    //! @}

    /*! rounding operations */
    //! @{
    template <coreUintW iByte, typename T> static inline T CeilAlign (const T& tInput) {const T  k = tInput - 1; return k - (k % iByte) + iByte;}
    template <coreUintW iByte, typename T> static inline T FloorAlign(const T& tInput) {const T& k = tInput;     return k - (k % iByte);}
    template                  <typename T> static inline T CeilPOT   (const T& tInput) {T k = 2; while(k <  tInput) k <<= 1; return k;}
    template                  <typename T> static inline T FloorPOT  (const T& tInput) {T k = 2; while(k <= tInput) k <<= 1; return k >> 1;}
    static inline coreFloat Ceil (const coreFloat fInput)                              {return std::ceil (fInput);}
    static inline coreFloat Floor(const coreFloat fInput)                              {return std::floor(fInput);}
    static inline coreFloat Round(const coreFloat fInput)                              {return std::round(fInput);}
    //! @}

    /*! bit operations */
    //! @{
    static inline coreUint32 PopCount  (coreUint32 iInput);
    static inline coreUint32 BitScanFwd(coreUint32 iInput);
    static inline coreUint32 BitScanRev(coreUint32 iInput);
    //! @}

    /*! converting operations */
    //! @{
    static inline coreUint16 Float32to16(const coreFloat  fInput);
    static inline coreFloat  Float16to32(const coreUint16 iInput);
    //! @}

    /*! miscellaneous functions */
    //! @{
    static inline void DisableDenormals();
    //! @}
};


// ****************************************************************
/* template specializations */
#if defined(_CORE_SSE_)

    // optimized calculation with SSE
    template <> inline coreFloat coreMath::Min  (const coreFloat& x, const coreFloat& y)                     {return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(x), _mm_set_ss(y)));}
    template <> inline coreFloat coreMath::Max  (const coreFloat& x, const coreFloat& y)                     {return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(x), _mm_set_ss(y)));}
    template <> inline coreFloat coreMath::Clamp(const coreFloat& x, const coreFloat& a, const coreFloat& b) {return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(_mm_set_ss(x), _mm_set_ss(a)), _mm_set_ss(b)));}

#else

    // normal calculation
    template <> constexpr coreFloat coreMath::Min(const coreFloat& x, const coreFloat& y) {return (x < y) ? x : y;}
    template <> constexpr coreFloat coreMath::Max(const coreFloat& x, const coreFloat& y) {return (x > y) ? x : y;}

#endif

template <> inline coreFloat coreMath::Log< 2u>(const coreFloat fInput) {return std::log2 (fInput);}
template <> inline coreFloat coreMath::Log<10u>(const coreFloat fInput) {return std::log10(fInput);}


// ****************************************************************
/* calculate inverse square root */
inline coreFloat coreMath::Rsqrt(const coreFloat fInput)
{
    ASSERT(fInput > 0.0f)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(fInput)));

#else

    // normal calculation
    return 1.0f / std::sqrt(fInput);

#endif
}


// ****************************************************************
/* calculate approximate reciprocal */
inline coreFloat coreMath::Rcp(const coreFloat fInput)
{
    ASSERT(fInput)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(fInput)));

#else

    // normal calculation
    return 1.0f / fInput;

#endif
}


// ****************************************************************
/* count the number of one-bits (population count) */
inline coreUint32 coreMath::PopCount(coreUint32 iInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::POPCNT())
    {
        // optimized calculation with POPCNT
        return __popcnt(iInput);
    }

#endif

    // normal calculation
    iInput = (iInput)               - ((iInput >> 1u) & 0x55555555u);
    iInput = (iInput & 0x33333333u) + ((iInput >> 2u) & 0x33333333u);
    return (((iInput + (iInput >> 4u)) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}


// ****************************************************************
/* get index of the least significant one-bit */
inline coreUint32 coreMath::BitScanFwd(coreUint32 iInput)
{
    if(!iInput) return 32u;

#if defined(_CORE_MSVC_)

    // calculation with MSVC intrinsic
    DWORD iOutput; _BitScanForward(&iOutput, iInput);
    return iOutput;

#else

    // calculation with GCC/Clang intrinsic
    return __builtin_ffs(iInput) - 1u;

#endif
}


// ****************************************************************
/* get index of the most significant one-bit */
inline coreUint32 coreMath::BitScanRev(coreUint32 iInput)
{
    if(!iInput) return 32u;

#if defined(_CORE_MSVC_)

    // calculation with MSVC intrinsic
    DWORD iOutput; _BitScanReverse(&iOutput, iInput);
    return iOutput;

#else

    // calculation with GCC/Clang intrinsic
    return 31u - __builtin_clz(iInput);

#endif
}


// ****************************************************************
/* convert single-precision float into half-precision */
inline coreUint16 coreMath::Float32to16(const coreFloat fInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(fInput), 0));
    }

#endif

    // normal calculation
    const coreUint32 A = *r_cast<const coreUint32*>(&fInput);
    return ((A & 0x7FFFFFFFu) > 0x38000000u) ? ((((A & 0x7FFFFFFFu) >> 13u) - 0x0001C000u) |
                                                 ((A & 0x80000000u) >> 16u)) & 0xFFFFu : 0u;
};


// ****************************************************************
/* convert half-precision float into single-precision */
inline coreFloat coreMath::Float16to32(const coreUint16 iInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(iInput)));
    }

#endif

    // normal calculation
    const coreUint32 A = (iInput & 0x7C00u) ? (((coreUint32(iInput & 0x7FFFu) << 13u) + 0x38000000u) |
                                                (coreUint32(iInput & 0x8000u) << 16u)) : 0u;
    return *r_cast<const coreFloat*>(&A);
};


// ****************************************************************
/* disable denormals in the MXCSR control register (per thread) */
inline void coreMath::DisableDenormals()
{
#if defined(_CORE_SSE_)

    // prevent denormal calculation results
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    // prevent denormal instruction inputs
    if(coreCPUID::SSE3()) _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

#endif
}


#endif /* _CORE_GUARD_MATH_H_ */
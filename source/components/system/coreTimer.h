//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TIMER_H_
#define _CORE_GUARD_TIMER_H_


// ****************************************************************
// timer class
class coreTimer final
{
private:
    float m_fCurrent;      //!< current value
    float m_fEnd;          //!< target value
    float m_fSpeed;        //!< speed factor of the timer

    coreUint m_iMaxLoop;   //!< max number of loops (0 = infinite)
    coreUint m_iCurLoop;   //!< current number of loops

    int m_iTimeID;         //!< ID of the used frame time
    bool m_bStatus;        //!< current status


public:
    constexpr_func coreTimer()noexcept;
    constexpr_func coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoops)noexcept;

    //! update the timer
    //! @{
    bool Update(const float& fModifier);
    //! @}

    //! control the timer
    //! @{
    inline void Play(const bool& bReset) {if(bReset) this->Reset(); m_bStatus = true;}
    inline void Pause()                  {m_bStatus = false;}
    inline void Reset()                  {m_fCurrent = 0.0f; m_iCurLoop = 0;}
    inline void Stop()                   {this->Pause(); this->Reset();}
    //! @}

    //! set all basic object properties
    //! @{
    inline void Set(const float& fEnd, const float& fSpeed, const coreUint& iLoops) {m_fEnd = fEnd; m_fSpeed = fSpeed; m_iMaxLoop = iLoops;}
    //! @}

    //! set object properties
    //! @{
    inline void SetCurrent(const float& fCurrent)    {m_fCurrent = fCurrent;}
    inline void SetEnd(const float& fEnd)            {m_fEnd     = fEnd;}
    inline void SetSpeed(const float& fSpeed)        {m_fSpeed   = fSpeed;}
    inline void SetMaxLoop(const coreUint& iMaxLoop) {m_iMaxLoop = iMaxLoop;}
    inline void SetCurLoop(const coreUint& iCurLoop) {m_iCurLoop = iCurLoop;}
    inline void SetTimeID(const int& iTimeID)        {m_iTimeID  = iTimeID;}
    //! @}

    //! get object properties
    //! @{
    inline float GetCurrent(const bool& bReverse)const {return bReverse ? m_fEnd-m_fCurrent : m_fCurrent;}
    inline const float& GetEnd()const                  {return m_fEnd;}
    inline const float& GetSpeed()const                {return m_fSpeed;}
    inline const coreUint& GetMaxLoop()const           {return m_iMaxLoop;}
    inline const coreUint& GetCurLoop()const           {return m_iCurLoop;}
    inline const int& GetTimeID()const                 {return m_iTimeID;}
    inline const bool& GetStatus()const                {return m_bStatus;}
    //! @}
};


// ****************************************************************
// simple flowing value class
class coreFlow final
{
private:
    float m_fCurrent;   //!< current value


public:
    constexpr_func coreFlow()noexcept : m_fCurrent(0.0f) {}
    explicit constexpr_func coreFlow(const float& fValue)noexcept : m_fCurrent(fValue) {}

    //! access current value
    //! @{
    inline operator float& ()noexcept                 {return m_fCurrent;}
    inline operator const float& ()const noexcept     {return m_fCurrent;}
    inline float& operator = (const float& f)noexcept {m_fCurrent = f; return m_fCurrent;}
    //! @}

    //! update the flowing value
    //! @{
    inline void Update(const float& fSpeed)noexcept                     {m_fCurrent += fSpeed * Core::System->GetTime();}
    inline void Update(const float& fSpeed, const int& iTimeID)noexcept {m_fCurrent += fSpeed * Core::System->GetTime(iTimeID);}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreTimer::coreTimer()noexcept
: coreTimer (1.0f, 1.0f, 0)
{
}

constexpr_func coreTimer::coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoops)noexcept
: m_fCurrent (0.0f)
, m_fEnd     (fEnd)
, m_fSpeed   (fSpeed)
, m_iMaxLoop (iLoops)
, m_iCurLoop (0)
, m_iTimeID  (-1)
, m_bStatus  (false)
{
}


#endif // _CORE_GUARD_TIMER_H_
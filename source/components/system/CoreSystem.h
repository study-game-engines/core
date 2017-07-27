//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYSTEM_H_
#define _CORE_GUARD_SYSTEM_H_

// TODO: fullscreen enumeration
// TODO: available resolutions are currently only retrieved from the first display (though some drivers may "merge" displays)
// TODO: handle display hot-plugging


// ****************************************************************
// system definitions
#define CORE_SYSTEM_TIMES (4u)   //!< number of dynamic frame times


// ****************************************************************
// main system component
class CoreSystem final
{
private:
    //! display structure
    struct coreDisplay final
    {
        std::vector<coreVector2> avAvailableRes;   //!< all available screen resolutions
        coreVector2              vDesktopRes;      //!< desktop resolution
    };


private:
    SDL_Window* m_pWindow;                         //!< SDL main window object

    std::vector<coreDisplay> m_aDisplayData;       //!< all available displays

    coreUint8   m_iDisplayIndex;                   //!< primary display index
    coreVector2 m_vResolution;                     //!< width and height of the window
    coreUint8   m_iFullscreen;                     //!< fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    coreBool m_bMinimized;                         //!< window/application was minimized
    coreBool m_bTerminated;                        //!< application will be terminated

    coreDouble m_dTotalTime;                       //!< total time since start of the application
    coreDouble m_dTotalTimeBefore;                 //!< total time of the previous frame
    coreFloat  m_fLastTime;                        //!< smoothed last frame time
    coreFloat  m_afTime     [CORE_SYSTEM_TIMES];   //!< dynamic frame times
    coreFloat  m_afTimeSpeed[CORE_SYSTEM_TIMES];   //!< speed factor for the dynamic frame times

    coreUint32 m_iCurFrame;                        //!< current frame number since start of the application
    coreUint8  m_iSkipFrame;                       //!< skip frame status

    coreDouble m_dPerfFrequency;                   //!< high-precision time coefficient
    coreUint64 m_iPerfTime;                        //!< high-precision time value


private:
    CoreSystem()noexcept;
    ~CoreSystem();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreSystem)

    //! control window
    //! @{
    void SetWindowTitle(const coreChar* pcTitle);
    void SetWindowIcon (const coreChar* pcPath);
    //! @}

    //! control time
    //! @{
    inline void SetTimeSpeed(const coreUintW iID, const coreFloat fTimeSpeed) {ASSERT(iID < CORE_SYSTEM_TIMES) m_afTimeSpeed[iID] = fTimeSpeed;}
    inline void SkipFrame()                                                   {m_iSkipFrame = 1u;}
    //! @}

    //! terminate the application
    //! @{
    inline void Quit() {m_bTerminated = true;}
    //! @}

    //! get component properties
    //! @{
    inline       SDL_Window*  GetWindow         ()const                    {return m_pWindow;}
    inline const coreDisplay& GetDisplayData    ()const                    {return m_aDisplayData[m_iDisplayIndex];}
    inline const coreDisplay& GetDisplayData    (const coreUintW iID)const {ASSERT(iID < m_aDisplayData.size()) return m_aDisplayData[iID];}
    inline       coreUintW    GetDisplayCount   ()const                    {return m_aDisplayData.size();}
    inline const coreUint8&   GetDisplayIndex   ()const                    {return m_iDisplayIndex;}
    inline const coreVector2& GetResolution     ()const                    {return m_vResolution;}
    inline const coreUint8&   GetFullscreen     ()const                    {return m_iFullscreen;}
    inline const coreBool&    GetMinimized      ()const                    {return m_bMinimized;}
    inline const coreDouble&  GetTotalTime      ()const                    {return m_dTotalTime;}
    inline const coreDouble&  GetTotalTimeBefore()const                    {return m_dTotalTimeBefore;}
    inline const coreFloat&   GetTime           ()const                    {return m_fLastTime;}
    inline const coreFloat&   GetTime           (const coreInt8  iID)const {ASSERT(iID < coreInt8(CORE_SYSTEM_TIMES)) return (iID >= 0) ? m_afTime[iID] : m_fLastTime;}
    inline const coreFloat&   GetTimeSpeed      (const coreUintW iID)const {ASSERT(iID <          CORE_SYSTEM_TIMES)  return m_afTimeSpeed[iID];}
    inline const coreUint32&  GetCurFrame       ()const                    {return m_iCurFrame;}
    inline const coreDouble&  GetPerfFrequency  ()const                    {return m_dPerfFrequency;}
    inline const coreUint64&  GetPerfTime       ()const                    {return m_iPerfTime;}
    //! @}


private:
    //! update the window event system
    //! @{
    coreBool __UpdateEvents();
    //! @}

    //! update the high-precision time
    //! @{
    void __UpdateTime();
    //! @}
};


#endif // _CORE_GUARD_SYSTEM_H_
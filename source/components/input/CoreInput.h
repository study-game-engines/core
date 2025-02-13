///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_INPUT_H_
#define _CORE_GUARD_INPUT_H_

// TODO 5: touch-related functions empty on non-mobile
// TODO 3: remove the 1-frame delay when using the Set* interface from game-code
// TODO 3: pseudo-joystick should accumulate all input from other devices
// TODO 2: press and release within the same frame gets ignored, but it is assumed that both cannot happen in the same loop
// TODO 5: <old comment style>
// TODO 3: mouse button 0 is not used
// TODO 2: in browser (Firefox?), xbox 360 controller: right stick is axis 3+4 (instead of 2+3), and d-pad is converted to another axis-pair 6+7


// ****************************************************************
/* input definitions */
#define __CORE_INPUT_PRESS(x)       {SET_BIT(x, CORE_INPUT_RELEASE, false)                       SET_BIT(x, CORE_INPUT_PRESS, !HAS_BIT(x, CORE_INPUT_HOLD)) SET_BIT(x, CORE_INPUT_HOLD, true)}
#define __CORE_INPUT_RELEASE(x)     {SET_BIT(x, CORE_INPUT_RELEASE, HAS_BIT(x, CORE_INPUT_HOLD)) SET_BIT(x, CORE_INPUT_PRESS, false)                        SET_BIT(x, CORE_INPUT_HOLD, false)}
#define __CORE_INPUT_COUNT(x,c)     {if(x) for(coreUintW j = 0u; j < CORE_INPUT_TYPES; ++j) if(HAS_BIT(x, j)) {++(c)[j]; ADD_BIT(m_iAnyButton, j)}}
#define __CORE_INPUT_JOYSTICK(i)    (m_aJoystick[MIN(i, m_aJoystick.size() - 1u)])

#define CORE_INPUT_BUTTONS_KEYBOARD (287u)   // number of handled keyboard buttons (#SDL_NUM_SCANCODES)
#define CORE_INPUT_BUTTONS_MOUSE    (16u)    // number of handled mouse buttons
#define CORE_INPUT_BUTTONS_JOYSTICK (32u)    // number of handled joystick buttons
#define CORE_INPUT_AXIS             (6u)     // number of handled joystick axis
#define CORE_INPUT_DIRECTIONS       (4u)     // number of handled joystick (hat) directions
#define CORE_INPUT_TYPES            (4u)     // number of different status types

#if defined(_CORE_MOBILE_)
    #define CORE_INPUT_FINGERS      (5u)     // maximum number of simultaneous fingers
#else
    #define CORE_INPUT_FINGERS      (1u)
#endif

#define CORE_INPUT_INVALID_KEYBOARD (CORE_INPUT_KEY(UNKNOWN))
#define CORE_INPUT_INVALID_MOUSE    (0xFFu)
#define CORE_INPUT_INVALID_JOYSTICK (0xFFu)

#define CORE_INPUT_KEY(k)           (SDL_SCANCODE_ ## k)
#define CORE_INPUT_CHAR(c)          (SDLK_         ## c)

using coreInputKey  = SDL_Scancode;
using coreInputChar = SDL_Keycode;

enum coreInputButton : coreUint8
{
    CORE_INPUT_LEFT   = SDL_BUTTON_LEFT,
    CORE_INPUT_MIDDLE = SDL_BUTTON_MIDDLE,
    CORE_INPUT_RIGHT  = SDL_BUTTON_RIGHT
};

enum coreInputDir : coreUint8
{
    CORE_INPUT_DIR_LEFT  = 0u,
    CORE_INPUT_DIR_RIGHT = 1u,
    CORE_INPUT_DIR_DOWN  = 2u,
    CORE_INPUT_DIR_UP    = 3u
};

enum coreInputType : coreUint8
{
    CORE_INPUT_DATA    = 0u,
    CORE_INPUT_HOLD    = 1u,
    CORE_INPUT_PRESS   = 2u,
    CORE_INPUT_RELEASE = 3u
};


// ****************************************************************
/* main input component */
class CoreInput final
{
private:
    /* keyboard structure */
    struct coreKeyboard final
    {
        coreUint8    aiButton[CORE_INPUT_BUTTONS_KEYBOARD];   // status of the keyboard buttons
        coreUint16   aiCount [CORE_INPUT_TYPES];              // number of keyboard buttons with same status
        coreInputKey iLast;                                   // last pressed keyboard button
        coreChar     acChar[5];                               // current text-input character (UTF-8, null-terminated)

        STATIC_ASSERT(sizeof(acChar) > sizeof(coreInputChar))
    };

    /* mouse structure */
    struct coreMouse final
    {
        coreUint8   aiButton[CORE_INPUT_BUTTONS_MOUSE];   // status of the mouse buttons
        coreUint8   aiCount [CORE_INPUT_TYPES];           // number of mouse buttons with same status
        coreUint8   iLast;                                // last pressed mouse button
        coreVector2 vPosition;                            // absolute position of the mouse cursor
        coreVector3 vRelative;                            // relative movement of the mouse cursor
    };

    /* joystick structure */
    struct coreJoystick final
    {
        SDL_GameController* pController;                   // game controller handle
        SDL_Joystick*       pJoystick;                     // joystick device handle

        coreUint8 aiButton[CORE_INPUT_BUTTONS_JOYSTICK];   // status of the joystick buttons
        coreUint8 aiCount [CORE_INPUT_TYPES];              // number of joystick buttons with same status
        coreUint8 iLast;                                   // last pressed joystick button
        coreUint8 aiHat     [CORE_INPUT_DIRECTIONS];       // status of the joystick hat
        coreFloat afRelative[CORE_INPUT_AXIS];             // relative movement of the joystick axis (0|1 = left stick, 2|3 = right stick, 4 = left shoulder, 5 = right shoulder)
    };

    /* touch structure */
    struct coreTouch final
    {
        coreUint8   iButton;     // status of the finger
        coreVector2 vPosition;   // absolute position of the finger
        coreVector2 vRelative;   // relative movement of the finger
        coreFloat   fPressure;   // current quantity of pressure applied
    };


private:
    coreKeyboard           m_Keyboard;              // main keyboard object
    coreMouse              m_Mouse;                 // main mouse object
    coreList<coreJoystick> m_aJoystick;             // list with joystick objects

    coreTouch m_aTouch      [CORE_INPUT_FINGERS];   // array with touch objects
    coreUint8 m_aiTouchCount[CORE_INPUT_TYPES];     // number of fingers with the same status

    SDL_Cursor* m_pCursor;                          // hardware mouse cursor
    coreBool    m_bCursorVisible;                   // status of the mouse cursor

    coreUint8 m_iAnyButton;                         // status of any available button (keyboard, mouse, joystick, touch)


private:
    CoreInput()noexcept;
    ~CoreInput();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreInput)

    /* control the mouse cursor */
    void SetCursor (const coreChar* pcPath);
    void ShowCursor(const coreBool  bStatus);
    inline const coreBool& IsCursorVisible()const {return m_bCursorVisible;}

    /* use controls from other devices */
    void UseMouseWithKeyboard(const coreInputKey iLeft, const coreInputKey iRight, const coreInputKey iDown, const coreInputKey iUp, const coreInputKey iButton1, const coreInputKey iButton2, const coreFloat fSpeed);
    void UseMouseWithJoystick(const coreUintW iIndex, const coreUint8 iButton1, const coreUint8 iButton2, const coreFloat fSpeed);
    void ForwardHatToStick   (const coreUintW iIndex);

    /* get joystick data */
    inline const coreChar* GetJoystickName     (const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pController ? SDL_GameControllerName     (__CORE_INPUT_JOYSTICK(iIndex).pController) : __CORE_INPUT_JOYSTICK(iIndex).pJoystick ? SDL_JoystickName     (__CORE_INPUT_JOYSTICK(iIndex).pJoystick) : "";}
    inline const coreChar* GetJoystickSerial   (const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pController ? SDL_GameControllerGetSerial(__CORE_INPUT_JOYSTICK(iIndex).pController) : __CORE_INPUT_JOYSTICK(iIndex).pJoystick ? SDL_JoystickGetSerial(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) : "";}
    inline const coreChar* GetJoystickPath     (const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pController ? SDL_GameControllerPath     (__CORE_INPUT_JOYSTICK(iIndex).pController) : __CORE_INPUT_JOYSTICK(iIndex).pJoystick ? SDL_JoystickPath     (__CORE_INPUT_JOYSTICK(iIndex).pJoystick) : "";}
    inline const coreChar* GetJoystickGUID     (const coreUintW iIndex)const {if(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) {static coreChar s_acGUID[64]; SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(__CORE_INPUT_JOYSTICK(iIndex).pJoystick), s_acGUID, ARRAY_SIZE(s_acGUID)); return s_acGUID;} return "";}
    inline coreBool        GetJoystickHasRumble(const coreUintW iIndex)const {return (SDL_JoystickHasRumble(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) != SDL_FALSE);}
    inline coreBool        GetJoystickHasLED   (const coreUintW iIndex)const {return (SDL_JoystickHasLED   (__CORE_INPUT_JOYSTICK(iIndex).pJoystick) != SDL_FALSE);}
    inline coreUintW       GetJoystickNum      ()const                       {return m_aJoystick.size() - 1u;}

    /* process input events */
    coreBool ProcessEvent(const SDL_Event& oEvent);

    /* access keyboard input */
    inline void            SetKeyboardButton  (const coreInputKey  iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; SET_BIT(m_Keyboard.aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) m_Keyboard.iLast = iButton;}
    inline void            SetKeyboardChar    (const coreInputChar iChar)                                   {(*r_cast<coreInputChar*>(m_Keyboard.acChar)) = iChar;}
    inline void            SetKeyboardCharUTF8(const coreChar*     pcChar)                                  {coreData::StrCopy(m_Keyboard.acChar, ARRAY_SIZE(m_Keyboard.acChar), pcChar);}
    inline coreBool        GetKeyboardButton  (const coreInputKey  iButton, const coreInputType eType)const {ASSERT(iButton < CORE_INPUT_BUTTONS_KEYBOARD) return HAS_BIT(m_Keyboard.aiButton[iButton], eType);}
    inline coreInputChar   GetKeyboardChar    ()const                                                       {return (*r_cast<const coreInputChar*>(m_Keyboard.acChar));}
    inline const coreChar* GetKeyboardCharUTF8()const                                                       {return m_Keyboard.acChar;}

    /* access mouse input */
    inline void               SetMouseButton  (const coreUint8 iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE) return; SET_BIT(m_Mouse.aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) m_Mouse.iLast = iButton;}
    inline void               SetMousePosition(const coreVector2 vPosition)                             {m_Mouse.vPosition   = vPosition;}
    inline void               SetMouseRelative(const coreVector2 vRelative)                             {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void               SetMouseWheel   (const coreFloat fValue)                                  {m_Mouse.vRelative.z = fValue;}
    inline coreBool           GetMouseButton  (const coreUint8 iButton, const coreInputType eType)const {ASSERT(iButton < CORE_INPUT_BUTTONS_MOUSE) return HAS_BIT(m_Mouse.aiButton[iButton], eType);}
    inline const coreVector2& GetMousePosition()const                                                   {return m_Mouse.vPosition;}
    inline const coreVector3& GetMouseRelative()const                                                   {return m_Mouse.vRelative;}
    inline const coreFloat&   GetMouseWheel   ()const                                                   {return m_Mouse.vRelative.z;}

    /* access joystick input */
    inline void               SetJoystickButton   (const coreUintW iIndex, const coreUint8 iButton, const coreBool bStatus)                                         {WARN_IF(iButton    >= CORE_INPUT_BUTTONS_JOYSTICK) return; SET_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) __CORE_INPUT_JOYSTICK(iIndex).iLast = iButton;}
    inline void               SetJoystickHat      (const coreUintW iIndex, const coreInputDir eDirection, const coreBool bStatus)                                   {WARN_IF(eDirection >= CORE_INPUT_DIRECTIONS)       return; SET_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiHat[eDirection], CORE_INPUT_DATA, bStatus)}
    inline void               SetJoystickRelative (const coreUintW iIndex, const coreUint8 iAxis, const coreFloat fValue)                                           {WARN_IF(iAxis      >= CORE_INPUT_AXIS)             return; __CORE_INPUT_JOYSTICK(iIndex).afRelative[iAxis] = fValue;}
    inline coreBool           GetJoystickButton   (const coreUintW iIndex, const coreUint8 iButton, const coreInputType eType)const                                 {ASSERT(iButton    < CORE_INPUT_BUTTONS_JOYSTICK) return HAS_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], eType);}
    inline coreBool           GetJoystickHat      (const coreUintW iIndex, const coreInputDir eDirection, const coreInputType eType)const                           {ASSERT(eDirection < CORE_INPUT_DIRECTIONS)       return HAS_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiHat[eDirection], eType);}
    inline const coreFloat&   GetJoystickRelative (const coreUintW iIndex, const coreUint8 iAxis)const                                                              {ASSERT(iAxis      < CORE_INPUT_AXIS)             return __CORE_INPUT_JOYSTICK(iIndex).afRelative[iAxis];}
    inline const coreVector2& GetJoystickRelativeL(const coreUintW iIndex)const                                                                                     {return r_cast<const coreVector2&>(__CORE_INPUT_JOYSTICK(iIndex).afRelative[0]);}
    inline const coreVector2& GetJoystickRelativeR(const coreUintW iIndex)const                                                                                     {return r_cast<const coreVector2&>(__CORE_INPUT_JOYSTICK(iIndex).afRelative[2]);}
    inline void               JoystickRumble      (const coreUintW iIndex, const coreFloat fStrengthLow, const coreFloat fStrengthHigh, const coreUint32 iLengthMs) {ASSERT((fStrengthLow >= 0.0f) && (fStrengthLow <= 1.0f) && (fStrengthHigh >= 0.0f) && (fStrengthHigh <= 1.0f)) if(Core::Config->GetBool(CORE_CONFIG_INPUT_RUMBLE)) SDL_JoystickRumble(__CORE_INPUT_JOYSTICK(iIndex).pJoystick, F_TO_UI(fStrengthLow * 65535.0f), F_TO_UI(fStrengthHigh * 65535.0f), iLengthMs);}
    inline void               JoystickChangeLED   (const coreUintW iIndex, const coreVector3 vColor)                                                                {ASSERT((vColor >= coreVector3(0.0f,0.0f,0.0f)) && (vColor <= coreVector3(1.0f,1.0f,1.0f))) SDL_JoystickSetLED(__CORE_INPUT_JOYSTICK(iIndex).pJoystick, F_TO_UI(vColor.x * 255.0f), F_TO_UI(vColor.y * 255.0f), F_TO_UI(vColor.z * 255.0f));}

    /* access touch input */
    inline void                SetTouchButton  (const coreUintW iIndex, const coreBool bStatus)         {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; SET_BIT(m_aTouch[iIndex].iButton, CORE_INPUT_DATA, bStatus)}
    inline void                SetTouchPosition(const coreUintW iIndex, const coreVector2 vPosition)    {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vPosition = vPosition;}
    inline void                SetTouchRelative(const coreUintW iIndex, const coreVector2 vRelative)    {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vRelative = vRelative;}
    inline void                SetTouchPressure(const coreUintW iIndex, const coreFloat fPressure)      {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].fPressure = fPressure;}
    inline coreBool            GetTouchButton  (const coreUintW iIndex, const coreInputType eType)const {ASSERT(iIndex < CORE_INPUT_FINGERS) return HAS_BIT(m_aTouch[iIndex].iButton, eType);}
    inline const coreVector2&  GetTouchPosition(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].vPosition;}
    inline const coreVector2&  GetTouchRelative(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].vRelative;}
    inline const coreFloat&    GetTouchPressure(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].fPressure;}
    template <typename F> void ForEachFinger   (const coreInputType eType, F&& nFunction);   // [](const coreUintW iFingerIndex) -> void

    /* get number of input buttons with same status */
    inline const coreUint16& GetCountKeyboard(const coreInputType eType)const                         {ASSERT(eType < CORE_INPUT_TYPES) return m_Keyboard.aiCount[eType];}
    inline const coreUint8&  GetCountMouse   (const coreInputType eType)const                         {ASSERT(eType < CORE_INPUT_TYPES) return m_Mouse.aiCount[eType];}
    inline const coreUint8&  GetCountJoystick(const coreUintW iIndex, const coreInputType eType)const {ASSERT(eType < CORE_INPUT_TYPES) return __CORE_INPUT_JOYSTICK(iIndex).aiCount[eType];}
    inline const coreUint8&  GetCountTouch   (const coreInputType eType)const                         {ASSERT(eType < CORE_INPUT_TYPES) return m_aiTouchCount[eType];}

    /* get last pressed input button */
    inline const coreInputKey& GetLastKeyboard()const                       {return m_Keyboard.iLast;}
    inline const coreUint8&    GetLastMouse   ()const                       {return m_Mouse   .iLast;}
    inline const coreUint8&    GetLastJoystick(const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).iLast;}

    /* get status of any available button */
    inline coreBool GetAnyButton(const coreInputType eType)const {return HAS_BIT(m_iAnyButton, eType);}

    /* clear status of input buttons */
    inline void ClearKeyboardButton   (const coreInputKey iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; m_Keyboard.aiButton[iButton]                    = 0u;}
    inline void ClearMouseButton      (const coreUint8    iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE)    return; m_Mouse   .aiButton[iButton]                    = 0u;}
    inline void ClearJoystickButton   (const coreUintW iIndex, const coreUint8 iButton) {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; __CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton] = 0u;}
    inline void ClearKeyboardButtonAll()                                                {std::memset(m_Keyboard.aiButton,                    0, sizeof(m_Keyboard.aiButton));}
    inline void ClearMouseButtonAll   ()                                                {std::memset(m_Mouse   .aiButton,                    0, sizeof(m_Mouse   .aiButton));}
    inline void ClearJoystickButtonAll(const coreUintW iIndex)                          {std::memset(__CORE_INPUT_JOYSTICK(iIndex).aiButton, 0, sizeof(__CORE_INPUT_JOYSTICK(iIndex).aiButton));}
    inline void ClearTouchButtonAll   ()                                                {for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i) m_aTouch[i].iButton = 0u;}
    inline void ClearAnyButton        ()                                                {m_iAnyButton = 0u;}
    void        ClearButtonAll        ();


private:
    /* update the input button interface */
    void __UpdateButtonsStart();
    void __UpdateButtonsEnd();

    /* handle joystick input */
    void      __OpenJoysticks();
    void      __CloseJoysticks();
    coreUintW __GetJoystickIndex(const SDL_JoystickID iID)const;
};


// ****************************************************************
/* call function for each active finger */
template <typename F> void CoreInput::ForEachFinger(const coreInputType eType, F&& nFunction)
{
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
        // check finger status and call function
        if(this->GetTouchButton(i, eType))
            nFunction(i);
    }
}


#endif /* _CORE_GUARD_INPUT_H_ */
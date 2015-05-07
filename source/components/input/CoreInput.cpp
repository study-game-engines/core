//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreInput::coreKeyboard::coreKeyboard()noexcept
: iLast (CORE_INPUT_KEY (UNKNOWN))
, iChar (CORE_INPUT_CHAR(UNKNOWN))
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::coreMouse::coreMouse()noexcept
: iLast     (0xFFu)
, vPosition (coreVector2(0.0f,0.0f))
, vRelative (coreVector3(0.0f,0.0f,0.0f))
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::coreJoystick::coreJoystick()noexcept
: pController (NULL)
, pJoystick   (NULL)
, pHaptic     (NULL)
, iLast       (0xFFu)
, vRelative   (coreVector2(0.0f,0.0f))
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::coreTouch::coreTouch()noexcept
: vPosition (coreVector2(0.0f,0.0f))
, vRelative (coreVector2(0.0f,0.0f))
, fPressure (0.0f)
{
    std::memset(abButton, 0, sizeof(abButton));
}


// ****************************************************************
// constructor
CoreInput::CoreInput()noexcept
: m_pCursor        (NULL)
, m_bCursorVisible (true)
{
    Core::Log->Header("Input Interface");

    // check for available joystick devices
    const coreUintW iNumJoysticks = SDL_NumJoysticks();
    if(iNumJoysticks)
    {
        Core::Log->ListStartInfo("Joysticks and Gamepads found");
        {
            m_aJoystick.reserve(iNumJoysticks);
            for(coreUintW i = 0u; i < iNumJoysticks; ++i)
            {
                coreJoystick oJoystick;

                // open game controller and joystick device
                oJoystick.pController = SDL_GameControllerOpen(i);
                oJoystick.pJoystick   = oJoystick.pController ? SDL_GameControllerGetJoystick(oJoystick.pController) : SDL_JoystickOpen(i);

                // try to open haptic device
                oJoystick.pHaptic = SDL_HapticOpenFromJoystick(oJoystick.pJoystick);
                if(oJoystick.pHaptic)
                {
                    // check for simple rumble playback
                    if(SDL_HapticRumbleInit(oJoystick.pHaptic))
                    {
                        SDL_HapticClose(oJoystick.pHaptic);
                        oJoystick.pHaptic = NULL;
                    }
                }

                // save joystick object
                m_aJoystick.push_back(oJoystick);
                Core::Log->ListAdd(CORE_LOG_BOLD("%s:") " %s (%s / %s)", oJoystick.pController ? "Gamepad" : "Joystick",
                                                                         this->GetJoystickName(i), this->GetJoystickGUID(i),
                                                                         oJoystick.pHaptic ? "haptic" : "no haptic");
            }
        }
        Core::Log->ListEnd();

        // sort correctly by joystick instance ID
        std::sort(m_aJoystick.begin(), m_aJoystick.end(), [](const coreJoystick& a, const coreJoystick& b)
        {
            return (SDL_JoystickInstanceID(a.pJoystick) < SDL_JoystickInstanceID(b.pJoystick));
        });
    }
    else Core::Log->Info("No Joysticks or Gamepads found");

    // append empty joystick object to prevent problems
    m_aJoystick.push_back(coreJoystick());
}


// ****************************************************************
// destructor
CoreInput::~CoreInput()
{
    // close all joystick and haptic devices
    FOR_EACH(it, m_aJoystick)
    {
             if(it->pHaptic)     SDL_HapticClose        (it->pHaptic);
             if(it->pController) SDL_GameControllerClose(it->pController);
        else if(it->pJoystick)   SDL_JoystickClose      (it->pJoystick);
    }
    m_aJoystick.clear();

    // free the hardware mouse cursor
    if(m_pCursor) SDL_FreeCursor(m_pCursor);

    Core::Log->Info(CORE_LOG_BOLD("Input Interface shut down"));
}


// ****************************************************************
// set the cursor object
void CoreInput::DefineCursor(const coreChar* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // load texture from file
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(pData)
    {
        if(m_pCursor) SDL_FreeCursor(m_pCursor);

        // create hardware mouse cursor
        m_pCursor = SDL_CreateColorCursor(pData, 0, 0);
        SDL_SetCursor(m_pCursor);

        // free the texture
        SDL_FreeSurface(pData);
    }
}


// ****************************************************************
// show or hide the mouse cursor
void CoreInput::ShowCursor(const coreBool& bStatus)
{
    if(m_bCursorVisible == bStatus) return;

    // toggle cursor visibility
    SDL_ShowCursor(bStatus ? 1 : 0);
    SDL_SetRelativeMouseMode(bStatus ? SDL_FALSE : SDL_TRUE);

    // save visibility status
    m_bCursorVisible = bStatus;
}


// ****************************************************************
// control mouse with keyboard
void CoreInput::UseMouseWithKeyboard(const coreInputKey& iLeft, const coreInputKey& iRight, const coreInputKey& iUp, const coreInputKey& iDown, const coreInputKey& iButton1, const coreInputKey& iButton2, const coreFloat& fSpeed)
{
    // TODO: implement function
}


// ****************************************************************
// control mouse with joystick
void CoreInput::UseMouseWithJoystick(const coreUintW& iIndex, const coreUint8& iButton1, const coreUint8& iButton2, const coreFloat& fSpeed)
{
    if(iIndex >= m_aJoystick.size()) return;

    // move the mouse cursor
    const coreVector2& vAcc = m_aJoystick[iIndex].vRelative;
    if(!vAcc.IsNull())
    {
        const coreVector2 vPos = this->GetMousePosition() + coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vAcc.Normalized() * Core::System->GetResolution().yx() / Core::System->GetResolution().Min() * Core::System->GetTime() * fSpeed + vPos) * Core::System->GetResolution();
        SDL_WarpMouseInWindow(Core::System->GetWindow(), F_TO_SI(vNew.x + 0.5f), F_TO_SI(-vNew.y + 0.5f));
    }

    // press mouse buttons
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_PRESS))   {this->SetMouseButton(1, true); this->RumbleJoystick(iIndex, 0.2f, 200u);}
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_RELEASE)) {this->SetMouseButton(1, false);}
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_PRESS))   {this->SetMouseButton(2, true); this->RumbleJoystick(iIndex, 0.2f, 200u);}
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_RELEASE)) {this->SetMouseButton(2, false);}
}


// ****************************************************************
// forward d-pad input to stick input on joystick
void CoreInput::ForwardDpadToStick(const coreUintW& iIndex)
{
    // check for d-pad buttons and invoke stick movement
         if(this->GetJoystickButton(iIndex, 11u, CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 1u,  1.0f);
    else if(this->GetJoystickButton(iIndex, 12u, CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 1u, -1.0f);
         if(this->GetJoystickButton(iIndex, 13u, CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 0u, -1.0f);
    else if(this->GetJoystickButton(iIndex, 14u, CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 0u,  1.0f);

    // reset stick movement on release
    if(this->GetJoystickButton(iIndex, 11u, CORE_INPUT_RELEASE) ||
       this->GetJoystickButton(iIndex, 12u, CORE_INPUT_RELEASE)) this->SetJoystickRelative(iIndex, 1u, 0.0f);
    if(this->GetJoystickButton(iIndex, 13u, CORE_INPUT_RELEASE) ||
       this->GetJoystickButton(iIndex, 14u, CORE_INPUT_RELEASE)) this->SetJoystickRelative(iIndex, 0u, 0.0f);
}


// ****************************************************************
// process input events
coreBool CoreInput::ProcessEvent(const SDL_Event& Event)
{
    switch(Event.type)
    {
    // set text-input character
    case SDL_TEXTINPUT:
        Core::Input->SetKeyboardChar(Event.text.text[0]);
        break;

    // press keyboard button
    case SDL_KEYDOWN:
        this->SetKeyboardButton(Event.key.keysym.scancode, true);
             if(Event.key.keysym.scancode == CORE_INPUT_KEY(BACKSPACE)) this->SetKeyboardChar(CORE_INPUT_CHAR(BACKSPACE));
        else if(Event.key.keysym.scancode == CORE_INPUT_KEY(RETURN))    this->SetKeyboardChar(CORE_INPUT_CHAR(RETURN));
        else if(Event.key.keysym.scancode == CORE_INPUT_KEY(KP_ENTER))  this->SetKeyboardChar(CORE_INPUT_CHAR(RETURN));
        else if(Event.key.keysym.mod & KMOD_CTRL)
        {
                 if(Event.key.keysym.scancode == CORE_INPUT_KEY(X)) this->SetKeyboardChar(CORE_INPUT_CHAR(CUT));
            else if(Event.key.keysym.scancode == CORE_INPUT_KEY(C)) this->SetKeyboardChar(CORE_INPUT_CHAR(COPY));
            else if(Event.key.keysym.scancode == CORE_INPUT_KEY(V)) this->SetKeyboardChar(CORE_INPUT_CHAR(PASTE));
        }
        else if(Event.key.keysym.scancode == CORE_INPUT_KEY(PRINTSCREEN)) return false;
        break;

    // release keyboard button
    case SDL_KEYUP:
        this->SetKeyboardButton(Event.key.keysym.scancode, false);
        break;

    // press mouse button
    case SDL_MOUSEBUTTONDOWN:
        this->SetMouseButton(Event.button.button, true);
        break;

    // release mouse button
    case SDL_MOUSEBUTTONUP:
        this->SetMouseButton(Event.button.button, false);
        break;

    // move mouse position
    case SDL_MOUSEMOTION:
        if(Event.motion.x != F_TO_SI(0.5f*Core::System->GetResolution().x) ||
           Event.motion.y != F_TO_SI(0.5f*Core::System->GetResolution().y))
        {
            this->SetMousePosition(coreVector2(I_TO_F(Event.motion.x),    -I_TO_F(Event.motion.y))   /Core::System->GetResolution() + coreVector2(-0.5f,0.5f));
            this->SetMouseRelative(coreVector2(I_TO_F(Event.motion.xrel), -I_TO_F(Event.motion.yrel))/Core::System->GetResolution() + this->GetMouseRelative().xy());
        }
        break;

    // move mouse wheel
    case SDL_MOUSEWHEEL:
        this->SetMouseWheel(I_TO_F(Event.wheel.y));
        break;

    // press joystick button
    case SDL_JOYBUTTONDOWN:
        if(__CORE_INPUT_JOYSTICK(Event.jbutton.which).pController) break;
    case SDL_CONTROLLERBUTTONDOWN:
        this->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, true);
        break;

    // release joystick button
    case SDL_JOYBUTTONUP:
        if(__CORE_INPUT_JOYSTICK(Event.jbutton.which).pController) break;
    case SDL_CONTROLLERBUTTONUP:
        this->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, false);
        break;

    // move joystick axis
    case SDL_JOYAXISMOTION:
        if(__CORE_INPUT_JOYSTICK(Event.jbutton.which).pController) break;
    case SDL_CONTROLLERAXISMOTION:
        if(Event.jaxis.axis < 2u)
        {
            if(ABS(coreInt32(Event.jaxis.value)) > CORE_INPUT_JOYSTICK_DEAD)
                 this->SetJoystickRelative(Event.jaxis.which, Event.jaxis.axis, CLAMP(I_TO_F(Event.jaxis.value) / I_TO_F(CORE_INPUT_JOYSTICK_MAX) * (Event.jaxis.axis ? -1.0f : 1.0f), -1.0f, 1.0f));
            else this->SetJoystickRelative(Event.jaxis.which, Event.jaxis.axis, 0.0f);
        }
        break;

    // press finger
    case SDL_FINGERDOWN:
        this->SetTouchButton  (coreUintW(Event.tfinger.fingerId), true);
        this->SetTouchPosition(coreUintW(Event.tfinger.fingerId), coreVector2(Event.tfinger.x, -Event.tfinger.y) + coreVector2(-0.5f,0.5f));
        break;

    // release finger
    case SDL_FINGERUP:
        this->SetTouchButton(coreUintW(Event.tfinger.fingerId), false);
        break;

    // move finger
    case SDL_FINGERMOTION:
        this->SetTouchPosition(coreUintW(Event.tfinger.fingerId), coreVector2(Event.tfinger.x,  -Event.tfinger.y)  + coreVector2(-0.5f,0.5f));
        this->SetTouchRelative(coreUintW(Event.tfinger.fingerId), coreVector2(Event.tfinger.dx, -Event.tfinger.dy) + this->GetTouchRelative(coreUintW(Event.tfinger.fingerId)));
        this->SetTouchPressure(coreUintW(Event.tfinger.fingerId), Event.tfinger.pressure);
        break;
    }

    return true;
}


// ****************************************************************
// update the input button interface
void CoreInput::__UpdateButtons()
{
    // process keyboard inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_KEYBOARD; ++i)
    {
             if( m_Keyboard.aabButton[i][0]) __CORE_INPUT_PRESS  (m_Keyboard.aabButton[i])
        else if(!m_Keyboard.aabButton[i][0]) __CORE_INPUT_RELEASE(m_Keyboard.aabButton[i])
    }

    // process mouse inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_MOUSE; ++i)
    {
             if( m_Mouse.aabButton[i][0]) __CORE_INPUT_PRESS  (m_Mouse.aabButton[i])
        else if(!m_Mouse.aabButton[i][0]) __CORE_INPUT_RELEASE(m_Mouse.aabButton[i])
    }

    // process joystick inputs
    FOR_EACH(it, m_aJoystick)
    {
        for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i)
        {
                 if( it->aabButton[i][0]) __CORE_INPUT_PRESS  (it->aabButton[i])
            else if(!it->aabButton[i][0]) __CORE_INPUT_RELEASE(it->aabButton[i])
        }
    }

    // process touch inputs
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
             if( m_aTouch[i].abButton[0]) __CORE_INPUT_PRESS  (m_aTouch[i].abButton)
        else if(!m_aTouch[i].abButton[0]) __CORE_INPUT_RELEASE(m_aTouch[i].abButton)
    }

#if defined(_CORE_LINUX_)

    if(!m_bCursorVisible)
    {
        // hold cursor in window center when not visible
        SDL_WarpMouseInWindow(Core::System->GetWindow(), F_TO_SI(0.5f*Core::System->GetResolution().x),
                                                         F_TO_SI(0.5f*Core::System->GetResolution().y));
    }

#endif
}


// ****************************************************************
// clear the input button interface
void CoreInput::__ClearButtons()
{
    // clear all last pressed buttons
    m_Keyboard.iLast = CORE_INPUT_KEY(UNKNOWN);
    m_Mouse.iLast    = 0xFFu;
    FOR_EACH(it, m_aJoystick) it->iLast = 0xFFu;

    // reset all relative movements
    m_Mouse.vRelative = coreVector3(0.0f,0.0f,0.0f);
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
        m_aTouch[i].vRelative = coreVector2(0.0f,0.0f);

    // clear current text-input character
    m_Keyboard.iChar = CORE_INPUT_CHAR(UNKNOWN);
}
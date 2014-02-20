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
coreTextBox::coreTextBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)
: coreButton  (pcIdle, pcBusy, pcFont, iHeight, iLength)
, m_sText     ("")
, m_sPrevious ("")
, m_cReplace  ('\0')
, m_bInput    (false)
, m_bDisplay  (false)
, m_bReturned (false)
{
    SDL_assert(iLength);

    // reserve memory for text
    m_sText.reserve(iLength+1);
    m_sPrevious.reserve(iLength+1);
}


// ****************************************************************    
// move the text-box
void coreTextBox::Move()
{
    // move the button
    coreButton::Move();

    // check for interaction
    if(this->IsClicked()) this->SetInput(true);
    if(m_bInput)
    {
        if(Core::Input->GetKeyboardButton(SDL_SCANCODE_ESCAPE, CORE_INPUT_PRESS))
        {
            // cancel and reset text-input
            m_sText = m_sPrevious;
            this->SetInput(false);
        }
        else if(Core::Input->GetMouseButton(CORE_INPUT_LEFT, CORE_INPUT_PRESS) && !this->IsFocused())
        {
            // cancel text-input
            this->SetInput(false);
        }
        else if(this->__Write()) 
        {
            // finish text-input
            m_bReturned = true;
            this->SetInput(false);
        }
    }
    else m_bReturned = false;

    // update the caption
    if(m_bDisplay)
    {
        if(m_cReplace)
        {
            // hide text with replacement character
            const std::string sPassword(m_sText.length(), m_cReplace);
            m_pCaption->SetText(m_bInput ? coreData::Print("%s|", sPassword.c_str()) : sPassword.c_str());
        }
        else m_pCaption->SetText(m_bInput ? coreData::Print("%s|", m_sText.c_str()) : m_sText.c_str());
            
        m_bDisplay = false;
    }
}


// ****************************************************************
// set text-input status
// TODO: test, if problems with concurrent SDL calls (use reference counter?)
void coreTextBox::SetInput(const bool& bInput)
{
    if(m_bInput == bInput) return;

    // save new text-input status
    m_bInput   = bInput;  
    m_bDisplay = true;

    if(m_bInput)
    {
        // start text-input
        SDL_StartTextInput();
        m_sPrevious = m_sText;
    }
    else
    {
        // stop text-input
        SDL_StopTextInput();
    }
}


// ****************************************************************
// process new text-input characters
bool coreTextBox::__Write()
{
    // get new text-input character
    const char cChar = Core::Input->GetKeyboardChar();
    if(cChar)
    {
        if(cChar == SDLK_RETURN)
        {
            // finish the text-input
            return true;
        }
        else if(cChar == SDLK_BACKSPACE)
        {
            // remove last character
            if(!m_sText.empty()) m_sText.pop_back();
        }
        else if(m_sText.length() < coreUint(m_pCaption->GetLength()-1))
        {
            // append new character
            m_sText.append(1, cChar);
        }

        m_bDisplay = true;
    }

    return false;
}
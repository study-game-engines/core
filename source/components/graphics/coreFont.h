//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FONT_H_
#define _CORE_GUARD_FONT_H_


// ****************************************************************
// font class
// TODO: distance fields for sharper text
class coreFont final : public coreResource
{
private:
    coreLookup<int, TTF_Font*> m_apFont;   //!< list with sub-fonts in different heights
    coreFile* m_pFile;                     //!< file object with resource data


public:
    coreFont()noexcept;
    ~coreFont();

    DISABLE_COPY(coreFont)

    //! load and unload font resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! create text surface with the font
    //! @{
    SDL_Surface* CreateText (const char*       pcText, const int& iHeight);
    SDL_Surface* CreateGlyph(const coreUshort& iGlyph, const int& iHeight);
    //! @}


private:
    //! init the font in a specific height
    //! @{
    bool __InitHeight(const int& iHeight);
    //! @}
};


// ****************************************************************
// font resource access type
typedef coreResourcePtr<coreFont> coreFontPtr;


#endif // _CORE_GUARD_FONT_H_
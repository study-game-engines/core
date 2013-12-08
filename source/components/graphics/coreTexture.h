//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTURE_H_
#define _CORE_GUARD_TEXTURE_H_


// ****************************************************************
// texture definitions
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define CORE_TEXTURE_MASK 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
    #define CORE_TEXTURE_MASK 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif

#define CORE_TEXTURE_UNITS  8                        //!< number of used texture units
#define CORE_TEXTURE_SHADOW (CORE_TEXTURE_UNITS-1)   //!< texture unit for shadow sampling


// ****************************************************************
// texture class
// TODO: check for max available texture units (only at start?)
// TODO: implement sampler objects
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier/OpenGL name
    coreVector2 m_vResolution;                           //!< texture resolution

    static int s_iActiveUnit;                            //!< current active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   //!< texture objects currently associated with texture units

    coreSync m_Sync;                                     //!< sync object for asynchronous texture loading
    static SDL_SpinLock s_iLock;                         //!< spinlock to prevent asynchronous texture unit access


public:
    coreTexture()noexcept;
    ~coreTexture();

    //! load and unload texture resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! enable and disable the texture
    //! @{
    void Enable(const coreByte& iUnit);
    static void Disable(const coreByte& iUnit);
    static void DisableAll();
    //! @}

    //! generate empty base texture
    //! @{
    inline void Generate() {SDL_assert(!m_iTexture); glGenTextures(1, &m_iTexture);}
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetTexture()const         {return m_iTexture;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {return "data/textures/black.png";}
    //! @}

    //! lock and unlock texture unit access
    //! @{
    static inline void Lock()   {SDL_AtomicLock(&s_iLock);}
    static inline void Unlock() {SDL_AtomicUnlock(&s_iLock);}
    //! @}
};


// ****************************************************************
// texture resource access type
typedef coreResourcePtr<coreTexture> coreTexturePtr;


#endif // _CORE_GUARD_TEXTURE_H_
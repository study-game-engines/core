//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

int          coreTexture::s_iActiveUnit                     = 0;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS]; // = NULL;
SDL_SpinLock coreTexture::s_iLock                           = 0;


// ****************************************************************
// constructor
coreTexture::coreTexture()noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
{
}

coreTexture::coreTexture(const char* pcPath)noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreTexture::coreTexture(coreFile* pFile)noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
{
    // load from file
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreTexture::~coreTexture()
{
    this->Unload();
}


// ****************************************************************
// load texture resource data
// TODO: check for proper use of PBO, maybe implement static buffer(s!)
coreError coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iStatus = m_Sync.CheckSync(0);
    if(iStatus >= 0) return iStatus;

    SDL_assert(!m_iTexture);

    if(m_iTexture)        return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // decompress file data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(!pData)
    {
        Core::Log->Error(0, coreData::Print("Texture (%s) could not be loaded", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    const coreUint iDataSize = pData->w * pData->h * 3;
    const bool& bPixelBuffer = Core::Graphics->SupportFeature("GL_ARB_pixel_buffer_object");
    const bool& bAnisotropic = Core::Graphics->SupportFeature("GL_EXT_texture_filter_anisotropic");
    const bool& bMipMap      = Core::Graphics->SupportFeature("GL_ARB_framebuffer_object");

    // save texture attributes
    m_sPath       = pFile->GetPath();
    m_iSize       = pData->w * pData->h * (bMipMap ? 4 : 3);
    m_vResolution = coreVector2(float(pData->w), float(pData->h));

    // convert data format
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pData->w, pData->h, 24, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pData, NULL, pConvert, NULL);

    GLuint iBuffer;
    if(bPixelBuffer)
    {
        // generate pixel buffer object for asynchronous texture loading
        glGenBuffers(1, &iBuffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, iBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, iDataSize, NULL, GL_STREAM_DRAW);

        // copy texture data into PBO
        void* pRange = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        std::memcpy(pRange, pConvert->pixels, iDataSize);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    SDL_AtomicLock(&s_iLock);
    {
        // generate texture
        glGenTextures(1, &m_iTexture);

        // bind texture to free texture unit
        int iLoadUnit = -1;
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
        {
            if(!s_apBound[i])
            {
                if(i != s_iActiveUnit)
                {
                    // switch active texture unit
                    iLoadUnit = i;
                    glActiveTexture(GL_TEXTURE0+iLoadUnit);
                }
                break;
            }
        }
        glBindTexture(GL_TEXTURE_2D, m_iTexture);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        if(bAnisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREFILTER));

        // load texture data from PBO
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pConvert->w, pConvert->h, 0, GL_RGB, GL_UNSIGNED_BYTE, bPixelBuffer ? 0 : pConvert->pixels);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);

        // unbind texture from texture unit
        glBindTexture(GL_TEXTURE_2D, 0);
        if(iLoadUnit >= 0) glActiveTexture(GL_TEXTURE0+s_iActiveUnit);
    }
    SDL_AtomicUnlock(&s_iLock);

    // create sync object
    const bool bSync = m_Sync.CreateSync();

    if(bPixelBuffer)
    {
        // delete PBO
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers(1, &iBuffer);
    }

    // delete file data
    SDL_FreeSurface(pData);
    SDL_FreeSurface(pConvert);

    Core::Log->Info(coreData::Print("Texture (%s) loaded%s", m_sPath.c_str(), bSync ? " asynchronous" : ""));
    return bSync ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload texture resource data
coreError coreTexture::Unload()
{
    if(!m_iTexture) return CORE_INVALID_CALL;

    // delete texture
    glDeleteTextures(1, &m_iTexture);
    if(!m_sPath.empty()) Core::Log->Info(coreData::Print("Texture (%s) unloaded", m_sPath.c_str()));

    // delete sync object
    m_Sync.DeleteSync();

    // reset attributes
    m_sPath       = "";
    m_iSize       = 0;
    m_iTexture    = 0;
    m_vResolution = coreVector2(0.0f,0.0f);

    return CORE_OK;
}


// ****************************************************************
// enable texture
void coreTexture::Enable(const coreByte& iUnit)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);
    SDL_assert(s_apBound[iUnit] == NULL);

    // save texture binding
    s_apBound[iUnit] = this;

    SDL_AtomicLock(&s_iLock);
    {
        // bind texture to texture unit
        if(s_iActiveUnit != iUnit)
        {
            glActiveTexture(GL_TEXTURE0+iUnit);
            s_iActiveUnit = iUnit;
        }
        glBindTexture(GL_TEXTURE_2D, m_iTexture);
    }
    SDL_AtomicUnlock(&s_iLock);
}


// ****************************************************************
// disable texture
void coreTexture::Disable(const coreByte& iUnit)
{
    SDL_assert(s_apBound[iUnit] != NULL);

    SDL_AtomicLock(&s_iLock);
    {
        // unbind texture from texture unit
        if(s_iActiveUnit != iUnit)
        {
            glActiveTexture(GL_TEXTURE0+iUnit);
            s_iActiveUnit = iUnit;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    SDL_AtomicUnlock(&s_iLock);

    // reset texture binding
    s_apBound[iUnit] = NULL;
}


// ****************************************************************
// disable all textures
void coreTexture::DisableAll()
{
    // traverse all texture units
    for(int i = CORE_TEXTURE_UNITS-1; i >= 0; --i)
        if(s_apBound[i]) Disable(i);
}
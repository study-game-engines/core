//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_LABEL_DETAIL 0.95f


// ****************************************************************
// constructor
coreLabel::coreLabel()noexcept
: m_iHeight     (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLength     (0)
, m_sText       ("")
, m_fScale      (1.0f)
, m_iGenerate   (0)
{
}

coreLabel::coreLabel(const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept
: coreLabel ()
{
    // construct on creation
    this->Construct(pcFont, iHeight, iLength);
}


// ****************************************************************
// destructor
coreLabel::~coreLabel()
{
    // free own texture
    Core::Manager::Resource->Free(&m_apTexture[0]);
}


// ****************************************************************
// construct the label
void coreLabel::Construct(const char* pcFont, const int& iHeight, const coreUint& iLength)
{
    // save properties
    m_iHeight = F_TO_SI(I_TO_F(iHeight) * (Core::System->GetResolution().y / 800.0f) * CORE_LABEL_DETAIL);
    m_iLength = iLength;

    // set font object
    m_pFont = Core::Manager::Resource->Get<coreFont>(pcFont);

    // allocate own texture to display text
    m_apTexture[0] = Core::Manager::Resource->LoadNew<coreTexture>();

    // load shaders
    this->DefineProgram(Core::Manager::Resource->Load<coreProgram>("default_label_program", CORE_RESOURCE_UPDATE_AUTO,   NULL))
        ->AttachShader (Core::Manager::Resource->Load<coreShader> ("default_label.vert",    CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label.vert"))
        ->AttachShader (Core::Manager::Resource->Load<coreShader> ("default_label.frag",    CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label.frag"))
        ->Finish();

    // reserve memory for text
    if(iLength) m_sText.reserve(iLength+1);
}


// ****************************************************************
// render the label
void coreLabel::Render()
{
    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    if(m_iGenerate)
    {
        // check if requested font is loaded
        if(!m_pFont.GetHandle()->IsLoaded()) return;

        // generate the texture
        if(m_iGenerate & 2) this->__Generate(m_sText.c_str(), m_iLength ? true : false);
        if(m_iGenerate & 1)
        {
            // update the object size
            this->SetSize(m_fScale * m_vTexSize * (m_vResolution * RCP(Core::System->GetResolution().y)) * (1.0f / CORE_LABEL_DETAIL));
            coreObject2D::Move();
        }
        m_iGenerate = 0;
    }

    // render the 2d-object
    coreObject2D::Render();
}


// ****************************************************************
// move the label
// TODO: transformation matrix is not always immediately updated after a Move(), because re-generation must be in Render(), with Move() afterwards
void coreLabel::Move()
{
    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    // move the 2d-object
    if(!(m_iGenerate & 1)) coreObject2D::Move();
}


// ****************************************************************
// change the current text
bool coreLabel::SetText(const char* pcText)
{
    ASSERT(!m_iLength || std::strlen(pcText) <= m_iLength)

    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText))
    {
        m_iGenerate = 3;

        // change the current text
        if(m_iLength) m_sText.assign(pcText, m_iLength);
                 else m_sText.assign(pcText);
        return true;
    }
    return false;
}

bool coreLabel::SetText(const char* pcText, const coreUint& iNum)
{
    ASSERT(!m_iLength || (MIN(iNum, (coreUint)std::strlen(pcText)) <= m_iLength && iNum <= m_iLength))

    // check for new text
    if(iNum != m_sText.length() || std::strcmp(m_sText.c_str(), pcText))
    {
        m_iGenerate = 3;

        // change the current text
        if(m_iLength) m_sText.assign(pcText, MIN(iNum, m_iLength));
                 else m_sText.assign(pcText, iNum);
        return true;
    }
    return false;
}


// ****************************************************************
// reset with the resource manager
void coreLabel::__Reset(const coreResourceReset& bInit)
{
    if(!m_pFont) return;

    if(bInit)
    {
        // invoke texture generation
        m_vResolution = coreVector2(0.0f,0.0f);
        m_iGenerate   = 3;
    }
    else m_apTexture[0]->Unload();
}


// ****************************************************************
// generate the texture
// TODO: use font-size calculation interface to check for font-size and pre-allocations
void coreLabel::__Generate(const char* pcText, const bool& bSub)
{
    // create text surface data
    SDL_Surface* pSurface = m_pFont->CreateText(pcText, m_iHeight);
    ASSERT(pSurface->format->BitsPerPixel == 8)

    // convert text surface data
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, coreMath::NextAlign<4>(pSurface->w), pSurface->h, 24, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pSurface, NULL, pConvert, NULL);

    // get new texture resolution
    const coreVector2 vNewResolution = coreVector2(I_TO_F(pSurface->w - 2), I_TO_F(pSurface->h));

    if(bSub)
    {
        if(!m_vResolution.x)
        {
            // create static texture
            this->__Generate((std::string(m_iLength, 'W') + "gjy").c_str(), false);
        }

        // update only a specific area of the texture
        ASSERT((vNewResolution.x <= m_vResolution.x) && (vNewResolution.y <= m_vResolution.y))
        m_apTexture[0]->Modify(0, 0, pConvert->w, pConvert->h, pConvert->w * pConvert->h * 3, pConvert->pixels);

        // display only the specific area
        this->SetTexSize(vNewResolution / m_vResolution);
    }
    else
    {
        // delete old texture
        m_apTexture[0]->Unload();

        // create new texture
        m_apTexture[0]->Create(pConvert->w, pConvert->h, CORE_TEXTURE_SPEC_RGB, GL_CLAMP_TO_EDGE, false);
        m_apTexture[0]->Modify(0, 0, pConvert->w, pConvert->h, pConvert->w * pConvert->h * 3, pConvert->pixels);

        // save new texture resolution
        m_vResolution = vNewResolution;
    }

    // delete text surface data
    SDL_FreeSurface(pSurface);
    SDL_FreeSurface(pConvert);
}
//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_GLES_)
    #define glClearDepth glClearDepthf
#endif


// ******************************************************************
// constructor
CoreGraphics::CoreGraphics()noexcept
: m_fFOV            (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FOV))
, m_fNearClip       (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_CLIP_NEAR))
, m_fFarClip        (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_CLIP_FAR))
, m_vCamPosition    (coreVector3(0.0f,0.0f,0.0f))
, m_vCamDirection   (coreVector3(0.0f,0.0f,0.0f))
, m_vCamOrientation (coreVector3(0.0f,0.0f,0.0f))
, m_vCurResolution  (coreVector2(0.0f,0.0f))
{
    Core::Log->Header("Graphics Interface");

    // create primary OpenGL context
    m_RenderContext = SDL_GL_CreateContext(Core::System->GetWindow());
    if(!m_RenderContext) Core::Log->Error(true, "Primary OpenGL context could not be created (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Primary OpenGL context created");

    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DUALCONTEXT) && (Core::System->SupportNumCores() >= 2))
    {
        // create secondary OpenGL context
        m_ResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
        if(!m_ResourceContext) Core::Log->Error(false, "Secondary OpenGL context could not be created (SDL: %s)", SDL_GetError());
        else Core::Log->Info("Secondary OpenGL context created");
    }
    else m_ResourceContext = NULL;

    // assign primary OpenGL context to main window
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), m_RenderContext))
        Core::Log->Error(true, "Primary OpenGL context could not be assigned to main window (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Primary OpenGL context assigned to main window");

    // init GLEW on primary OpenGL context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error(true, "GLEW could not be initialized on primary OpenGL context (GLEW: %s)", glewGetErrorString(iError));
    else Core::Log->Info("GLEW initialized on primary OpenGL context (%s)", glewGetString(GLEW_VERSION));

    // log video card information
    Core::Log->ListStart("Video Card Information");
    Core::Log->ListEntry("<b>Vendor:</b> %s",         glGetString(GL_VENDOR));
    Core::Log->ListEntry("<b>Renderer:</b> %s",       glGetString(GL_RENDERER));
    Core::Log->ListEntry("<b>OpenGL Version:</b> %s", glGetString(GL_VERSION));
    Core::Log->ListEntry("<b>Shader Version:</b> %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    Core::Log->ListEntry(r_cast<const char*>(glGetString(GL_EXTENSIONS)));
    Core::Log->ListEnd();

    // set numerical OpenGL version
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL);
    m_fOpenGL = fForceOpenGL ? fForceOpenGL : coreData::StrVersion(r_cast<const char*>(glGetString(GL_VERSION)));
    m_fGLSL   = coreData::StrVersion(r_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // check OpenGL version
    if(m_fOpenGL < 2.0f) Core::Log->Error(true, "Minimum system requirements not met, video card supporting at least OpenGL 2.0 required");

    // enable OpenGL debugging
    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DEBUGCONTEXT) || g_bDebug)
        Core::Log->EnableOpenGL();

    // enable vertical synchronization
    if(SDL_GL_SetSwapInterval(1)) Core::Log->Error(false, "Vertical Synchronization not directly supported (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Vertical Synchronization enabled");

    // enable texturing
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DITHER);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonOffset(1.1f, 4.0f);
    glClearDepth(1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create uniform buffer object for global shader-data
    if(GLEW_ARB_uniform_buffer_object)
    {
        // calculate UBO size (std140)
        constexpr_var coreUint iSize = sizeof(coreMatrix4)*4 + sizeof(coreLight)*CORE_GRAPHICS_LIGHTS;

        // generate and bind global UBO to a buffer target
        m_iUniformBuffer.Create(GL_UNIFORM_BUFFER, iSize, NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_GLOBAL_NUM, m_iUniformBuffer);
    }

    // reset camera and view
    this->SetCamera(coreVector3(0.0f,0.0f,0.0f), coreVector3(0.0f,0.0f,-1.0f), coreVector3(0.0f,1.0f,0.0f));
    this->ResizeView(coreVector2(0.0f,0.0f));

    // reset ambient
    for(int i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));

    // reset scene
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// ******************************************************************
// destructor
CoreGraphics::~CoreGraphics()
{
    Core::Log->Info("Graphics Interface shut down");

    // delete global UBO
    m_iUniformBuffer.Delete();

    // dissociate primary OpenGL context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DeleteContext(m_ResourceContext);
    SDL_GL_DeleteContext(m_RenderContext);
}


// ******************************************************************
// set camera and create camera matrix
void CoreGraphics::SetCamera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    bool bNewCamera = false;

    // set attributes of the camera
    const coreVector3 vDirNorm = vDirection.Normalized();   
    const coreVector3 vOriNorm = vOrientation.Normalized(); 
    if(m_vCamPosition    != vPosition) {m_vCamPosition    = vPosition; bNewCamera = true;}
    if(m_vCamDirection   != vDirNorm)  {m_vCamDirection   = vDirNorm;  bNewCamera = true;}
    if(m_vCamOrientation != vOriNorm)  {m_vCamOrientation = vOriNorm;  bNewCamera = true;}

    if(bNewCamera)
    {
        // create camera matrix
        m_mCamera = coreMatrix4::Camera(m_vCamPosition, m_vCamDirection, m_vCamOrientation);

        // send transformation data to the global UBO
        this->__SendTransformation();
    }
}


// ******************************************************************
// resize view and create projection matrices
void CoreGraphics::ResizeView(coreVector2 vResolution)
{
    // retrieve window resolution
    if(!vResolution.x) vResolution.x = Core::System->GetResolution().x;
    if(!vResolution.y) vResolution.y = Core::System->GetResolution().y;

    if(m_vCurResolution != vResolution)
    {
        m_vCurResolution = vResolution;

        // set viewport
        glViewport(0, 0, (int)vResolution.x, (int)vResolution.y);

        // generate projection matrices
        m_mPerspective = coreMatrix4::Perspective(vResolution, TO_RAD(m_fFOV), m_fNearClip, m_fFarClip);
        m_mOrtho       = coreMatrix4::Ortho(vResolution);

        // send transformation data to the global UBO
        this->__SendTransformation();
    }
}


// ******************************************************************
// set and update ambient light attributes
void CoreGraphics::SetLight(const int& iID, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue)
{
    SDL_assert(iID < CORE_GRAPHICS_LIGHTS);
    coreLight& CurLight = m_aLight[iID];

    bool bNewLight = false;

    // set attributes of the light
    const coreVector4 vDirNorm = coreVector4(vDirection.xyz().Normalized(), vDirection.w); 
    if(CurLight.vPosition  != vPosition) {CurLight.vPosition  = vPosition; bNewLight = true;}
    if(CurLight.vDirection != vDirNorm)  {CurLight.vDirection = vDirNorm;  bNewLight = true;}
    if(CurLight.vValue     != vValue)    {CurLight.vValue     = vValue;    bNewLight = true;}

    if(bNewLight && m_iUniformBuffer)
    {
        // map required area of the global UBO
        coreByte* pRange = m_iUniformBuffer.Map(sizeof(coreMatrix4)*4 + sizeof(coreLight)*iID, sizeof(coreLight));

        // update specific light
        std::memcpy(pRange, &CurLight, sizeof(coreLight));
        m_iUniformBuffer.Unmap(pRange);
    }
}


// ******************************************************************
// take screenshot
void CoreGraphics::Screenshot(const char* pcPath)
{
    const coreUint iWidth  = (coreUint)Core::System->GetResolution().x;
    const coreUint iHeight = (coreUint)Core::System->GetResolution().y;
    const coreUint iPitch  = iWidth*3;
    const coreUint iSize   = iHeight*iPitch;

    // create folder hierarchy
    const char* pcFullPath = coreData::Print(std::strcmp(coreData::StrRight(pcPath, 4), ".png") ? "%s.png" : "%s", pcPath);
    coreData::FolderCreate(pcFullPath);

    // read pixel data from the frame buffer
    coreByte* pData = new coreByte[iSize];
    glReadPixels(0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

    // flip pixel data vertically
    coreByte* pConvert = new coreByte[iSize];
    for(coreUint i = 0; i < iHeight; ++i)
        std::memcpy(pConvert + (iHeight-i-1)*iPitch, pData + i*iPitch, iPitch);

    // create an SDL surface
    SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(pConvert, iWidth, iHeight, 24, iPitch, CORE_TEXTURE_MASK);
    if(pSurface)
    {
        // save the surface as PNG image
        IMG_SavePNG(pSurface, pcFullPath);
        SDL_FreeSurface(pSurface);
    }

    SAFE_DELETE_ARRAY(pData)
    SAFE_DELETE_ARRAY(pConvert)
}

void CoreGraphics::Screenshot()
{
    // get timestamp
    coreUint awTime[6];
    coreData::DateTime(&awTime[0], &awTime[1], &awTime[2], &awTime[3], &awTime[4], &awTime[5]);

    // take standard screenshot
    this->Screenshot(coreData::Print("screenshots/screenshot_%04d%02d%02d_%02d%02d%02d",
                                     awTime[5], awTime[4], awTime[3], awTime[2], awTime[1], awTime[0]));
}


// ******************************************************************
// update the graphics scene
void CoreGraphics::__UpdateScene()
{
    // take screenshot
    if(Core::Input->GetKeyboardButton(SDL_SCANCODE_PRINTSCREEN, CORE_INPUT_PRESS))
        this->Screenshot();

    // disable last model, textures and shader-program
    coreModel::Disable(true);
    coreTexture::DisableAll();
    coreProgram::Disable(true);

    // swap main frame buffers
    SDL_GL_SwapWindow(Core::System->GetWindow());

    // reset the depth buffer
#if defined(_CORE_DEBUG_)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_DEPTH_BUFFER_BIT);
#endif
}


// ******************************************************************
// send transformation data to the global UBO
void CoreGraphics::__SendTransformation()
{
    if(!m_iUniformBuffer) return;

    const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

    // map required area of the global UBO
    coreByte* pRange = m_iUniformBuffer.Map(0, sizeof(coreMatrix4)*4);

    // update transformation matrices
    std::memcpy(pRange,                         &mViewProj,      sizeof(coreMatrix4));
    std::memcpy(pRange + sizeof(coreMatrix4)*1, &m_mCamera,      sizeof(coreMatrix4));
    std::memcpy(pRange + sizeof(coreMatrix4)*2, &m_mPerspective, sizeof(coreMatrix4));
    std::memcpy(pRange + sizeof(coreMatrix4)*3, &m_mOrtho,       sizeof(coreMatrix4));
    m_iUniformBuffer.Unmap(pRange);
}
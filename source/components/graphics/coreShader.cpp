//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreProgram* coreProgram::s_pCurrent = NULL;


// ****************************************************************
// constructor
coreShader::coreShader()noexcept
: m_iShader (0)
{
}

coreShader::coreShader(const char* pcPath)noexcept
: m_iShader (0)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreShader::coreShader(coreFile* pFile)noexcept
: m_iShader (0)
{
    // load from file
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreShader::~coreShader()
{
    this->Unload();
}


// ****************************************************************
// load shader resource data
coreError coreShader::Load(coreFile* pFile)
{
    SDL_assert(!m_iShader);

    if(m_iShader)         return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // extract file extension
    const char* pcExtension = coreData::StrExtension(pFile->GetPath());

    // set shader type
    GLenum iType;
         if(!std::strcmp(pcExtension, "vs") || !std::strcmp(pcExtension, "vert")) iType = GL_VERTEX_SHADER;
    else if(!std::strcmp(pcExtension, "fs") || !std::strcmp(pcExtension, "frag")) iType = GL_FRAGMENT_SHADER;
    else
    {
        Core::Log->Error(0, coreData::Print("Shader (%s) could not be identified (valid extension: vs, vert, fs, frag)", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    // retrieve global shader file
    coreFile* pGlobal = Core::Manager::Resource->RetrieveResourceFile(this->GetGlobalPath(iType));

    // assemble the shader
    const char* apcData[3] = {coreData::Print("#version %.0f", Core::Graphics->GetUniformBuffer() ? Core::Graphics->SupportGLSL()*100.0f : 110.0f),
                              reinterpret_cast<const char*>(pGlobal->GetData()),
                              reinterpret_cast<const char*>(pFile->GetData())};
    const GLint aiSize[3]  = {(GLint)std::strlen(apcData[0]),
                              (GLint)pGlobal->GetSize(),
                              (GLint)pFile->GetSize()};

    // create and compile the shader
    m_iShader = glCreateShader(iType);
    glShaderSource(m_iShader, 3, apcData, aiSize);
    glCompileShader(m_iShader);

    // check for errors
    int iStatus;
    glGetShaderiv(m_iShader, GL_COMPILE_STATUS, &iStatus);
    if(!iStatus)
    {
        // get length of error-log
        int iLength;
        glGetShaderiv(m_iShader, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get error-log
            char* pcLog = new char[iLength];
            glGetShaderInfoLog(m_iShader, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, coreData::Print("Shader (%s) could not be compiled", pFile->GetPath()));
            Core::Log->ListStart("Shader Error Log");
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    // save shader attributes
    m_sPath = pFile->GetPath();
    m_iSize = pFile->GetSize() + pGlobal->GetSize();

    Core::Log->Info(coreData::Print("Shader (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreError coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    Core::Log->Info(coreData::Print("Shader (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath   = "";
    m_iSize   = 0;
    m_iShader = 0;

    return CORE_OK;
}


// ****************************************************************
// constructor
coreProgram::coreProgram()
: m_iProgram (0)
, m_iStatus  (0)
{
    // reserve memory for shader objects
    m_apShader.reserve(4);
}


// ****************************************************************
// destructor
coreProgram::~coreProgram()
{
    // shut down the shader-program
    this->Reset(false);

    // remove all shader objects
    m_apShader.clear();
}


// ****************************************************************
// reset the object with the resource manager
void coreProgram::Reset(const bool& bInit)
{
    if(bInit) this->Init();
         else this->Exit();
}


// ****************************************************************
// init the shader-program
coreError coreProgram::Init()
{
    if(m_iStatus != 1) return CORE_INVALID_CALL;

    // check if all requested shaders are loaded
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        if(!(*it).IsLoaded()) return CORE_BUSY;

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    for(coreUint i = 0; i < m_apShader.size(); ++i)
        for(coreUint j = i+1; j < m_apShader.size(); ++j)
            SDL_assert(std::strcmp(m_apShader[i]->GetPath(), m_apShader[j]->GetPath()));

#endif

    // create shader-program
    SDL_assert(!m_iProgram);
    m_iProgram = glCreateProgram();

    // attach shader objects
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        glAttachShader(m_iProgram, (*it)->GetShader());

    // set attribute and output locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  CORE_SHADER_ATTRIBUTE_TEXTURE);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);
    if(Core::Graphics->GetUniformBuffer())
    {
        glBindFragDataLocation(m_iProgram, 0, CORE_SHADER_OUTPUT_COLOR_0);
        glBindFragDataLocation(m_iProgram, 1, CORE_SHADER_OUTPUT_COLOR_1);
        glBindFragDataLocation(m_iProgram, 2, CORE_SHADER_OUTPUT_COLOR_2);
        glBindFragDataLocation(m_iProgram, 3, CORE_SHADER_OUTPUT_COLOR_3);
    }

    // link shader-program
    glLinkProgram(m_iProgram);
    m_iStatus = 2;

    // bind global uniform buffer object
    if(Core::Graphics->GetUniformBuffer())
        glUniformBlockBinding(m_iProgram, glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_GLOBAL), CORE_SHADER_BUFFER_GLOBAL_NUM);

    // check for errors
    int iLinked;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iLinked);

#if defined(_CORE_DEBUG_)

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    int iValid;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iValid);
    iLinked += iValid;

#endif

    if(!iLinked)
    {
        // get length of error-log
        int iLength;
        glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get error-log
            char* pcLog = new char[iLength];
            glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, "Shader-Program could not be linked/validated");
            Core::Log->ListStart("Shader-Program Error Log");
            for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
                Core::Log->ListEntry(coreData::Print("(%s)", (*it)->GetPath()));
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }
    return CORE_OK;
}


 // ****************************************************************
// exit the shader-program
coreError coreProgram::Exit()
{
    if(!m_iProgram) return CORE_INVALID_CALL;

    // detach shader objects
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        glDetachShader(m_iProgram, (*it)->GetShader());

    // clear cached identifiers
    m_aiUniform.clear();
    m_aiAttribute.clear();

    // delete shader-program
    glDeleteProgram(m_iProgram);
    m_iProgram = 0;
    m_iStatus  = 1;

    return CORE_OK;
}


// ****************************************************************
// enable the shader-program
// TODO: remove/move linking part somehow
bool coreProgram::Enable()
{
    SDL_assert(m_iStatus);

    // check current shader-program
    if(s_pCurrent == this) return true;

    // link shader-program
    if(!this->IsFinished())
    {
        if(this->Init() != CORE_OK)
            return false;
    }

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iProgram);

    // forward global uniform data without UBO
    if(!Core::Graphics->GetUniformBuffer())
    {
        this->SetUniform(CORE_SHADER_UNIFORM_PERSPECTIVE,     Core::Graphics->GetPerspective(), false);
        this->SetUniform(CORE_SHADER_UNIFORM_ORTHO,           Core::Graphics->GetOrtho(),       false);
        this->SetUniform(CORE_SHADER_UNIFORM_CAMERA,          Core::Graphics->GetCamera(),      false);
        this->SetUniform(CORE_SHADER_UNIFORM_CAM_DIRECTION,   Core::Graphics->GetCamDirection());
        this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_DIRECTION, coreVector3(0.0f,0.0f,-1.0f));
        this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_VALUE,     coreVector4(1.0f,1.0f,1.0f,1.0f));
    }

    return true;
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable()
{
    if(!s_pCurrent) return;

    // reset current shader-program
    s_pCurrent = NULL;
    glUseProgram(0);
}


// ****************************************************************
// add shader object for later attachment
coreProgram* coreProgram::AttachShaderFile(const char* pcPath)
{
    if(!m_iStatus) m_apShader.push_back(Core::Manager::Resource->LoadFile<coreShader>(pcPath));
    return this;
}

coreProgram* coreProgram::AttachShaderLink(const char* pcName)
{
    if(!m_iStatus) m_apShader.push_back(Core::Manager::Resource->LoadLink<coreShader>(pcName));
    return this;
}
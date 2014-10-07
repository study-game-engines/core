﻿//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

std::string  coreShader::s_asGlobalCode[2]; // = "";
coreProgram* coreProgram::s_pCurrent           = NULL;


// ****************************************************************
// create static string lists
template <const char* pcString, coreUint iLength, coreUint iNum> struct sStringList
{
    char aacEntry[iNum][iLength];

    sStringList()noexcept {for(coreUint i = 0; i < iNum; ++i) std::sprintf(aacEntry[i], pcString, i);}
    inline const char* operator [] (const coreUint& i)const {return aacEntry[i];}
};

#define STRING_ARRAY(s,n,v)           \
    extern const char v ## __a[] = s; \
    const sStringList<v ## __a, ARRAY_SIZE(v ## __a), n> v;

STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_POSITION,  CORE_GRAPHICS_LIGHTS,      avLightPosition)
STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_DIRECTION, CORE_GRAPHICS_LIGHTS,      avLightDirection)
STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_VALUE,     CORE_GRAPHICS_LIGHTS,      avLightValue)
STRING_ARRAY(CORE_SHADER_UNIFORM_TEXTURE_2D,      CORE_TEXTURE_UNITS_2D,     avTexture2D)
STRING_ARRAY(CORE_SHADER_UNIFORM_TEXTURE_SHADOW,  CORE_TEXTURE_UNITS_SHADOW, avTextureShadow)
STRING_ARRAY(CORE_SHADER_OUTPUT_COLOR,            CORE_SHADER_OUTPUT_COLORS, avOutColor)


// ****************************************************************
// constructor
coreShader::coreShader()noexcept
: m_iShader (0)
, m_iType   (0)
{
}

coreShader::coreShader(const char* pcCustomCode)noexcept
: m_iShader     (0)
, m_iType       (0)
, m_sCustomCode (pcCustomCode)
{
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
    coreFileUnload Unload(pFile);

    WARN_IF(m_iShader)    return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // extract file extension
    const char* pcExtension = coreData::StrExtension(pFile->GetPath());

    // set shader type
    const char* pcTypeDef;
         if(!std::strcmp(pcExtension, "vs")  || !std::strcmp(pcExtension, "vert")) {m_iType = GL_VERTEX_SHADER;          pcTypeDef = "#define _CORE_VERTEX_SHADER_          (1) \n";}
    else if(!std::strcmp(pcExtension, "tcs") || !std::strcmp(pcExtension, "tesc")) {m_iType = GL_TESS_CONTROL_SHADER;    pcTypeDef = "#define _CORE_TESS_CONTROL_SHADER_    (1) \n";}
    else if(!std::strcmp(pcExtension, "tes") || !std::strcmp(pcExtension, "tese")) {m_iType = GL_TESS_EVALUATION_SHADER; pcTypeDef = "#define _CORE_TESS_EVALUATION_SHADER_ (1) \n";}
    else if(!std::strcmp(pcExtension, "gs")  || !std::strcmp(pcExtension, "geom")) {m_iType = GL_GEOMETRY_SHADER;        pcTypeDef = "#define _CORE_GEOMETRY_SHADER_        (1) \n";}
    else if(!std::strcmp(pcExtension, "fs")  || !std::strcmp(pcExtension, "frag")) {m_iType = GL_FRAGMENT_SHADER;        pcTypeDef = "#define _CORE_FRAGMENT_SHADER_        (1) \n";}
    else
    {
        Core::Log->Warning("Shader (%s) could not be identified (valid extensions: vs, vert, tcs, tesc, tes, tese, gs, geom, fs, frag)", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // check for OpenGL extensions
    if((m_iType == GL_TESS_CONTROL_SHADER || m_iType == GL_TESS_EVALUATION_SHADER) && !CORE_GL_SUPPORT(ARB_tessellation_shader)) return CORE_OK;
    if((m_iType == GL_GEOMETRY_SHADER)                                             && !CORE_GL_SUPPORT(ARB_geometry_shader4))    return CORE_OK;
    
    // load quality level and global shader data
    const char* pcQualityDef = PRINT("#define _CORE_QUALITY_ (%d) \n", Core::Config->GetInt(CORE_CONFIG_GRAPHICS_QUALITY));
    coreShader::__LoadGlobalCode();

    // assemble the shader
    const char* apcData[6] = {s_asGlobalCode[0].c_str(),       pcTypeDef,                   pcQualityDef,                   m_sCustomCode.c_str(),       s_asGlobalCode[1].c_str(),       r_cast<const char*>(pFile->GetData())};
    const int   aiSize[6]  = {(int)s_asGlobalCode[0].length(), (int)std::strlen(pcTypeDef), (int)std::strlen(pcQualityDef), (int)m_sCustomCode.length(), (int)s_asGlobalCode[1].length(), (int)pFile->GetSize()};

    // create and compile the shader
    m_iShader = glCreateShader(m_iType);
    glShaderSource(m_iShader, 6, apcData, aiSize);
    glCompileShader(m_iShader);

    // save properties
    m_sPath = pFile->GetPath();
    m_iSize = aiSize[0] + aiSize[1] + aiSize[2];

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
            Core::Log->Warning("Shader (%s) could not be compiled", pFile->GetPath());
            Core::Log->ListStartWarning("Shader Error Log");
            Core::Log->ListAdd(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Shader (%s:%u) loaded", pFile->GetPath(), m_iShader);
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreError coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    if(!m_sPath.empty()) Core::Log->Info("Shader (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath   = "";
    m_iSize   = 0;
    m_iShader = 0;
    m_iType   = 0;

    return CORE_OK;
}


// ****************************************************************
// load global shader code
// TODO: GLES 3.0 uses "300 es", also version >= 140 is wrong for GLES 3.0
void coreShader::__LoadGlobalCode()
{
    if(!s_asGlobalCode[0].empty()) return;

    // set global shader definitions
    s_asGlobalCode[0].assign(PRINT("#version %.0f \n", CORE_GL_SUPPORT(ARB_uniform_buffer_object) ? Core::Graphics->VersionGLSL()*100.0f : (DEFINED(_CORE_GLES_) ? 100.0f : 110.0f)));
    s_asGlobalCode[1].assign(PRINT("#define CORE_NUM_TEXTURES_2D     (%d) \n", CORE_TEXTURE_UNITS_2D));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_TEXTURES_SHADOW (%d) \n", CORE_TEXTURE_UNITS_SHADOW));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_LIGHTS          (%d) \n", CORE_GRAPHICS_LIGHTS));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_OUTPUTS         (%d) \n", CORE_SHADER_OUTPUT_COLORS));

    // retrieve global shader file
    coreFile* pFile = Core::Manager::Resource->RetrieveFile("data/shaders/global.glsl");
    WARN_IF(!pFile->GetData()) return;

    // copy and unload data
    s_asGlobalCode[1].append(r_cast<const char*>(pFile->GetData()), pFile->GetSize());
    s_asGlobalCode[1].append(1, '\n');
    pFile->UnloadData();

    // reduce memory consumption
    s_asGlobalCode[0].shrink_to_fit();
    s_asGlobalCode[1].shrink_to_fit();
}


// ****************************************************************
// constructor
coreProgram::coreProgram()noexcept
: m_iProgram (0)
, m_iStatus  (CORE_SHADER_NEW)
{
}


// ****************************************************************
// destructor
coreProgram::~coreProgram()
{
    // unload shader-program
    this->Unload();

    // remove all shader objects and attribute locations
    m_apShader.clear();
    m_aiAttribute.clear();
}


// ****************************************************************
// load shader-program
coreError coreProgram::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iCheck = m_Sync.Check(0);
    if(iCheck == CORE_OK) m_iStatus = CORE_SHADER_FINISHED;
    if(iCheck >= CORE_OK) return iCheck;

    // check for shader-program status
    if(m_iStatus < CORE_SHADER_DEFINED) return CORE_BUSY;
    if(m_iStatus > CORE_SHADER_DEFINED) return CORE_INVALID_CALL;
    WARN_IF(m_iProgram)                 return CORE_INVALID_CALL;

    // load all required shader objects
    FOR_EACH(it, m_apShader)
    {
        it->SetActive(true);
        it->GetHandle()->Update();
    }
    FOR_EACH(it, m_apShader)
    {
        if(!it->IsUsable()) 
            return CORE_BUSY;
    }

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    FOR_EACH(it, m_apShader)
        FOR_EACH_SET(et, it+1, m_apShader)
            ASSERT(std::strcmp((*it)->GetPath(), (*et)->GetPath()))

    // check for duplicate attribute locations
    FOR_EACH(it, m_aiAttribute)
        FOR_EACH_SET(et, it+1, m_aiAttribute)
            ASSERT(it->second != et->second && it->second >= 0)

#endif

    // create shader-program
    m_iProgram = glCreateProgram();

    // attach shader objects
    FOR_EACH(it, m_apShader)
    {
        if((*it)->GetShader())
            glAttachShader(m_iProgram, (*it)->GetShader());
    }

    // bind default attribute locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  CORE_SHADER_ATTRIBUTE_TEXTURE);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);

    // bind instancing attribute locations
    if(CORE_GL_SUPPORT(ARB_instanced_arrays) && CORE_GL_SUPPORT(ARB_uniform_buffer_object) && CORE_GL_SUPPORT(ARB_vertex_array_object))
    {
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_TRANSFORM_NUM, CORE_SHADER_ATTRIBUTE_DIV_TRANSFORM);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM,  CORE_SHADER_ATTRIBUTE_DIV_POSITION);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,      CORE_SHADER_ATTRIBUTE_DIV_DATA);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,     CORE_SHADER_ATTRIBUTE_DIV_COLOR);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,  CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM);
    }

    // bind custom attribute locations
    FOR_EACH(it, m_aiAttribute)
    {
        if(it->second >= 0) 
            glBindAttribLocation(m_iProgram, it->second, it->first);
    }

    // bind output locations
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        for(coreByte i = 0; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            glBindFragDataLocation(m_iProgram, i, avOutColor[i]);
    }

    // link shader-program
    glLinkProgram(m_iProgram);
    glUseProgram (m_iProgram);

    // bind texture units
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS_2D; ++i)     glUniform1i(glGetUniformLocation(m_iProgram, avTexture2D    [i]), i);
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS_SHADOW; ++i) glUniform1i(glGetUniformLocation(m_iProgram, avTextureShadow[i]), i + CORE_TEXTURE_SHADOW);

    // bind uniform buffer objects
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const int iTransformBlock = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_TRANSFORM);
        const int iAmbientBlock   = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_AMBIENT);
        if(iTransformBlock >= 0) glUniformBlockBinding(m_iProgram, iTransformBlock, CORE_SHADER_BUFFER_TRANSFORM_NUM);
        if(iAmbientBlock   >= 0) glUniformBlockBinding(m_iProgram, iAmbientBlock,   CORE_SHADER_BUFFER_AMBIENT_NUM);
    }

    // save properties
    FOR_EACH(it, m_apShader)
    {
        m_sPath += (*it).GetHandle()->GetName();
        m_sPath += ":";
        m_iSize += (*it)->GetSize();
    }
    m_sPath.pop_back();

    // check for errors
    int iStatus;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iStatus);
    if(!iStatus)
    {
        this->__WriteLog();
        return CORE_INVALID_DATA;
    }

    // create sync object
    const bool bSync = m_Sync.Create();
    if(!bSync) m_iStatus = CORE_SHADER_FINISHED;

    Core::Log->Info("Program (%s:%u) loaded", m_sPath.c_str(), m_iProgram);
    return bSync ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload shader-program
coreError coreProgram::Unload()
{
    if(!m_iProgram) return CORE_INVALID_CALL;

    // disable still active shader-program
    if(s_pCurrent == this) coreProgram::Disable(true);

    // disable shader objects
    FOR_EACH(it, m_apShader) it->SetActive(false);

    // delete shader-program (with implicit shader object detachment)
    glDeleteProgram(m_iProgram);
    if(!m_sPath.empty()) Core::Log->Info("Program (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath    = "";
    m_iSize    = 0;
    m_iProgram = 0;
    m_iStatus  = CORE_SHADER_DEFINED;

    // clear uniform locations and cache
    m_aiUniform.clear();
    m_avCache.clear();

    return CORE_OK;
}


// ****************************************************************
// enable the shader-program
bool coreProgram::Enable()
{
    ASSERT(m_iStatus)

    // try to update global uniform data
    Core::Graphics->SendTransformation();
    Core::Graphics->SendAmbient();

    // check current shader-program
    if(s_pCurrent == this)                 return true;
    if(m_iStatus  != CORE_SHADER_FINISHED) return false;

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iProgram);

    // forward global uniform data without UBOs
    if(!CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const coreMatrix4 mViewProj = Core::Graphics->GetCamera() * Core::Graphics->GetPerspective();

        // forward transformation data
        this->SendUniform(CORE_SHADER_UNIFORM_VIEWPROJ,    mViewProj,                        false);
        this->SendUniform(CORE_SHADER_UNIFORM_CAMERA,      Core::Graphics->GetCamera(),      false);
        this->SendUniform(CORE_SHADER_UNIFORM_PERSPECTIVE, Core::Graphics->GetPerspective(), false);
        this->SendUniform(CORE_SHADER_UNIFORM_ORTHO,       Core::Graphics->GetOrtho(),       false);
        this->SendUniform(CORE_SHADER_UNIFORM_RESOLUTION,  Core::Graphics->GetViewResolution());

        // forward ambient data
        for(coreByte i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        {
            this->SendUniform(avLightPosition[i],  Core::Graphics->GetLight(i).vPosition);
            this->SendUniform(avLightDirection[i], Core::Graphics->GetLight(i).vDirection);
            this->SendUniform(avLightValue[i],     Core::Graphics->GetLight(i).vValue);
        }
    }

#if defined(_CORE_DEBUG_)

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    int iStatus;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iStatus);
    if(!iStatus)
    {
        this->__WriteLog();
        return false;
    }

#endif

    return true;
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable(const bool& bFull)
{
    // reset current shader-program
    s_pCurrent = NULL;
    if(bFull) glUseProgram(0);
}


// ****************************************************************
// send new uniform 3x3-matrix
void coreProgram::SendUniform(const char* pcName, const coreMatrix3& mMatrix, const bool& bTranspose)
{
    // get uniform location
    const int iLocation = this->GetUniform(pcName);

    // check for cached uniform value
    if(this->CheckCache(iLocation, coreVector4(mMatrix._11 + mMatrix._12 + mMatrix._13,
                                               mMatrix._21 + mMatrix._22 + mMatrix._23, 
                                               mMatrix._31 + mMatrix._32 + mMatrix._33, 0.0f)))
    {
        // send new value
#if defined(_CORE_GLES_)
        if(bTranspose) glUniformMatrix3fv(iLocation, 1, false, mMatrix);
                  else glUniformMatrix3fv(iLocation, 1, false, mMatrix.Transposed());
#else
        glUniformMatrix3fv(iLocation, 1, bTranspose, mMatrix);
#endif
    }
}


// ****************************************************************
// send new uniform 4x4-matrix
void coreProgram::SendUniform(const char* pcName, const coreMatrix4& mMatrix, const bool& bTranspose)
{
    // get uniform location
    const int iLocation = this->GetUniform(pcName);
    if(iLocation >= 0)
    {
        // send new value
#if defined(_CORE_GLES_)
        if(bTranspose) glUniformMatrix4fv(iLocation, 1, false, mMatrix);
                  else glUniformMatrix4fv(iLocation, 1, false, mMatrix.Transposed());
#else
        glUniformMatrix4fv(iLocation, 1, bTranspose, mMatrix);
#endif
    }
}


// ****************************************************************
// write error-log
void coreProgram::__WriteLog()const
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
        Core::Log->Warning("Program (%s) could not be linked or validated", m_sPath.c_str());
        Core::Log->ListStartWarning("Program Error Log");
        {
            FOR_EACH(it, m_apShader)
                Core::Log->ListAdd("%s (%s)", it->GetHandle()->GetName(), (*it)->GetPath());
            Core::Log->ListAdd(pcLog);
        }
        Core::Log->ListEnd();

        SAFE_DELETE_ARRAY(pcLog)
    }
}
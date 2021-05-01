///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SHADER_H_
#define _CORE_GUARD_SHADER_H_

// TODO: document shader-name definitions
// TODO: increase number-performance with constexpr-array int-to-char or char+# (everywhere)
// TODO: CORE_SHADER_STATUS_* enum
// TODO: #include interface
// TODO: split shaders/progams more into Object3D, Object2D and Particle
// TODO: harder integration of instancing
// TODO: (material data in UBO)
// TODO: try to use variable template with uniform cache (C++14)
// TODO: assert-check for new shaders while already finished
// TODO: allow additional shaders and attributes in-between
// TODO: glGetProgramInterface, GetProgramResource[Name/Index] to analyze programs ?
// TODO: check for low-precision sends (colors as byte-array (normalized to 1.0f) instead of float-vector)
// TODO: uber-shaders
// TODO: GLES 3.0 uses "300 es", also version >= 140 is wrong for GLES 3.0
// TODO: <old comment style>

// NOTE: GL below 3.0/130 or without UBO support has always GLSL version 110, GLES is currently always version 100


// ****************************************************************
/* shader definitions */
#define CORE_SHADER_BUFFER_TRANSFORM            "b_Transform"
#define CORE_SHADER_BUFFER_AMBIENT              "b_Ambient"
#define CORE_SHADER_BUFFER_TRANSFORM_NUM        (0u)
#define CORE_SHADER_BUFFER_AMBIENT_NUM          (1u)

#define CORE_SHADER_UNIFORM_VIEWPROJ            "u_m4ViewProj"
#define CORE_SHADER_UNIFORM_CAMERA              "u_m4Camera"
#define CORE_SHADER_UNIFORM_PERSPECTIVE         "u_m4Perspective"
#define CORE_SHADER_UNIFORM_ORTHO               "u_m4Ortho"
#define CORE_SHADER_UNIFORM_RESOLUTION          "u_v4Resolution"
#define CORE_SHADER_UNIFORM_CAMPOSITION         "u_v3CamPosition"

#define CORE_SHADER_UNIFORM_LIGHT_POSITION      "u_aLight[%zu].v4Position"
#define CORE_SHADER_UNIFORM_LIGHT_DIRECTION     "u_aLight[%zu].v4Direction"
#define CORE_SHADER_UNIFORM_LIGHT_VALUE         "u_aLight[%zu].v4Value"

#define CORE_SHADER_UNIFORM_3D_POSITION         "u_v3Position"
#define CORE_SHADER_UNIFORM_3D_SIZE             "u_v3Size"
#define CORE_SHADER_UNIFORM_3D_ROTATION         "u_v4Rotation"
#define CORE_SHADER_UNIFORM_2D_SCREENVIEW       "u_m3ScreenView"
#define CORE_SHADER_UNIFORM_COLOR               "u_v4Color"
#define CORE_SHADER_UNIFORM_TEXPARAM            "u_v4TexParam"
#define CORE_SHADER_UNIFORM_TEXTURE_2D          "u_as2Texture2D[%zu]"
#define CORE_SHADER_UNIFORM_TEXTURE_SHADOW      "u_as2TextureShadow[%zu]"

#define CORE_SHADER_ATTRIBUTE_POSITION          "a_v3RawPosition"
#define CORE_SHADER_ATTRIBUTE_TEXCOORD          "a_v2RawTexCoord"
#define CORE_SHADER_ATTRIBUTE_NORMAL            "a_v3RawNormal"
#define CORE_SHADER_ATTRIBUTE_TANGENT           "a_v4RawTangent"
#define CORE_SHADER_ATTRIBUTE_POSITION_NUM      (0u)
#define CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM      (1u)
#define CORE_SHADER_ATTRIBUTE_NORMAL_NUM        (2u)
#define CORE_SHADER_ATTRIBUTE_TANGENT_NUM       (3u)

#define CORE_SHADER_ATTRIBUTE_DIV_POSITION      "a_v3DivPosition"
#define CORE_SHADER_ATTRIBUTE_DIV_SIZE          "a_v3DivSize"
#define CORE_SHADER_ATTRIBUTE_DIV_ROTATION      "a_v4DivRotation"
#define CORE_SHADER_ATTRIBUTE_DIV_DATA          "a_v3DivData"
#define CORE_SHADER_ATTRIBUTE_DIV_COLOR         "a_v4DivColor"
#define CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM      "a_v4DivTexParam"
#define CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM  (4u)
#define CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM      (5u)
#define CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM  (6u)
#define CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM      (5u)
#define CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM     (7u)
#define CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM  (8u)

#define CORE_SHADER_OUTPUT_COLOR                "o_av4OutColor[%zu]"
#define CORE_SHADER_OUTPUT_COLORS               (2u)

#define CORE_SHADER_OPTION_INSTANCING           "#define _CORE_OPTION_INSTANCING_     (1) \n"
#define CORE_SHADER_OPTION_NO_ROTATION          "#define _CORE_OPTION_NO_ROTATION_    (1) \n"
#define CORE_SHADER_OPTION_NO_EARLY_DEPTH       "#define _CORE_OPTION_NO_EARLY_DEPTH_ (1) \n"

enum coreProgramStatus : coreUint8
{
    CORE_PROGRAM_NEW      = 0u,   // new and empty
    CORE_PROGRAM_DEFINED  = 1u,   // ready for linking
    CORE_PROGRAM_FINISHED = 2u    // texture units bound
};


// ****************************************************************
/* shader class */
class coreShader final : public coreResource
{
private:
    GLuint m_iIdentifier;                    // shader identifier
    GLenum m_iType;                          // shader type (e.g. GL_VERTEX_SHADER)

    coreString m_sCustomCode;                // custom shader code added to the beginning of the shader

    static coreString   s_asGlobalCode[2];   // global shader code (0 = version | 1 = global shader file)
    static coreSpinLock s_GlobalLock;        // spinlock to prevent concurrent initialization of global shader code


public:
    explicit coreShader(const coreChar* pcCustomCode = "")noexcept;
    ~coreShader()final;

    DISABLE_COPY(coreShader)

    /* load and unload shader resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* set object properties */
    inline void SetCustomCode(const coreChar* pcCustomCode) {m_sCustomCode = pcCustomCode;}

    /* get object properties */
    inline const GLuint&   GetIdentifier()const {return m_iIdentifier;}
    inline const GLenum&   GetType      ()const {return m_iType;}
    inline const coreChar* GetCustomCode()const {return m_sCustomCode.c_str();}


private:
    /* load global shader code */
    static void __LoadGlobalCode();

    /* reduce shader code size */
    static void __ReduceCodeSize(coreString* OUTPUT psCode);
};


// ****************************************************************
/* shader resource access type */
using coreShaderPtr = coreResourcePtr<coreShader>;


// ****************************************************************
/* shader-program class */
class coreProgram final : public coreResource
{
private:
    GLuint m_iIdentifier;                             // shader-program identifier

    coreList<coreShaderPtr>       m_apShader;         // attached shader objects
    coreList<coreResourceHandle*> m_apShaderHandle;   // raw shader resource handles (to preserve while unloaded)
    coreProgramStatus m_eStatus;                      // current status

    coreMapStr<coreInt8> m_aiUniform;                 // uniform locations
    coreMapStrFull<coreInt8> m_aiAttribute;           // attribute locations
    coreMap<coreInt8, coreVector4> m_avCache;         // cached uniform values

    coreSync m_Sync;                                  // sync object for asynchronous shader-program loading

    static coreProgram* s_pCurrent;                   // currently active shader-program


public:
    coreProgram()noexcept;
    ~coreProgram()final;

    DISABLE_COPY(coreProgram)

    /* load and unload shader-program */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* enable and disable the shader-program */
    coreBool Enable();
    static void Disable(const coreBool bFull);

    /* execute a compute shader-program */
    void DispatchCompute(const coreUint32 iGroupsX, const coreUint32 iGroupsY, const coreUint32 iGroupsZ);

    /* define shader objects and attribute locations */
    inline coreProgram* AttachShader (const coreShaderPtr&  pShader)                          {if(!m_eStatus)  m_apShaderHandle.push_back(pShader.GetHandle());                             return this;}
    inline coreProgram* AttachShader (const coreHashString& sName)                            {if(!m_eStatus)  m_apShaderHandle.push_back(Core::Manager::Resource->Get<coreShader>(sName)); return this;}
    inline coreProgram* BindAttribute(const coreHashString& sName, const coreUint8 iLocation) {if(!m_eStatus)  m_aiAttribute[sName] = iLocation;                                            return this;}
    inline void Finish ()                                                                     {if(!m_eStatus) {m_apShader.reserve(m_apShaderHandle.size()); m_apShaderHandle.shrink_to_fit(); m_aiAttribute.shrink_to_fit(); m_eStatus = CORE_PROGRAM_DEFINED;}}
    inline void Restart()                                                                     {this->Unload(); m_apShader.clear();                          m_apShaderHandle.clear();         m_aiAttribute.clear();         m_eStatus = CORE_PROGRAM_NEW;}

    /* send new uniform values */
    inline void SendUniform(const coreHashString& sName, const coreInt32    iInt)    {const coreInt8 iLocation = this->RetrieveUniform(sName); if((iLocation >= 0) && this->CheckCache(iLocation, coreVector4(I_TO_F(iInt), 0.0f, 0.0f, 0.0f))) glUniform1i (iLocation,    iInt);}
    inline void SendUniform(const coreHashString& sName, const coreFloat    fFloat)  {const coreInt8 iLocation = this->RetrieveUniform(sName); if((iLocation >= 0) && this->CheckCache(iLocation, coreVector4(fFloat,       0.0f, 0.0f, 0.0f))) glUniform1f (iLocation,    fFloat);}
    inline void SendUniform(const coreHashString& sName, const coreVector2& vVector) {const coreInt8 iLocation = this->RetrieveUniform(sName); if((iLocation >= 0) && this->CheckCache(iLocation, coreVector4(vVector,      0.0f, 0.0f)))       glUniform2fv(iLocation, 1, vVector);}
    inline void SendUniform(const coreHashString& sName, const coreVector3& vVector) {const coreInt8 iLocation = this->RetrieveUniform(sName); if((iLocation >= 0) && this->CheckCache(iLocation, coreVector4(vVector,      0.0f)))             glUniform3fv(iLocation, 1, vVector);}
    inline void SendUniform(const coreHashString& sName, const coreVector4& vVector) {const coreInt8 iLocation = this->RetrieveUniform(sName); if((iLocation >= 0) && this->CheckCache(iLocation, vVector))                                     glUniform4fv(iLocation, 1, vVector);}
    void        SendUniform(const coreHashString& sName, const coreMatrix2& mMatrix, const coreBool bTranspose);
    void        SendUniform(const coreHashString& sName, const coreMatrix3& mMatrix, const coreBool bTranspose);
    void        SendUniform(const coreHashString& sName, const coreMatrix4& mMatrix, const coreBool bTranspose);

    /* retrieve uniform and attribute locations */
    inline const coreInt8& RetrieveUniform  (const coreHashString& sName) {if(!m_aiUniform  .count(sName)) {ASSERT(m_eStatus >= CORE_PROGRAM_FINISHED && s_pCurrent == this) m_aiUniform  .emplace(sName, glGetUniformLocation(m_iIdentifier, sName.GetString()));} ASSERT(m_aiUniform  .at(sName) >= -1) return m_aiUniform  .at(sName);}
    inline const coreInt8& RetrieveAttribute(const coreHashString& sName) {if(!m_aiAttribute.count(sName)) {ASSERT(m_eStatus >= CORE_PROGRAM_FINISHED && s_pCurrent == this) m_aiAttribute.emplace(sName, glGetAttribLocation (m_iIdentifier, sName.GetString()));} ASSERT(m_aiAttribute.at(sName) >= -1) return m_aiAttribute.at(sName);}

    /* check for cached uniform values */
    inline coreBool CheckCache(const coreInt8 iLocation, const coreVector4& vVector) {if(m_avCache.count(iLocation)) {if(m_avCache.at(iLocation) == vVector) return false;} m_avCache[iLocation] = vVector; return true;}

    /* get object properties */
    inline const GLuint& GetIdentifier()const                       {return m_iIdentifier;}
    inline coreShader*   GetShader    (const coreUintW iIndex)const {ASSERT(iIndex < m_apShaderHandle.size()) return d_cast<coreShader*>(m_apShaderHandle[iIndex]->GetRawResource());}
    inline coreUintW     GetNumShaders()const                       {return m_apShaderHandle.size();}

    /* get currently active shader-program */
    static inline coreProgram* GetCurrent() {return s_pCurrent;}


private:
    /* write debug information to log file */
    void __WriteLog()const;
    void __WriteInterface()const;
};


// ****************************************************************
/* shader-program resource access type */
using coreProgramPtr = coreResourcePtr<coreProgram>;


#endif /* _CORE_GUARD_SHADER_H_ */
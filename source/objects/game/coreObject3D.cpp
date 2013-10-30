//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// define model through resource file
const coreModelPtr& coreObject3D::DefineModelFile(const char* pcPath)
{
    // set and return the model resource pointer
    m_pModel = Core::Manager::Resource->LoadFile<coreModel>(pcPath);
    return m_pModel;
}


// ****************************************************************
// define model through linked resource
const coreModelPtr& coreObject3D::DefineModelLink(const char* pcName)
{
    // set and return the model resource pointer
    m_pModel = Core::Manager::Resource->LoadLink<coreModel>(pcName);
    return m_pModel;
}


// ****************************************************************
// undefine the visual appearance
void coreObject3D::Undefine()
{
    // reset all resource and memory pointer
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
    m_pModel   = NULL;
}


// ****************************************************************
// render the 3d-object
void coreObject3D::Render(const coreProgramShr& pProgram, const bool& bTextured)
{
    // enable the shader-program
    if(!pProgram) return;
    if(!pProgram->Enable()) return;

    // update all object uniforms
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TRANSFORM,  m_mTransform, false);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_COLOR,      m_vColor);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEX_SIZE,   m_vTexSize);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEX_OFFSET, m_vTexOffset);

    if(bTextured)
    {
        // enable all active textures
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
            if(m_apTexture[i].IsActive()) m_apTexture[i]->Enable(i);
    }
    else coreTexture::DisableAll();

    // render the model
    m_pModel->RenderList();
}


// ****************************************************************
// move the 3d-object
void coreObject3D::Move()
{
    if(m_iUpdate & 1)
    {
        if(m_iUpdate & 2)
        {
            // update rotation matrix
            m_mRotation = coreMatrix::Orientation(m_vDirection, m_vOrientation);
        }

        // update transformation matrix
        m_mTransform = coreMatrix::Scaling(m_vSize) * m_mRotation *
                       coreMatrix::Translation(m_vPosition);

        // reset the update status
        m_iUpdate = 0;
    }
}
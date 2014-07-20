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
/* constructor */
coreResourceRelation::coreResourceRelation()noexcept
{
    // add object to resource manager
    Core::Manager::Resource->BindRelation(this);
}


// ****************************************************************
/* destructor */
coreResourceRelation::~coreResourceRelation()
{
    // remove object from resource manager
    Core::Manager::Resource->UnbindRelation(this);
}


// ****************************************************************
/* constructor */
coreResourceManager::coreResourceManager()noexcept
: m_iLock   (0)
, m_bActive (false)
{
    // start up the resource manager
    this->Reset(CORE_RESOURCE_RESET_INIT);

    Core::Log->Info("Resource Manager created");
}


// ****************************************************************
/* destructor */
coreResourceManager::~coreResourceManager()
{
    ASSERT(!m_apRelation.size())

    // shut down the resource manager
    this->Reset(CORE_RESOURCE_RESET_EXIT);

    // delete resource handles
    FOR_EACH(it, m_apHandle) SAFE_DELETE(it->second)

    // delete resource files
    FOR_EACH(it, m_apArchive)    SAFE_DELETE(it->second)
    FOR_EACH(it, m_apDirectFile) SAFE_DELETE(it->second)

    // clear memory
    m_apHandle.clear();
    m_apArchive.clear();
    m_apDirectFile.clear();
    m_apRelation.clear();

    Core::Log->Info("Resource Manager destroyed");
}


// ****************************************************************
/* retrieve archive */
coreArchive* coreResourceManager::RetrieveArchive(const char* pcPath)
{
    // check for existing archive
    if(m_apArchive.count(pcPath)) return m_apArchive[pcPath];

    // load new archive
    coreArchive* pNewArchive = new coreArchive(pcPath);
    m_apArchive[pcPath] = pNewArchive;

    ASSERT(pNewArchive->GetNumFiles())
    return pNewArchive;
}


// ****************************************************************
/* retrieve resource file */
coreFile* coreResourceManager::RetrieveFile(const char* pcPath)
{
    // try to open direct resource file first
    if(!coreData::FileExists(pcPath))
    {
        // check archives
        FOR_EACH(it, m_apArchive)
        {
            coreFile* pFile = it->second->GetFile(pcPath);
            if(pFile) return pFile;
        }

        // resource file not found
        ASSERT(false)
    }

    // check for existing direct resource file
    if(m_apDirectFile.count(pcPath)) return m_apDirectFile[pcPath];

    // load new direct resource file
    coreFile* pNewFile = new coreFile(pcPath);
    m_apDirectFile[pcPath] = pNewFile;

    return pNewFile;
}


// ****************************************************************
/* reset all resources and relation-objects */
void coreResourceManager::Reset(const coreResourceReset& bInit)
{
    const bool bActive = bInit ? true : false;

    // check and set current status
    if(m_bActive == bActive) return;
    m_bActive = bActive;

    if(m_bActive)
    {
        // start up relation-objects
        FOR_EACH(it, m_apRelation)
            (*it)->__Reset(CORE_RESOURCE_RESET_INIT);

        // start resource thread
        if(Core::Graphics->GetResourceContext())
            this->StartThread("resource_thread");
    }
    else
    {
        // kill resource thread
        if(Core::Graphics->GetResourceContext())
            this->KillThread();

        // shut down relation-objects
        FOR_EACH(it, m_apRelation)
            (*it)->__Reset(CORE_RESOURCE_RESET_EXIT);

        // unload all resources
        FOR_EACH(it, m_apHandle)
            it->second->Nullify();
    }
}


// ****************************************************************
/* init resource thread */
int coreResourceManager::__InitThread()
{
    // assign resource context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphics->GetResourceContext()))
        Core::Log->Error("Resource context could not be assigned to resource thread (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Resource context assigned to resource thread");

    // init GLEW on resource context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK)
        Core::Log->Error("GLEW could not be initialized on resource context (GLEW: %s)", glewGetErrorString(iError));
    else Core::Log->Info("GLEW initialized on resource context (%s)", glewGetString(GLEW_VERSION));

    // enable OpenGL debug output
    Core::Log->DebugOpenGL();

    return 0;
}


// ****************************************************************
/* run resource thread */
int coreResourceManager::__RunThread()
{
    // check for current status
    if(m_bActive)
    {
        SDL_AtomicLock(&m_iLock);
        {
            for(coreUint i = 0; i < m_apHandle.size(); ++i)
            {
                // update resource handle
                if(m_apHandle[i]->Update())
                {
                    // allow changes during iteration
                    SDL_AtomicUnlock(&m_iLock);
                    SDL_AtomicLock(&m_iLock);
                }
            }
        }
        SDL_AtomicUnlock(&m_iLock);
    }
    return 0;
}


// ****************************************************************
/* exit resource thread */
void coreResourceManager::__ExitThread()
{
    // dissociate resource context from resource thread
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);
}
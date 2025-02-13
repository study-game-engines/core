///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATABUFFER_H_
#define _CORE_GUARD_DATABUFFER_H_

// TODO 3: implement read and copy operations (currently only static and write/dynamic)
// TODO 3: improve vertex attribute array enable/disable for OGL (ES) 2.0 without vertex array objects, cache current enabled arrays, may need reset
// TODO 5: <old comment style>

// NOTE: superior objects have to handle resource-resets, to refill the buffers


// ****************************************************************
/* data buffer definitions */
#define CORE_VERTEXBUFFER_ATTRIBUTES (16u)   // max number of vertex attribute locations

enum coreDataBufferStorage : coreUint8
{
    CORE_DATABUFFER_STORAGE_STATIC  = 0x01u,   // fast static buffer (STATIC_DRAW)
    CORE_DATABUFFER_STORAGE_DYNAMIC = 0x02u,   // writable dynamic buffer (DYNAMIC_DRAW), persistent mapped if supported
    CORE_DATABUFFER_STORAGE_STREAM  = 0x04u    // writable temporary buffer (STREAM_DRAW)
};
ENABLE_BITWISE(coreDataBufferStorage)

enum coreDataBufferMap : coreUint8
{
    CORE_DATABUFFER_MAP_INVALIDATE_ALL = GL_MAP_INVALIDATE_BUFFER_BIT,                             // invalidate complete buffer   (best for complete updating)
    CORE_DATABUFFER_MAP_UNSYNCHRONIZED = GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT   // map and unmap unsynchronized (best for partial updating)
};


// ****************************************************************
/* data buffer class */
class coreDataBuffer
{
private:
    GLuint m_iIdentifier;                       // data buffer identifier
    coreDataBufferStorage m_eStorageType;       // storage type

    GLenum     m_iTarget;                       // buffer target (e.g. GL_ARRAY_BUFFER)
    coreUint32 m_iSize;                         // data size in bytes
    coreUint32 m_iFallbackSize;                 // current size of fallback memory (if regular mapping is not supported)

    coreByte*  m_pPersistentBuffer;             // pointer to persistent mapped buffer (or fallback memory)
    coreUint32 m_iMapOffset;                    // current mapping offset
    coreUint32 m_iMapLength;                    // current mapping length

    coreSync m_Sync;                            // sync object for reliable access (unsynchronized, persistent mapped)

    static coreMap<GLenum, GLuint> s_aiBound;   // data buffer objects currently associated with buffer targets <target, identifier>


public:
    constexpr coreDataBuffer()noexcept;
    inline coreDataBuffer(coreDataBuffer&& m)noexcept;
    ~coreDataBuffer();

    /* assignment operations */
    coreDataBuffer& operator = (coreDataBuffer&& m)noexcept;

    /* control the data buffer object */
    void Create(const GLenum iTarget, const coreUint32 iSize, const void* pData, const coreDataBufferStorage eStorageType);
    void Delete();

    /* bind and unbind the data buffer object */
    inline void        Bind  ()const                                          {ASSERT(m_iIdentifier) coreDataBuffer::Bind(m_iTarget, m_iIdentifier);}
    static inline void Bind  (const GLenum iTarget, const GLuint iIdentifier) {if(s_aiBound.count(iTarget)) {if(s_aiBound.at(iTarget) == iIdentifier) return;} s_aiBound[iTarget] = iIdentifier; glBindBuffer(iTarget, iIdentifier);}
    static inline void Unbind(const GLenum iTarget, const coreBool bFull)     {if(bFull) coreDataBuffer::Bind(iTarget, 0u); else s_aiBound[iTarget] = 0u;}

    /* modify buffer memory */
    RETURN_RESTRICT coreByte* Map  (const coreUint32 iOffset, const coreUint32 iLength, const coreDataBufferMap eMapType);
    void                      Unmap();
    coreStatus                Copy (const coreUint32 iReadOffset, const coreUint32 iWriteOffset, const coreUint32 iLength, coreDataBuffer* OUTPUT pDestination)const;

    /* protect buffer memory up to now */
    inline void Synchronize() {if(CORE_GL_SUPPORT(ARB_map_buffer_range)) m_Sync.Create();}

    /* reset content of the data buffer object */
    coreStatus Clear(const coreTextureSpec& oTextureSpec, const void* pData);
    coreStatus Invalidate();

    /* check for current buffer status */
    inline coreBool IsWritable()const {return !HAS_FLAG(m_eStorageType, CORE_DATABUFFER_STORAGE_STATIC);}
    inline coreBool IsMapped  ()const {return (m_iMapLength  != 0u);}
    inline coreBool IsValid   ()const {return (m_iIdentifier != 0u);}

    /* get object properties */
    inline const GLuint&                GetIdentifier ()const {return m_iIdentifier;}
    inline const coreDataBufferStorage& GetStorageType()const {return m_eStorageType;}
    inline const GLenum&                GetTarget     ()const {return m_iTarget;}
    inline const coreUint32&            GetSize       ()const {return m_iSize;}
};


// ****************************************************************
/* vertex buffer class */
class coreVertexBuffer final : public coreDataBuffer
{
private:
    /* vertex attribute array structure */
    struct coreAttribute final
    {
        GLenum    iType;           // component type (e.g. GL_FLOAT)
        coreUint8 iLocation;       // attribute location
        coreUint8 iComponents;     // number of components
        coreBool  bInteger;        // pure integer attribute
        coreUint8 iBufferOffset;   // offset within the vertex buffer (multiplied with number of vertices)
        coreUint8 iVertexOffset;   // offset within the vertex
    };

    /* vertex stream structure */
    struct coreStream final
    {
        coreUint8 iBinding;   // vertex buffer binding point index
        coreUint8 iStride;    // stride between vertices
    };


private:
    coreUint32 m_iNumVertices;                     // number of vertices
    coreUint8  m_iVertexSize;                      // size of each vertex in bytes

    coreList<coreAttribute>        m_aAttribute;   // defined vertex attribute arrays
    coreMap<coreUint8, coreStream> m_aStream;      // accumulated vertex streams <buffer offset, data>


public:
    coreVertexBuffer()noexcept;
    coreVertexBuffer(coreVertexBuffer&& m)noexcept;
    ~coreVertexBuffer();

    /* assignment operations */
    coreVertexBuffer& operator = (coreVertexBuffer&& m)noexcept;

    /* control the vertex buffer object */
    void Create(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage eStorageType);
    void Delete();

    /* define and activate the vertex structure */
    void DefineAttribute(const coreUint8 iLocation, const coreUint8 iComponents, const GLenum iType, const coreUint8 iSize, const coreBool bInteger, const coreUint8 iBufferOffset, const coreUint8 iVertexOffset);
    void Activate       (const coreUint8 iDivisor);

    /* get object properties */
    inline const coreUint32& GetNumVertices()const {return m_iNumVertices;}
    inline const coreUint8&  GetVertexSize ()const {return m_iVertexSize;}
};


// ****************************************************************
/* constructor */
constexpr coreDataBuffer::coreDataBuffer()noexcept
: m_iIdentifier       (0u)
, m_eStorageType      (CORE_DATABUFFER_STORAGE_STATIC)
, m_iTarget           (0u)
, m_iSize             (0u)
, m_iFallbackSize     (0u)
, m_pPersistentBuffer (NULL)
, m_iMapOffset        (0u)
, m_iMapLength        (0u)
, m_Sync              ()
{
}

inline coreDataBuffer::coreDataBuffer(coreDataBuffer&& m)noexcept
: m_iIdentifier       (m.m_iIdentifier)
, m_eStorageType      (m.m_eStorageType)
, m_iTarget           (m.m_iTarget)
, m_iSize             (m.m_iSize)
, m_iFallbackSize     (m.m_iFallbackSize)
, m_pPersistentBuffer (m.m_pPersistentBuffer)
, m_iMapOffset        (m.m_iMapOffset)
, m_iMapLength        (m.m_iMapLength)
, m_Sync              (std::move(m.m_Sync))
{
    m.m_iIdentifier = 0u;
}


#endif /* _CORE_GUARD_DATABUFFER_H_ */
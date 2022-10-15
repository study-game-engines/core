///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MUSIC_H_
#define _CORE_GUARD_MUSIC_H_

// TODO 3: player: callback when music track changes (or return true is enough probably)
// TODO 5: player: own (cached) parameters like volume, pitch - forwarded to music files
// TODO 4: player: reorder properties, SwitchBox is similar
// TODO 3: threaded music update (->coreMusicPlayer)
// TODO 4: split up: coreMusicFile.cpp, coreMusicPlayer.cpp
// TODO 5: <old comment style>
// TODO 3: on play, only stream first buffer, and move streaming other buffers to next iterations (queue them empty ?) (on update still keep loop, to handle possible catch-up, to not decay to single-buffering and stuttering if chunks-per-iteration > 1.0)


// ****************************************************************
/* music definitions */
#define CORE_MUSIC_CHUNK   (0x1000u)   // size of a music stream chunk in floats/shorts (on speed 1.0x)
#define CORE_MUSIC_BUFFERS (3u)        // number of sound buffers (with chunks)

enum coreMusicRepeat : coreUint8
{
    CORE_MUSIC_SINGLE_NOREPEAT = 0u,
    CORE_MUSIC_SINGLE_REPEAT   = 1u,
    CORE_MUSIC_ALL_NOREPEAT    = 2u,
    CORE_MUSIC_ALL_REPEAT      = 3u
};


// ****************************************************************
/* music class */
class coreMusic final
{
private:
    ALuint m_aiBuffer[CORE_MUSIC_BUFFERS];   // sound buffers for streaming
    ALuint m_iSource;                        // currently used audio source

    coreFloat m_fVolume;                     // current volume
    coreFloat m_fPitch;                      // current playback speed
    coreBool  m_bLoop;                       // loop status
    coreBool  m_bStatus;                     // playback status

    coreFile*   m_pFile;                     // file object with streaming data
    stb_vorbis* m_pStream;                   // music stream object

    stb_vorbis_info    m_Info;               // format of the music file
    stb_vorbis_comment m_Comment;            // meta-information
    coreUint32         m_iMaxSample;         // length of the music track (in samples)
    coreFloat          m_fMaxTime;           // length of the music track (in seconds)


public:
    explicit coreMusic(const coreChar* pcPath)noexcept;
    explicit coreMusic(coreFile*       pFile)noexcept;
    ~coreMusic();

    DISABLE_COPY(coreMusic)

    /* update the music object */
    coreBool Update();

    /* control playback */
    coreStatus Play ();
    void       Stop ();
    void       Pause();
    inline const coreBool& IsPlaying()const {return m_bStatus;}

    /* set various audio source properties */
    inline void SetVolume(const coreFloat fVolume) {if(m_iSource && (m_fVolume != fVolume)) Core::Audio->UpdateSource(m_iSource, fVolume); m_fVolume = fVolume; ASSERT(fVolume >= 0.0f)}
    inline void SetPitch (const coreFloat fPitch)  {if(m_iSource && (m_fPitch  != fPitch))  alSourcef(m_iSource, AL_PITCH, fPitch);        m_fPitch  = fPitch;  ASSERT(fPitch  >= 0.0f)}
    inline void SetLoop  (const coreBool  bLoop)   {m_bLoop = bLoop; ASSERT(!m_iSource)}

    /* change and retrieve current music track position */
    inline void Rewind    ()                          {stb_vorbis_seek_start(m_pStream);}
    inline void SeekSample(const coreUint32 iSamples) {stb_vorbis_seek(m_pStream, iSamples); ASSERT(iSamples <= m_iMaxSample)}
    inline void SeekFactor(const coreDouble dFactor)  {this->SeekSample(F_TO_UI   (dFactor   * coreDouble(m_iMaxSample)));}
    inline void SeekTime  (const coreFloat  fSeconds) {this->SeekFactor(coreDouble(fSeconds) / coreDouble(m_fMaxTime));}
    inline coreUint32 TellSample()const               {return stb_vorbis_get_sample_offset(m_pStream);}
    inline coreDouble TellFactor()const               {return coreDouble(this->TellSample()) / coreDouble(m_iMaxSample);}
    inline coreFloat  TellTime  ()const               {return coreFloat (this->TellFactor()  * coreDouble(m_fMaxTime));}

    /* get meta-information */
    const coreChar* GetComment(const coreChar* pcName)const;
    inline const coreChar* GetArtist()const {return this->GetComment("ARTIST");}
    inline const coreChar* GetTitle ()const {return this->GetComment("TITLE");}

    /* get object properties */
    inline const coreChar*        GetPath     ()const {return m_pFile ? m_pFile->GetPath() : "";}
    inline const stb_vorbis_info& GetInfo     ()const {return m_Info;}
    inline const coreUint32&      GetMaxSample()const {return m_iMaxSample;}
    inline const coreFloat&       GetMaxTime  ()const {return m_fMaxTime;}


private:
    /* read from music stream and update sound buffer */
    coreBool  __Stream    (const ALuint  iBuffer);
    coreUintW __StreamList(const ALuint* piBuffer, const coreUintW iCount);
};


// ****************************************************************
/* music-player class */
class coreMusicPlayer final
{
private:
    coreList<coreMusic*> m_apMusic;      // music objects
    coreMusic* m_pEmptyMusic;            // empty music object

    coreList<coreMusic*> m_apSequence;   // playback sequence
    coreMusicRepeat m_eRepeat;           // repeat behavior

    coreTimer  m_FadeTimer;              // timer for a transition between two music objects
    coreMusic* m_pFadePrevious;          // previous music object during a transition

    coreMusic* m_pCurMusic;              // current music object
    coreUintW  m_iCurIndex;              // sequence-index of the current music object


public:
    coreMusicPlayer()noexcept;
    ~coreMusicPlayer();

    DISABLE_COPY(coreMusicPlayer)

    /* update the music-player */
    coreBool Update();

    /* control playback behavior */
    void Order();
    void Shuffle();

    /* manage music objects */
    coreStatus AddMusicFile   (const coreChar* pcPath);
    coreStatus AddMusicArchive(const coreChar* pcPath, const coreChar* pcFilter);
    coreStatus AddMusicFolder (const coreChar* pcPath, const coreChar* pcFilter);
    coreStatus DeleteMusic    (const coreUintW iIndex);
    void ClearMusic();

    /* switch current music object */
    void Select(const coreUintW iIndex);
    coreBool Next    ();
    coreBool Previous();

    /* access music objects */
    inline const coreMusic* GetMusic(const coreUintW iIndex)const {WARN_IF(iIndex >= m_apMusic.size()) return m_pEmptyMusic; return m_apMusic[iIndex];}
    inline       coreMusic* Control()const                        {ASSERT(m_pCurMusic != m_pEmptyMusic) return m_pCurMusic;}

    /* set object properties */
    inline void SetRepeat(const coreMusicRepeat eRepeat) {m_eRepeat = eRepeat;}
    inline void SetFade  (const coreFloat       fTime)   {if(fTime) m_FadeTimer.SetSpeed(RCP(fTime)); else {m_FadeTimer.SetValue(1.0f); m_FadeTimer.SetSpeed(0.0f);}}

    /* get object properties */
    inline       coreUintW        GetNumMusic()const {return m_apMusic.size();}
    inline const coreUintW&       GetCurIndex()const {return m_iCurIndex;}
    inline const coreMusicRepeat& GetRepeat  ()const {return m_eRepeat;}


private:
    /* add music object */
    coreStatus __AddMusic(coreFile* pFile);
};


#endif /* _CORE_GUARD_MUSIC_H_ */
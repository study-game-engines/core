//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SELECT_H_
#define _CORE_GUARD_SELECT_H_

// TODO: create a dynamic-sized version with std::vector ? (replacement in coreRand and coreParticleSystem)


// ****************************************************************
/* extended array class */
template <typename T, coreUint iSize> class coreSelect final
{
private:
    std::array<T, iSize> m_atItem;   //!< static item container
    coreUint m_iCurIndex;            //!< index of the current item


public:
    coreSelect()noexcept;
    coreSelect(const coreSelect<T, iSize>& c)noexcept;
    coreSelect(coreSelect<T, iSize>&&      m)noexcept;

    /*! assignment operations */
    //! @{
    coreSelect<T, iSize>& operator = (coreSelect<T, iSize> o)noexcept;
    template <typename S, coreUint iSize2> friend void swap(coreSelect<S, iSize2>& a, coreSelect<S, iSize2>& b)noexcept;
    //! @}

    /*! switch current item */
    //! @{
    inline void Select(const coreUint& iIndex) {ASSERT(iIndex < iSize)     m_iCurIndex = CLAMP(iIndex, 0u, iSize - 1);}
    inline void Next    ()                     {if(++m_iCurIndex >= iSize) m_iCurIndex = 0u;}
    inline void Previous()                     {if(--m_iCurIndex >= iSize) m_iCurIndex = iSize - 1;}
    //! @}

    /*! access reference to current item */
    //! @{
    inline       T& Current()      {return m_atItem[m_iCurIndex];}
    inline const T& Current()const {return m_atItem[m_iCurIndex];}
    //! @}

    /*! modify the item container */
    //! @{
    inline void Fill(const T& tValue) {m_atItem.fill(tValue);}
    //! @}

    /*! access item container directly */
    //! @{
    inline operator       T* ()                    {return &m_atItem.front();}
    inline operator const T* ()const               {return &m_atItem.front();}
    inline       std::array<T, iSize>* List()      {return &m_atItem;}
    inline const std::array<T, iSize>* List()const {return &m_atItem;}
    //! @}
};


// ****************************************************************
/* constructor */
template <typename T, coreUint iSize> coreSelect<T, iSize>::coreSelect()noexcept
: m_iCurIndex (0)
{
}

template <typename T, coreUint iSize> coreSelect<T, iSize>::coreSelect(const coreSelect<T, iSize>& c)noexcept
: m_atItem    (c.m_atItem)
, m_iCurIndex (c.m_iCurIndex)
{
}

template <typename T, coreUint iSize> coreSelect<T, iSize>::coreSelect(coreSelect<T, iSize>&& m)noexcept
: m_atItem    (std::move(m.m_atItem))
, m_iCurIndex (m.m_iCurIndex)
{
}


// ****************************************************************
/* assignment operations */
template <typename T, coreUint iSize> coreSelect<T, iSize>& coreSelect<T, iSize>::operator = (coreSelect<T, iSize> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename S, coreUint iSize2> void swap(coreSelect<S, iSize2>& a, coreSelect<S, iSize2>& b)noexcept
{
    using std::swap;
    swap(a.m_atItem,    b.m_atItem);
    swap(a.m_iCurIndex, b.m_iCurIndex);
}


#endif /* _CORE_GUARD_SELECT_H_ */
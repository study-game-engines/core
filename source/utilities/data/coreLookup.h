//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOOKUP_H_
#define _CORE_GUARD_LOOKUP_H_

// TODO: implement quicksort and binary search
// TODO: measure performance with high amount of entries (>200), but should not be as good as (unordered_)map
// TODO: radix-tree, crit-bit-tree, splay-tree ?
// TODO: caching system may still cause problems (e.g. coreLookupStr in combination with PRINT)


// ****************************************************************
/* generic lookup container class */
template <typename K, typename I, typename T> class coreLookupGen
{
protected:
    /*! internal types */
    typedef std::pair<K, T>                   coreEntry;
    typedef std::vector<coreEntry>            coreList;
    typedef typename coreList::iterator       coreIterator;
    typedef typename coreList::const_iterator coreConstIterator;


protected:
    coreList m_aList;      //!< vector-list with pair-values

    coreEntry* m_pCache;   //!< last requested entry
    I m_CacheKey;          //!< key to the last requested entry


public:
    coreLookupGen()noexcept;
    coreLookupGen(const coreLookupGen<K, I, T>& c)noexcept;
    coreLookupGen(coreLookupGen<K, I, T>&&      m)noexcept;

    /*! assignment operator */
    //! @{
    coreLookupGen<K, I, T>& operator = (coreLookupGen<K, I, T> o)noexcept;
    template <typename R, typename O, typename S> friend void swap(coreLookupGen<R, O, S>& a, coreLookupGen<R, O, S>& b)noexcept;
    //! @}

    /*! access specific entry */
    //! @{
    const T&    at (const I& Key)const;
    T& operator [] (const I& Key);
    //! @}

    /*! check number of existing entries */
    //! @{
    inline coreUint count(const I& Key)      {return this->__check(this->__retrieve(Key)) ? 1 : 0;}
    inline coreUint count(const I& Key)const {return this->__check(this->__retrieve(Key)) ? 1 : 0;}
    inline coreUint size ()const             {return m_aList.size();}
    inline bool     empty()const             {return m_aList.empty();}
    //! @}

    /*! control memory consumption */
    //! @{
    inline void     reserve(const coreUint& iReserve) {m_aList.reserve(iReserve);}
    inline void     shrink_to_fit()                   {m_aList.shrink_to_fit();}
    inline coreUint capacity()const                   {return m_aList.capacity();}
    //! @}

    /*! remove existing entries */
    //! @{
    bool                erase(const I& Key);
    inline coreIterator erase(const coreConstIterator& Iterator) {this->__cache_clear(); return m_aList.erase(Iterator);}
    inline void         clear()                                  {this->__cache_clear(); m_aList.clear();}
    //! @}

    /*! retrieve internal iterator */
    //! @{
    inline coreIterator      begin()      {return m_aList.begin();}
    inline coreConstIterator begin()const {return m_aList.begin();}
    inline coreIterator      end  ()      {return m_aList.end();}
    inline coreConstIterator end  ()const {return m_aList.end();}
    //! @}


protected:
    /*! check for valid iterator */
    //! @{
    inline bool __check(const coreIterator&      it)const {return (it != m_aList.end()) ? true : false;}
    inline bool __check(const coreConstIterator& it)const {return (it != m_aList.end()) ? true : false;}
    //! @}

    /*! cache last requested entry */
    //! @{
    inline void __cache_set(coreEntry* pEntry, const I& Key) {m_pCache = pEntry; m_CacheKey = Key;}
    inline void __cache_clear()                              {m_pCache = NULL;   m_CacheKey = (I)NULL;}
    inline bool __cache_try(const I& Key)const               {ASSERT(!m_pCache || ((m_CacheKey == Key) == (m_pCache->first == Key))) return (m_pCache && m_CacheKey == Key) ? true : false;}
    //! @}

    /*! retrieve iterator */
    //! @{
    coreIterator      __retrieve(const I& Key)hot_func;
    coreConstIterator __retrieve(const I& Key)const hot_func;
    //! @}
};


// ****************************************************************
/* simplified lookup container type */
template <typename K, typename T> using coreLookup = coreLookupGen<K, K, T>;


// ****************************************************************
/* string lookup container class */
template <typename T> class coreLookupStr final : public coreLookupGen<std::string, const char*, T>
{
public:
    coreLookupStr()noexcept {}
    using coreLookupGen<std::string, const char*, T>::operator [];
    using coreLookupGen<std::string, const char*, T>::erase;

    /*! access specific entry */
    //! @{
    inline T& operator [] (const coreUint& iIndex) {return this->m_aList[iIndex].second;}
    //! @}

    /*! remove existing entries */
    //! @{
    bool erase(const T& Entry);
    inline typename coreLookupStr<T>::coreIterator erase(const coreUint& iIndex) {this->__cache_clear(); ASSERT(iIndex < this->m_aList.size()) return this->m_aList.erase(this->m_aList.begin()+iIndex);}
    //! @}


private:
    /*! retrieve iterator */
    //! @{
    typename coreLookupStr<T>::coreIterator      __retrieve(const T&    Entry)hot_func;
    typename coreLookupStr<T>::coreConstIterator __retrieve(const T&    Entry)const hot_func;
    typename coreLookupStr<T>::coreIterator      __retrieve(const char* pcKey)hot_func;
    typename coreLookupStr<T>::coreConstIterator __retrieve(const char* pcKey)const hot_func;
    //! @}
};


// ****************************************************************
/* constructor */
template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen()noexcept
: m_pCache   (NULL)
, m_CacheKey ((I)NULL)
{
    m_aList.reserve(MAX(64u / sizeof(T), 2u));
}

template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen(const coreLookupGen<K, I, T>& c)noexcept
: m_aList    (c.m_aList)
, m_pCache   (NULL)
, m_CacheKey ((I)NULL)
{
}

template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen(coreLookupGen<K, I, T>&& m)noexcept
: m_aList    (std::move(m.m_aList))
, m_pCache   (NULL)
, m_CacheKey ((I)NULL)
{
}


// ****************************************************************
/* assignment operator */
template <typename K, typename I, typename T> coreLookupGen<K, I, T>& coreLookupGen<K, I, T>::operator = (coreLookupGen<K, I, T> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename R, typename O, typename S> void swap(coreLookupGen<R, O, S>& a, coreLookupGen<R, O, S>& b)noexcept
{
    using std::swap;
    swap(a.m_aList, b.m_aList);
}


// ****************************************************************
/* access specific entry */
template <typename K, typename I, typename T> const T& coreLookupGen<K, I, T>::at(const I& Key)const
{
    // check for cached entry
    if(this->__cache_try(Key)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    ASSERT(this->__check(it))

    return it->second;
}


// ****************************************************************
/* access specific entry and create it if necessary */
template <typename K, typename I, typename T> T& coreLookupGen<K, I, T>::operator [] (const I& Key)
{
    // check for cached entry
    if(this->__cache_try(Key)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    if(!this->__check(it))
    {
        // create new entry
        m_aList.push_back(coreEntry(Key, T()));
        it = m_aList.end()-1;

        // cache current entry
        this->__cache_set(&(*it), Key);
    }

    return it->second;
}


// ****************************************************************
/* remove existing entry */
template <typename K, typename I, typename T> bool coreLookupGen<K, I, T>::erase(const I& Key)
{
    // reset cache
    this->__cache_clear();

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
/* retrieve iterator by specific key */
template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreIterator coreLookupGen<K, I, T>::__retrieve(const I& Key)
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare keys
        if(it->first == Key)
        {
            // cache current entry
            this->__cache_set(&(*it), Key);
            return it;
        }
    }
    return m_aList.end();
}

template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreConstIterator coreLookupGen<K, I, T>::__retrieve(const I& Key)const
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare keys
        if(it->first == Key)
            return it;
    }
    return m_aList.end();
}


// ****************************************************************
/* remove existing entry */
template <typename T> bool coreLookupStr<T>::erase(const T& Entry)
{
    // reset cache
    this->__cache_clear();

    // retrieve and check iterator by specific value
    auto it = this->__retrieve(Entry);
    if(this->__check(it))
    {
        // remove existing entry
        this->m_aList.erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
/* retrieve iterator by specific value */
template <typename T> typename coreLookupStr<T>::coreIterator coreLookupStr<T>::__retrieve(const T& Entry)
{
    // loop through all entries
    FOR_EACH(it, this->m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return this->m_aList.end();
}

template <typename T> typename coreLookupStr<T>::coreConstIterator coreLookupStr<T>::__retrieve(const T& Entry)const
{
    // loop through all entries
    FOR_EACH(it, this->m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return this->m_aList.end();
}


// ****************************************************************
/* retrieve iterator by specific key */
template <typename T> typename coreLookupStr<T>::coreIterator coreLookupStr<T>::__retrieve(const char* pcKey)
{
    // loop through all entries
    FOR_EACH(it, this->m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
        {
            // cache current entry
            this->__cache_set(&(*it), pcKey);
            return it;
        }
    }
    return this->m_aList.end();
}

template <typename T> typename coreLookupStr<T>::coreConstIterator coreLookupStr<T>::__retrieve(const char* pcKey)const
{
    // loop through all entries
    FOR_EACH(it, this->m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
            return it;
    }
    return this->m_aList.end();
}


#endif /* _CORE_GUARD_LOOKUP_H_ */
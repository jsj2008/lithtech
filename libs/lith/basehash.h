/****************************************************************************
;
;   MODULE:     BASEHASH (.H)
;
;   PURPOSE:
;
;   HISTORY:    05/29/95 [m]
;
;   NOTICE:     Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __BASEHASH_H__
#define __BASEHASH_H__

#ifndef __LITHTYPES_H__
#include "lithtypes.h"
#endif

#ifndef __BASELIST_H__
#include "baselist.h"
#endif


class CBaseHashItem;


// The basic hash table class can be used as is if desired.
class CBaseHash {
public:
    // constructors and destructors
    CBaseHash(); // only use this one if you are not going to use the hash table at all (UNUSUAL!)
    CBaseHash(unsigned int NumBins);    
    ~CBaseHash();

    // access functions
    void            Insert(CBaseHashItem* pItem);
    void            Delete(CBaseHashItem* pItem);
    CBaseHashItem*  GetFirst();
    CBaseHashItem*  GetLast();

protected:
    // protected access functions
    CBaseHashItem*  GetFirstInBin(unsigned int Bin);
    unsigned int    GetNumBins() { return m_nNumBins; };

private:
    friend class CBaseHashItem;

    // internal class to hold array of hash elements
    class CHashBin : CBaseListItem {
    public:
        CLTBaseList m_lstItems;
    };

    // internal member variables
    unsigned int    m_nNumBins;
    CHashBin*       m_pBinAry;
};


// User must derive all hash table elements from this class
class CBaseHashItem : public CBaseListItem {
public:
    CBaseHashItem*  Next();
    CBaseHashItem*  Prev();
    CBaseHashItem*  NextInBin() { return (CBaseHashItem*)CBaseListItem::Next(); };
    CBaseHashItem*  PrevInBin() { return (CBaseHashItem*)CBaseListItem::Prev(); };

protected:
    virtual unsigned int HashFunc() = 0;

    CBaseHash* GetParentHash() { return m_pParentHash; };

private:
    friend class CBaseHash;

    CBaseHash*      m_pParentHash;          // The hash table that contains this item
    unsigned int    m_nCurBin;              // The bin we are currently stored in
};


#endif

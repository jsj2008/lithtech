/****************************************************************************
;
;	MODULE:		RezHash (.H)
;
;	PURPOSE:
;
;	HISTORY:	04/08/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __REZHASH_H__
#define __REZHASH_H__

#ifndef __LITH_H__
#include "lith.h"
#endif


// constants
#define kDefaultByNameNumHashBins      19      // number of hash bins in the ItmByName hash table
#define kDefaultByIDNumHashBins        19      // number of hash bins in the ItmByID hash table 
#define kDefaultDirNumHashBins         5       // number of hash bins in the Directory hash table
#define kDefaultTypNumHashBins         9       // number of hash bins in the Type hash table

class CRezItmHashTableByName;

// -----------------------------------------------------------------------------------------
// CRezItmByName

class CRezItmHashByName : public CBaseHashItem {
public:
    CRezItmHashByName() : CBaseHashItem() { m_pRezItm = NULL; };
    CRezItmHashByName(CRezItm* pRezItm) : CBaseHashItem() { m_pRezItm = pRezItm; };
	void SetRezItm(CRezItm* pRezItm) { m_pRezItm = pRezItm; };
	CRezItm* GetRezItm() { return m_pRezItm; };
   	CRezItmHashByName* Next() { return (CRezItmHashByName*)CBaseHashItem::Next(); };
	CRezItmHashByName* Prev() { return (CRezItmHashByName*)CBaseHashItem::Prev(); };
	CRezItmHashByName* NextInBin() { return (CRezItmHashByName*)CBaseHashItem::NextInBin(); };
	CRezItmHashByName* PrevInBin() { return (CRezItmHashByName*)CBaseHashItem::PrevInBin(); };

protected:
	virtual	unsigned int HashFunc();
	CRezItmHashTableByName* GetParentHash() { return (CRezItmHashTableByName*)CBaseHashItem::GetParentHash(); };

private:
    friend class CRezItmHashTableByName;
	CRezItm* m_pRezItm;
};	


// -----------------------------------------------------------------------------------------
// CRezItmHashTableByName

class CRezItmHashTableByName : public CBaseHash {
public:
    CRezItmHashTableByName(unsigned int NumBins) : CBaseHash(NumBins) { };	
    CRezItmHashTableByName() : CBaseHash(1) { };	
    CRezItm*            Find(REZCNAME sName, BOOL bIgnoreCase = TRUE);
   	void			    Insert(CRezItmHashByName* pItem) { CBaseHash::Insert(pItem); };
	void			    Delete(CRezItmHashByName* pItem) { CBaseHash::Delete(pItem); };
    CRezItmHashByName*  GetFirst() { return (CRezItmHashByName*)CBaseHash::GetFirst(); };
    CRezItmHashByName*  GetLast() { return (CRezItmHashByName*)CBaseHash::GetLast(); };

protected:
	friend class CRezItmHashByName;
    CRezItmHashByName*  GetFirstInBin(unsigned int Bin) { return (CRezItmHashByName*)CBaseHash::GetFirstInBin(Bin); };
	unsigned int		HashFunc(REZCNAME pStr);
};

class CRezItmHashTableByID;

// -----------------------------------------------------------------------------------------
// CRezItmByID

class CRezItmHashByID : public CBaseHashItem {
public:
    CRezItmHashByID() : CBaseHashItem() { m_pRezItm = NULL; };
    CRezItmHashByID(CRezItm* pRezItm) : CBaseHashItem() { m_pRezItm = pRezItm; };
	void SetRezItm(CRezItm* pRezItm) { m_pRezItm = pRezItm; };
	CRezItm* GetRezItm() { return m_pRezItm; };
   	CRezItmHashByID* Next();// { return (CRezItmHashByID*)CBaseHashItem::Next(); };
	CRezItmHashByID* Prev() { return (CRezItmHashByID*)CBaseHashItem::Prev(); };
	CRezItmHashByID* NextInBin() { return (CRezItmHashByID*)CBaseHashItem::NextInBin(); };
	CRezItmHashByID* PrevInBin() { return (CRezItmHashByID*)CBaseHashItem::PrevInBin(); };

protected:
	virtual	unsigned int HashFunc();
	CRezItmHashTableByID* GetParentHash() { return (CRezItmHashTableByID*)CBaseHashItem::GetParentHash(); };

private:
    friend class CRezItmHashTableByID;
	CRezItm*	m_pRezItm;
};	


// -----------------------------------------------------------------------------------------
// CRezItmHashTableByID

class CRezItmHashTableByID : public CBaseHash {
public:
    CRezItmHashTableByID(unsigned int nBins) : CBaseHash(nBins) {	};	
    CRezItmHashTableByID() : CBaseHash() {	};	// special version for when this hash table is not used
    CRezItm*            Find(REZID nID);
   	void			    Insert(CRezItmHashByID* pItem) { CBaseHash::Insert(pItem); };
	void			    Delete(CRezItmHashByID* pItem) { CBaseHash::Delete(pItem); };
    CRezItmHashByID*    GetFirst() { return (CRezItmHashByID*)CBaseHash::GetFirst(); };
    CRezItmHashByID*    GetLast() { return (CRezItmHashByID*)CBaseHash::GetLast(); };

protected:
	friend class CRezItmHashByID;
    CRezItmHashByID*    GetFirstInBin(unsigned int Bin) { return (CRezItmHashByID*)CBaseHash::GetFirstInBin(Bin); };
	unsigned int		HashFunc(REZID ID);
};

class CRezTypeHashTable;

// -----------------------------------------------------------------------------------------
// CRezTypeHash

class CRezTypeHash : public CBaseHashItem {
public:
    CRezTypeHash() : CBaseHashItem() { m_pRezTyp = NULL; };
    CRezTypeHash(CRezTyp* pRezTyp) : CBaseHashItem() { m_pRezTyp = pRezTyp; };
	void SetRezTyp(CRezTyp* pRezTyp) { m_pRezTyp = pRezTyp; };
	CRezTyp* GetRezTyp() { return m_pRezTyp; };
   	CRezTypeHash* Next() { return (CRezTypeHash*)CBaseHashItem::Next(); };
	CRezTypeHash* Prev() { return (CRezTypeHash*)CBaseHashItem::Prev(); };
	CRezTypeHash* NextInBin() { return (CRezTypeHash*)CBaseHashItem::NextInBin(); };
	CRezTypeHash* PrevInBin() { return (CRezTypeHash*)CBaseHashItem::PrevInBin(); };

protected:
	virtual	unsigned int HashFunc();
	CRezTypeHashTable* GetParentHash() { return (CRezTypeHashTable*)CBaseHashItem::GetParentHash(); };

private:
    friend class CRezTypeHashTable;
    CRezTyp*  m_pRezTyp;    
};	


// -----------------------------------------------------------------------------------------
// CRezTypeHashTable

class CRezTypeHashTable : public CBaseHash {
public:
    CRezTypeHashTable(unsigned int NumBins) : CBaseHash(NumBins) { };	
    CRezTyp*            Find(REZTYPE nType);
   	void			    Insert(CRezTypeHash* pItem) { CBaseHash::Insert(pItem); };
	void			    Delete(CRezTypeHash* pItem) { CBaseHash::Delete(pItem); };
    CRezTypeHash*       GetFirst() { return (CRezTypeHash*)CBaseHash::GetFirst(); };
    CRezTypeHash*       GetLast() { return (CRezTypeHash*)CBaseHash::GetLast(); };
protected:
	friend class CRezTypeHash;
    CRezTypeHash*       GetFirstInBin(unsigned int Bin) { return (CRezTypeHash*)CBaseHash::GetFirstInBin(Bin); };
	unsigned int		HashFunc(REZTYPE nType);
};


class CRezDirHashTable;

// -----------------------------------------------------------------------------------------
// CRezDirHash

class CRezDirHash : public CBaseHashItem {
public:
    CRezDirHash() : CBaseHashItem() { m_pRezDir = NULL; };
    CRezDirHash(CRezDir* pRezDir) : CBaseHashItem() { m_pRezDir = pRezDir; };
	void SetRezDir(CRezDir* pRezDir) { m_pRezDir = pRezDir; };
	CRezDir* GetRezDir() { return m_pRezDir; };
   	CRezDirHash* Next() { return (CRezDirHash*)CBaseHashItem::Next(); };
	CRezDirHash* Prev() { return (CRezDirHash*)CBaseHashItem::Prev(); };
	CRezDirHash* NextInBin() { return (CRezDirHash*)CBaseHashItem::NextInBin(); };
	CRezDirHash* PrevInBin() { return (CRezDirHash*)CBaseHashItem::PrevInBin(); };

protected:
	virtual	unsigned int HashFunc();
	CRezDirHashTable* GetParentHash() { return (CRezDirHashTable*)CBaseHashItem::GetParentHash(); };

private:
    friend class CRezDirHashTable;
	CRezDir* m_pRezDir;
};	


// -----------------------------------------------------------------------------------------
// CRezDirHashTable

class CRezDirHashTable : public CBaseHash {
public:
    CRezDirHashTable(unsigned int NumBins) : CBaseHash(NumBins) { };	
    CRezDir*            Find(REZCDIRNAME sDirName, BOOL bIgnoreCase = TRUE);
   	void			    Insert(CRezDirHash* pItem) { CBaseHash::Insert(pItem); };
	void			    Delete(CRezDirHash* pItem) { CBaseHash::Delete(pItem); };
    CRezDirHash*        GetFirst() { return (CRezDirHash*)CBaseHash::GetFirst(); };
    CRezDirHash*        GetLast() { return (CRezDirHash*)CBaseHash::GetLast(); };
protected:
	friend class CRezDirHash;
    CRezDirHash*        GetFirstInBin(unsigned int Bin) { return (CRezDirHash*)CBaseHash::GetFirstInBin(Bin); };
	unsigned int		HashFunc(REZCDIRNAME sDirName);
};


#endif


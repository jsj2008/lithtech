//------------------------------------------------------
// PrefabRef.h
//
// Prefab reference object header
//
// Author: Kevin Francis
// Created: 03/15/2001
// Modification History: 
//		03/15/2001 - First implementation
//
// Note : !!IMPORTANT!! Do not call any of the functions
// on this object unless the edit region they belong to 
// is the active document.  It was either make that 
// restriction (which probably isn't going to be a 
// problem) or require sending the region into the 
// functions.  (Which probably wouldn't fix the issue 
// due to that parameter probably being filled in with 
// the region from the active document...)  This is a
// restriction which has been silently introduced in a
// number of places in DEdit, but this is the only place
// where it has been explicitly documented.
//------------------------------------------------------


#ifndef __PREFABREF_H__
#define __PREFABREF_H__

#include "worldnode.h"

#ifdef DIRECTEDITOR_BUILD
#include "undo_mgr.h"
#endif

class CEditRegion;

class CPrefabRef : public CWorldNode
{
public:

	//callback function for obtaining error information during instantiation
	typedef	void (*TInstantiateErrorCallback)(const char* pszText);

	CPrefabRef();
	CPrefabRef(const CPrefabRef &cOther);
	virtual ~CPrefabRef();

	// Allocation/copy overrides from CWorldNode
	virtual CWorldNode*		AllocateSameKind()	{return new CPrefabRef;}
	virtual void			DoCopy(CWorldNode *pOther);

	// Get/set the current prefab tree
	const CWorldNode *		GetPrefabTree() const { return m_pPrefabTree; }
	void					SetPrefabTree(const CWorldNode *pPrefabTree) { m_pPrefabTree = pPrefabTree; }

	// Get/set the prefab filename
	const char *			GetPrefabFilename() const { return m_sPrefabFilename; }
	void					SetPrefabFilename(const char *pName) { m_sPrefabFilename = pName; }

	virtual LTVector		GetPos();						// Returns the position
	virtual void			SetPos(const LTVector &v);		// Sets the position
	virtual LTVector		GetOr();						// Returns the rotation
	virtual void			SetOr(const LTVector &v);		// Sets the rotation

	virtual CVector			GetUpperLeftCornerPos() { return GetPos(); } // Act like an object for the corner position

	LTVector				GetPrefabMin() const { return m_vMin; }
	LTVector				GetPrefabMax() const { return m_vMax; }
	void					SetPrefabDims(const LTVector &vMin, const LTVector& vMax) { m_vMin = vMin; m_vMax = vMax; }

	LTVector				GetPrefabCenter() const { return (m_vMax + m_vMin) * 0.5f; }
	LTVector				GetPrefabDims() const { return (m_vMax - m_vMin) * 0.5f; }

	// Instantiate final versions of the prefab objects into the world.
	// Note that you should remove and delete this object after calling this function.
	#ifdef DIRECTEDITOR_BUILD
	CWorldNode *			InstantiatePrefab(CEditRegion *pRegion, TInstantiateErrorCallback pErrorCallback, PreActionList *pUndoList = LTNULL);
	// Notification that a property of this object has changed
	void					OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers);
	#else
	CWorldNode *			InstantiatePrefab(CEditRegion *pRegion, TInstantiateErrorCallback pErrorCallback);
	#endif
	
private:
	// Note : The prefab mgr owns this tree
	const CWorldNode *		m_pPrefabTree;
	CString					m_sPrefabFilename;
	LTVector				m_vMin;
	LTVector				m_vMax;
};

#endif //__PREFABREF_H__

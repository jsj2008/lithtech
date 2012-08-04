//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ObjectImporter.h: interface for the CObjectImporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECTIMPORTER_H__7D7E6313_E0C2_11D2_BE0B_0060971BDC6D__INCLUDED_)
#define AFX_OBJECTIMPORTER_H__7D7E6313_E0C2_11D2_BE0B_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "proplist.h"
#include "butemgr.h"

class CWorldNode;
class CObjectImporter  
{
public:
	CObjectImporter();
	virtual ~CObjectImporter();

	// Parses an object file but does not import them into the world.  Call ImportObjects
	// to import them into the world.
	BOOL			LoadObjectFile(CString sFilename);

	// Imports objects from a filename into the current world
	BOOL			ImportObjects();

	// Returns the number of objects, their name, and type
	int				GetNumObjects()						{ return m_tagArray.GetSize(); }
	CString			GetObjectName(int nIndex);
	CString			GetObjectType(int nIndex);

	// Update the properties for the object from the bute file.  This is used
	// by both ImportObjects and the template class code.
	BOOL			UpdateObjectProperties(CWorldNode *pObject, CString sButeTag);
	BOOL			UpdateObjectProperties(CWorldNode *pObject, CString sName, CString sType);

	// Parses the name_type string into separate strings
	BOOL			ParseNameAndType(CString sNameAndType, CString &sName, CString &sType);	

protected:
	// Processes a single object.  The string name refers to the object name
	// and type in this format: ObjectName_Type
	// bUpdateObjects determines if objects are updated or if new ones are created.
	BOOL		ProcessObject(CString sType, BOOL bUpdateObjects);	

	// Check to see if the imported objects already exist in the world.
	// True is returned if one or more objects exist in the world.
	BOOL		CheckObjectExistance();

	// Updates the various properties from the attribute file
	BOOL		UpdateStringProperty	(CStringProp	*pStringProp,	CString sButeTag);
	BOOL		UpdateVectorProperty	(CVectorProp	*pVectorProp,	CString sButeTag);
	BOOL		UpdateColorProperty		(CColorProp		*pColorProp,	CString sButeTag);
	BOOL		UpdateRealProperty		(CRealProp		*pRealProp,		CString sButeTag);
	BOOL		UpdateBoolProperty		(CBoolProp		*pBoolProp,		CString sButeTag);
	BOOL		UpdateRotationProperty	(CRotationProp	*pRotationProp,	CString sButeTag);

	// The callback used by ButeMgr to get the tags
	static bool GetTagsCallback(const char *lpszString, void *pAuxData);	

protected:
	CButeMgr			m_buteMgr;			// The attribute manager
	CStringArray		m_tagArray;			// The array of tags that have been parsed
};

#endif // !defined(AFX_OBJECTIMPORTER_H__7D7E6313_E0C2_11D2_BE0B_0060971BDC6D__INCLUDED_)

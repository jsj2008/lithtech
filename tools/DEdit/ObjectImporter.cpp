//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ObjectImporter.cpp: implementation of the CObjectImporter class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "objectimporter.h"
#include "dedit.h"
#include <afxtempl.h>

#include "butemgr.h"
#include "edithelpers.h"
#include "editregion.h"
#include "regiondoc.h"
#include "importobjectdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////
// Static members

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CObjectImporter::CObjectImporter()
{
	// Remove all of the tags
	m_tagArray.RemoveAll();	
}

CObjectImporter::~CObjectImporter()
{

}

/************************************************************************/
// Parses an object file but does not import them into the world.  Call ImportObjects
// to import them into the world.
BOOL CObjectImporter::LoadObjectFile(CString sFilename)
{
	// Parse the input file
	if (!m_buteMgr.Parse(sFilename))
	{
		return FALSE;
	}

	// Remove all of the tags
	m_tagArray.RemoveAll();	

	// Get the tags	
	m_buteMgr.GetTags(GetTagsCallback, (void *)&m_tagArray);	
	
	return TRUE;
}

/************************************************************************/
// Imports objects from a filename
BOOL CObjectImporter::ImportObjects()
{	
	// Determines if objects are updated or if new objects are created
	BOOL bUpdateObjects=TRUE;

	// Check to see if these objects already exist in the world
	if (CheckObjectExistance())
	{
		// Ask the user if they want to replace the existing objects or create new ones
		CImportObjectDlg objectDlg;
		if (objectDlg.DoModal() == IDOK)
		{
			if (objectDlg.m_nUpdateRadio == 0)
			{
				bUpdateObjects=TRUE;
			}
			else
			{
				bUpdateObjects=FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}

	// Process each object	
	int i;
	for (i=0; i < m_tagArray.GetSize(); i++)
	{
		ProcessObject(m_tagArray[i], bUpdateObjects);
	}

	// Update the properties dialog
	GetActiveRegionDoc()->SetupPropertiesDlg(FALSE);
	GetActiveRegionDoc()->RedrawAllViews();

	return TRUE;
}

/************************************************************************/
// Returns the object name for a specific index
CString CObjectImporter::GetObjectName(int nIndex)
{
	CString sName;
	CString sType;
	ParseNameAndType(m_tagArray[nIndex], sName, sType);

	return sName;
}

/************************************************************************/
// Returns the object type for a specific index
CString	CObjectImporter::GetObjectType(int nIndex)
{
	CString sName;
	CString sType;
	ParseNameAndType(m_tagArray[nIndex], sName, sType);

	return sType;
}

/************************************************************************/
// Check to see if the imported objects already exist in the world.
// True is returned if one or more objects exist in the world.
BOOL CObjectImporter::CheckObjectExistance()
{
	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		return FALSE;
	}

	// Check to see if the imported objects already exist in the world
	int i;
	for (i=0; i < m_tagArray.GetSize(); i++)
	{
		CBaseEditObj *pObj;

		// Parse the name and type
		CString sName;
		CString sType;
		ParseNameAndType(m_tagArray[i], sName, sType);

		// Find the object by name
		if (pDoc->GetRegion()->FindObjectsByName(sName, &pObj, 1) > 0)
		{
			return TRUE;
		}
	}

	// No objects were found
	return FALSE;
}

/************************************************************************/
// Processes a single object.  The string name refers to the object name
// and type in this format: ObjectName$Type
BOOL CObjectImporter::ProcessObject(CString sNameAndType, BOOL bUpdateObjects)
{
	// The name and type
	CString sName;
	CString sType;

	// Parse the name_type string
	if (!ParseNameAndType(sNameAndType, sName, sType))
	{
		// Default to "Object" for the name
		sType = sNameAndType;
		sName = "Object";
	}

	// Get the document
	CRegionDoc *pDoc=GetActiveRegionDoc();
	if (!pDoc)
	{
		return FALSE;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();

	// Find existing objects with this name
	CBaseEditObj *pFoundObjectArray[4096];
	int nFoundObjects=pDoc->GetRegion()->FindObjectsByName(sName, (CBaseEditObj **)&pFoundObjectArray, 4096);

	// Determine if we should update existing objects or create new ones
	if (bUpdateObjects && nFoundObjects > 0)
	{
		// Update each object
		int i;
		for (i=0; i < nFoundObjects; i++)
		{
			// Check to see if there is a class type collision
			if (pFoundObjectArray[i]->GetClassName() != sType)
			{
				CString sWarning;
				sWarning.Format("Object %s (%s) has class type collision with %s.  Would you like to skip this object?",
								pFoundObjectArray[i]->GetName(), pFoundObjectArray[i]->GetClassName(), sType);

				if (MessageBox(AfxGetMainWnd()->GetSafeHwnd(), sWarning, "Warning", MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
				{
					// Skip this object
					continue;
				}
			}
			UpdateObjectProperties(pFoundObjectArray[i], sNameAndType);
		}
	}
	else
	{
		// Add the object
		CWorldNode *pObject=pDoc->AddObject(sType, pRegion->GetMarker());

		if (!pObject)
		{
			AfxMessageBox("Unable to create object of class " + sType);
			return FALSE;
		}

		// Set the object name
		pObject->SetName(sName);

		// Update the properties for the object from the bute file
		UpdateObjectProperties(pObject, sNameAndType);

		// Bind the node to the active parent
		pDoc->BindNode(pObject, pRegion->GetActiveParentNode());

		// Select the object
		pDoc->SelectNode(pObject, false);	
	}

	return TRUE;
}

/************************************************************************/
// Parses the name_type string into separate strings
BOOL CObjectImporter::ParseNameAndType(CString sNameAndType, CString &sName, CString &sType)
{	
	// Find the separating dash
	int nDashIndex=sNameAndType.Find("___");
	if (nDashIndex == -1)
	{
		return FALSE;
	}

	// Separate the name and type from the combined string
	sType=sNameAndType.Left(nDashIndex);
	sName=sNameAndType.Right(sNameAndType.GetLength()-nDashIndex-3);

	return TRUE;
}

/************************************************************************/
// Update the properties for the object from the bute file
BOOL CObjectImporter::UpdateObjectProperties(CWorldNode *pObject, CString sButeTag)
{
	// Get the property list
	CPropList *pPropList=pObject->GetPropertyList();
	if (!pPropList)
	{
		return FALSE;
	}
	
	// Go through each property and see if it has a matching property in the bute file
	int i;
	for (i=0; i < pPropList->GetSize(); i++)
	{
		// Get the property
		CBaseProp *pProp=pPropList->GetAt(i);

		// Handle each type differently
		switch (pProp->GetType())
		{		
		case LT_PT_STRING:		// String property
			{
				UpdateStringProperty((CStringProp *)pProp, sButeTag);
				break;
			}
		case LT_PT_VECTOR:		// Vector property
			{
				UpdateVectorProperty((CVectorProp *)pProp, sButeTag);
				break;
			}
		case LT_PT_COLOR:		// Color property
			{
				UpdateColorProperty((CColorProp *)pProp, sButeTag);
				break;
			}		
		case LT_PT_FLAGS:		// Flags property
		case LT_PT_LONGINT:	// Integer property
		case LT_PT_REAL:		// Real property
			{
				UpdateRealProperty((CRealProp *)pProp, sButeTag);
				break;
			}			
		case LT_PT_BOOL:		// Bool property
			{
				UpdateBoolProperty((CBoolProp *)pProp, sButeTag);
				break;
			}			
		case LT_PT_ROTATION:	// Rotation property
			{
				UpdateRotationProperty((CRotationProp *)pProp, sButeTag);
				break;
			}
		default:
			{
				ASSERT(FALSE);
			}
		}
	}

	return TRUE;
}

/************************************************************************/
// Update the properties for the object from the bute file
BOOL CObjectImporter::UpdateObjectProperties(CWorldNode *pObject, CString sName, CString sType)
{
	return UpdateObjectProperties(pObject, sType+"___"+sName);
}

/************************************************************************/
// Updates a string property from the attribute file
BOOL CObjectImporter::UpdateStringProperty(CStringProp *pStringProp, CString sButeTag)
{
	// Check the pointer
	if (pStringProp == NULL)
	{
		return FALSE;
	}

	// Get the string
	CString sString=m_buteMgr.GetString(sButeTag, pStringProp->GetName());
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}

	pStringProp->SetString(sString);
	return TRUE;
}

/************************************************************************/
// Updates a vector property from the attribute file
BOOL CObjectImporter::UpdateVectorProperty(CVectorProp *pVectorProp, CString sButeTag)
{
	// Check the pointer
	if (pVectorProp == NULL)
	{
		return FALSE;
	}

	// Get the vector
	DVector v=m_buteMgr.GetVector(sButeTag, pVectorProp->GetName());
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}

	pVectorProp->SetVector(CVector(v.x, v.y, v.z));
	return TRUE;	
}

/************************************************************************/
// Updates a color property from the attribute file
BOOL CObjectImporter::UpdateColorProperty(CColorProp *pColorProp, CString sButeTag)
{
	// Check the pointer
	if (pColorProp == NULL)
	{
		return FALSE;
	}

	// Get the color as a vector
	DVector vColor=m_buteMgr.GetVector(sButeTag, pColorProp->GetName());
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}

	pColorProp->SetColor(CVector(vColor.x, vColor.y, vColor.z));

	return TRUE;
}

/************************************************************************/
// Updates a real property from the attribute file
BOOL CObjectImporter::UpdateRealProperty(CRealProp *pRealProp, CString sButeTag)
{
	// Check the pointer
	if (pRealProp == NULL)
	{
		return FALSE;
	}

	// Get the value
	float fValue=m_buteMgr.GetFloat(sButeTag, pRealProp->GetName());
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}

	pRealProp->SetValue(fValue);

	return TRUE;
}

/************************************************************************/
// Updates a bool property from the attribute file
BOOL CObjectImporter::UpdateBoolProperty(CBoolProp *pBoolProp, CString sButeTag)
{
	// Check the pointer
	if (pBoolProp == NULL)
	{
		return FALSE;
	}

	// Get the value
	bool bValue=m_buteMgr.GetBool(sButeTag, pBoolProp->GetName());		
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}

	// Set the property value
	pBoolProp->SetValue(bValue);
	
	return TRUE;
}

/************************************************************************/
// Updates a rotation property from the attribute file
BOOL CObjectImporter::UpdateRotationProperty(CRotationProp *pRotationProp, CString sButeTag)
{
	// Check the pointer
	if (pRotationProp == NULL)
	{
		return FALSE;
	}

	// Get the rotation as a vector
	DVector vRotation=m_buteMgr.GetVector(sButeTag, pRotationProp->GetName());
	if (!m_buteMgr.Success())
	{		
		// The attribute wasn't found
		return FALSE;
	}	

	pRotationProp->SetEulerAngles(vRotation);
	return TRUE;
}

/************************************************************************/
// The callback used by ButeMgr to get the tags
bool CObjectImporter::GetTagsCallback(const char *lpszString, void *pAuxData)
{
	// Cast the auxilary data to the tag array
	CStringArray *pTagArray=(CStringArray *)pAuxData;
	
	// Add the tag to the array
	if (pTagArray)
	{
		pTagArray->Add(lpszString);
	}

	return true;
}
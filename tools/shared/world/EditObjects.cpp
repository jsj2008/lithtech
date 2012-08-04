//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditObjects.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : November 28 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editobjects.h"
#include "editregion.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "conparse.h"

#ifdef DIRECTEDITOR_BUILD
	#include "objectimporter.h"
	#include "edithelpers.h"
	#include "editprojectmgr.h"
	#include "objectimporter.h"
	#include "projectbar.h"
	#include "iobjectplugin.h"
	#include "modelmgr.h"
	#include "optionsdisplay.h"
	#include "genericprop_setup.h"
	#include "prefabmgr.h"
#endif


// The basic properties that all CDEObjects have.
char *g_NameName="Name", *g_PosName="Pos", *g_AnglesName="Rotation";
CVector			g_DummyPos(0.0f, 0.0f, 0.0f);
LTVector		g_DummyRotation(0.0f, 0.0f, 0.0f);
CStringProp		g_DummyString("DUMMY");



CBaseProp* CreatePropFromCode( int code, char *pName )
{
	CBaseProp		*pRet = NULL;

	if(code == LT_PT_REAL || code == LT_PT_FLAGS || code == LT_PT_LONGINT)
		pRet = new CRealProp(pName);
	else if(code == LT_PT_VECTOR)
		pRet = new CVectorProp(pName);
	else if(code == LT_PT_COLOR)
		pRet = new CColorProp(pName);
	else if(code == LT_PT_STRING)
		pRet = new CStringProp(pName);
	else if(code == LT_PT_ROTATION)
		pRet = new CRotationProp(pName);
	else if(code == LT_PT_BOOL)
		pRet = new CBoolProp(pName);
	else
		ASSERT(FALSE);

	if(pRet)
		pRet->m_Type = code;
	
	return pRet;
}


void SetupNewProp(CBaseProp *pProp, PropDef *pVar)
{
	pProp->m_PropFlags |= pVar->m_PropFlags;
	pProp->m_pPropDef = pVar;

	// Fill in the default values.
	if(pProp->m_Type == LT_PT_BOOL)
	{
		((CBoolProp*)pProp)->m_Value = (char)pVar->m_DefaultValueFloat;
	}
	else if(pProp->m_Type == LT_PT_REAL || pProp->m_Type == LT_PT_LONGINT ||
		pProp->m_Type == LT_PT_FLAGS)
	{
		((CRealProp*)pProp)->m_Value = pVar->m_DefaultValueFloat;
	}
	else if(pProp->m_Type == LT_PT_STRING && pVar->m_DefaultValueString)
	{
		strncpy(((CStringProp*)pProp)->m_String, pVar->m_DefaultValueString, MAX_STRINGPROP_LEN);
	}
	else if(pProp->m_Type == LT_PT_COLOR)
	{
		((CColorProp*)pProp)->m_Vector = pVar->m_DefaultValueVector;
	}
	else if(pProp->m_Type == LT_PT_VECTOR)
	{
		((CVectorProp*)pProp)->m_Vector = pVar->m_DefaultValueVector;
	}
	else if(pProp->m_Type == LT_PT_ROTATION)
	{
		((CRotationProp*)pProp)->InitEulerAngles();
	}
	else
	{
		ASSERT(FALSE);
	}
}


PropDef* FindPropInList(CMoArray<PropDef*> &propList, char *pPropName, uint32 *pIndex)
{
	uint32 i;

	for(i=0; i < propList.GetSize(); i++)
	{
		if(strcmp(propList[i]->m_PropName, pPropName) == 0)
		{
			if(pIndex)
				*pIndex = i;

			return propList[i];
		}
	}

	return NULL;
}

void GetClassDefProps(ClassDef *pClass, CMoArray<PropDef*> &propList)
{
	int i, chainLen;
	uint32 j, propIndex, curPropIndex;
	ClassDef *pCurClass;
	CMoArray<ClassDef*> classChain;
	PropDef *pProp, *pAlreadyThere;
	CMoArray<PropDef*> unhiddenProps, hiddenProps;

	propList.Term();

	// Unwind the chain.
	chainLen = 0;
	pCurClass = pClass;
	while(pCurClass)
	{
		pCurClass = pCurClass->m_ParentClass;
		++chainLen;
	}

	classChain.SetSize(chainLen);
	--chainLen;
	pCurClass = pClass;
	while(pCurClass)
	{
		classChain[chainLen] = pCurClass;
		pCurClass = pCurClass->m_ParentClass;
		--chainLen;
	}

	// Now go up the chain and add properties.
	for(i=0; i < (int)classChain; i++)
	{
		pCurClass = classChain[i];

		for(j=0; j < (uint32)pCurClass->m_nProps; j++)
		{
			pProp = &pCurClass->m_Props[j];

			pAlreadyThere = FindPropInList(propList, pProp->m_PropName, &propIndex);
			if(pAlreadyThere)
			{
				propList.Remove(propIndex);
			}
			
			propList.Append(pProp);
		}
	}

	// Move hidden properties to the end of the list.
	for(i=0; i < (int)propList; i++)
	{
		if((propList[i]->m_PropFlags & PF_HIDDEN) || 
			((propList[i]->m_PropFlags & PF_GROUPMASK) && !(propList[i]->m_PropFlags & PF_GROUPOWNER)))
		{
			hiddenProps.Append(propList[i]);
		}
		else
		{
			unhiddenProps.Append(propList[i]);
		}
	}

	curPropIndex = 0;
	for(i=0; i < (int)unhiddenProps; i++)
		propList[curPropIndex++] = unhiddenProps[i];

	for(i=0; i < (int)hiddenProps; i++)
		propList[curPropIndex++] = hiddenProps[i];
}


BOOL SetupWorldNodeFromClass(
	ClassDef *pClass, 
	TemplateClass *pTemplate,
	CWorldNode *pNode, 
	CEditRegion *pRegion,
	char *pNameOverride)
{
	int i, iName;
	PropDef *pVar;
	CBaseProp *pProp;
	CMoArray<PropDef*> propList;
	char testName[256];
	CStringProp *pNameProp;


	pNode->SetClassName(pClass->m_ClassName);
	pNode->SetNodeLabel(pClass->m_ClassName);
	
	GetClassDefProps(pClass, propList);
	
	for(i=0; i < (int)propList; i++)
	{
		pVar = propList[i];
		
		pProp = CreatePropFromCode(pVar->m_PropType, pVar->m_PropName);
		if(!pProp)
			continue;

		SetupNewProp(pProp, pVar);

		pNode->m_PropList.m_Props.Append(pProp);
	}

	// Copy parameters from the template (if there is one).
#ifdef DIRECTEDITOR_BUILD
	if(pTemplate)
	{
		CObjectImporter *pObjectImporter=GetProject()->GetTemplateObjectImporter();
		if (pObjectImporter)
		{
			pObjectImporter->UpdateObjectProperties(pNode, pTemplate->m_ClassName, pTemplate->m_ParentClassName);
		}
	}

	pNode->RefreshAllProperties( NULL );
#endif

	// Give it a unique name.
	if(pNameProp = (CStringProp*)pNode->m_PropList.GetMatchingProp(g_NameName, LT_PT_STRING))
	{
		if(pNameOverride)
		{
			SAFE_STRCPY(pNameProp->m_String, pNameOverride);
		}
		else
		{
			for(iName=0; iName < 1000; iName++)
			{
				sprintf(testName, "%s%d", pClass->m_ClassName, iName);
				if(pRegion->FindNodeByName(testName))
				{
				}
				else
				{
					strncpy(pNameProp->m_String, testName, MAX_STRINGPROP_LEN);
					break;
				}
			}
		}
	}

	return TRUE;
}


BOOL SetupWorldNode(const char *pClassName, CWorldNode *pNode, CEditRegion *pRegion)
{
#ifdef DIRECTEDITOR_BUILD
	TemplateClass *pTemplate;
	ClassDef *pClass;

	pTemplate = NULL;
	if( !(pClass = GetProject()->FindClassDef(pClassName)) )
	{
		if((pTemplate = GetProject()->FindTemplateClass(pClassName)) &&
			(pClass = GetProject()->FindClassDef(pTemplate->m_ParentClassName)))
		{
		}
		else
		{
			return FALSE;
		}
	}

	return SetupWorldNodeFromClass(pClass, pTemplate, pNode, pRegion);
#else
	return TRUE;
#endif
}

BOOL LoadNodeProperties(CAbstractIO &file, CWorldNode *pNode, CEditProjectMgr* pProject)
{
	uint32			k, nProperties, objDataStart;
	char			typeName[256], propName[256];
	CBaseProp		*pProp;
	BYTE			propCode;
	WORD			propLen, objDataLen;
	uint32 propFlags;


	LithTry
	{
		file >> objDataLen;
		objDataStart = file.GetCurPos();
		
		if( !file.ReadString(typeName, 256) )
			return FALSE;

		pNode->SetClassName(typeName);
		
		file >> nProperties;
		pNode->m_PropList.m_Props.SetCacheSize(nProperties);

		for( k=0; k < nProperties; k++ )
		{
			if( !file.ReadString(propName,256) )
				return FALSE;

			file >> propCode;
			file >> propFlags;
			file >> propLen;

			if( (pProp = CreatePropFromCode(propCode, propName)) )
			{
				pProp->m_PropFlags = propFlags;
				pProp->LoadData( file );

#ifdef DIRECTEDITOR_BUILD
				if ( pProject )
				{
					// Associate the PropDef

					ClassDef* pClassDef = pProject->FindClassDef(pNode->GetClassName());
					if ( pClassDef )
					{
						CMoArray<PropDef*> *pPropList;
						if(pPropList = pProject->GetClassDefProps(pClassDef))
						{
							pProp->m_pPropDef = pProject->FindPropInList(*pPropList, propName);
						}
					}
				}
#endif
				pNode->m_PropList.m_Props.Append( pProp );
			}
			else
			{
				file.SeekTo( file.GetCurPos() + propLen );
			}
		}
	}
	LithCatch(CLithException &exception)
	{
		exception=exception;
		return FALSE;
	}

	return TRUE;
}

BOOL LoadObjectList( CAbstractIO &file, CMoArray<CBaseEditObj*> &theList)
{
	uint32			i, nObjects;
	CBaseEditObj	*pObj;


	// Load in all the objects.
	file >> nObjects;
	theList.SetCacheSize( 30 );

	for( i=0; i < nObjects; i++ )
	{
		pObj = new CBaseEditObj;

		if(!LoadNodeProperties(file, pObj, NULL))
		{
			delete pObj;
			return FALSE;
		}
		
		theList.Append(pObj);
	}

	return TRUE;
}

class CPropListContainer;
extern CPropListContainer g_propListContainer;


static uint32 StringSize(const char* pszString)
{
	return 2 + strlen(pszString);
}

uint32 GetNodePropertiesSize(CWorldNode* pObj)
{
	CBaseProp		*pProp;

	uint32 nTotalSize = 0;

	//length of the object data length
	nTotalSize += 2;

	// Write the object data.
	if( strlen( pObj->GetClassName() ) < 256 )
	{
		nTotalSize += StringSize(pObj->GetClassName());
	}
	else
	{
		nTotalSize += StringSize("");
	}

	//the number of properties
	nTotalSize += 4;

	for(uint32 k=0; k	< pObj->m_PropList.m_Props; k++ )
	{
		pProp = pObj->m_PropList.m_Props[k];

		if( strlen( pProp->m_Name ) < 256 )
			nTotalSize += StringSize(pProp->m_Name);
		else
			nTotalSize += StringSize("");
		
		//type and flags
		nTotalSize += 5;

		//length of the property data
		nTotalSize += 2;
		
		nTotalSize += pProp->DataSize();
	}

	return nTotalSize;
}

void SaveNodeProperties(CAbstractIO &file, CWorldNode *pObj)
{
	uint32			k, curPos1, curPos2;
	uint32			curPos, objDataStart;
	WORD			objDataLen=0, propLen=0;
	CBaseProp		*pProp;


	file << objDataLen;
	objDataStart = file.GetCurPos();
	
	// Write the object data.
	if( strlen( pObj->GetClassName() ) < 256 )
		file.WriteString( (char*)pObj->GetClassName() );
	else
		file.WriteString( "" );

	file << pObj->m_PropList.m_Props.GetSize();
	for( k=0; k	< pObj->m_PropList.m_Props; k++ )
	{
		pProp = pObj->m_PropList.m_Props[k];

		if( strlen( pProp->m_Name ) < 256 )
			file.WriteString( pProp->m_Name );
		else
			file.WriteString( "" );
		
		file << (BYTE)pProp->m_Type;
		file << (uint32)pProp->m_PropFlags;

		curPos1 = file.GetCurPos();
		file << propLen;
		
		pProp->SaveData( file );

		// Go back and write the length of the property's data.
		curPos2 = file.GetCurPos();
		file.SeekTo( curPos1 );
		propLen = (WORD)(curPos2 - curPos1) - sizeof(WORD);
		file << propLen;
		file.SeekTo( curPos2 );
	}


	// Write how big the data was.
	curPos = file.GetCurPos();
	objDataLen = (WORD)(curPos - objDataStart);
	file.SeekTo( objDataStart - sizeof(objDataLen) );
	file << objDataLen;
	file.SeekTo( curPos );
}

void SaveNodePropertiesLTA(CLTAFile* pFile, CWorldNode *pObj, uint32 level)
{
	//uint32			k;
	//CBaseProp		*pProp;

	PrependTabs(pFile, level);
	pFile->WriteStr("( properties" );
		if( strlen(pObj->GetClassName()) )
		{
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( name \"%s\" ) ", pObj->GetClassName() );
		}
		//save the ID of the property list
		PrependTabs(pFile, level+1);
		pFile->WriteStrF("( propid  %d ) ", g_propListContainer.AddPropList(&(pObj->m_PropList)) );

	PrependTabs(pFile, level);
	pFile->WriteStr(")" );
}

void SaveObjectList(CAbstractIO &file, CMoArray<CBaseEditObj*> &theList)
{
	uint32			i;
	CBaseEditObj	*pObj;

	file << (uint32)theList.GetSize();
	for( i=0; i < theList; i++ )
	{
		pObj = theList[i];
		SaveNodeProperties(file, pObj->AsNode());
	}
}

uint32 GetObjectListSize(CMoArray<CBaseEditObj*> &theList)
{
	//4 bytes for the object count
	uint32 nTotalSize = 4;

	for(uint32 nCurrObj = 0; nCurrObj < theList.GetSize(); nCurrObj++)
	{
		nTotalSize += GetNodePropertiesSize(theList[nCurrObj]->AsNode());
	}
	return nTotalSize;
}

void CopyObjectList( CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out )
{
	uint32			i;
	
	DeleteAndClearArray( out );
	
	out.SetSize( in.GetSize() );
	for( i=0; i < in.GetSize(); i++ )
	{
		out[i] = new CBaseEditObj;

		if(out[i])
		{
			out[i]->DoCopy(in[i]);
		}
	}
}

CBaseEditObj *FindMatchingObjectInList( CBaseEditObj *pObj, CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out )
{
	if (!pObj)
		return LTNULL;

	ASSERT(out.GetSize() == in.GetSize());
	for (uint32 nSearch = 0; nSearch < in.GetSize(); ++nSearch)
	{
		if (in[nSearch] == pObj)
			return out[nSearch];
	}

	// Note : This will happen when the object it's looking for is not in the list.  e.g. the root node.
	return LTNULL;
}

// Find a suitable parent, moving up the heirarchy until something is found
CBaseEditObj *FindParentObjectInList( CBaseEditObj *pObj, CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out )
{
	while (pObj)
	{
		// Search at this level of the heirarchy
		CBaseEditObj *pResult = FindMatchingObjectInList((CBaseEditObj*)pObj->GetParent(), in, out);
		if (pResult)
			return pResult;

		// Go up a level
		pObj = (CBaseEditObj*)pObj->GetParent();
	}

	// Didn't find anything, sorry.
	return LTNULL;
}

bool DuplicateObjectHeirarchy( CMoArray<CBaseEditObj*> &in, CMoArray<CBaseEditObj*> &out )
{
	// Note : The implementation of this function takes advantage of the list being in an array

	if (in.GetSize() != out.GetSize())
	{
		ASSERT(!"Object list size mismatch found in DuplicateObjectHeirarchy");
		return false;
	}

	for (uint32 nObjectLoop = 0; nObjectLoop < out.GetSize(); ++nObjectLoop)
	{
		CBaseEditObj *pInObj = in[nObjectLoop];
		CBaseEditObj *pOutObj = out[nObjectLoop];

		// Connect the parent
		CBaseEditObj *pParent = FindParentObjectInList(pInObj, in, out);
		pOutObj->SetParent(pParent);

		// Add the object to its parent's child list
		// Note : This changes the order of the children, but that's not something you can rely on anyway
		// This is a lot faster than an in-order children insert, and automatically handles removed levels
		// in the heirarchy due to null nodes and that sort of thing.
		if (pParent)
		{
			pParent->m_Children.AddTail(pOutObj);
		}
	}
	
	return true;
}


//------------------------------------------------------------------------------
//
//  CBaseEditObj::CBaseEditObj()
//
//  Purpose:		Defualt Constructor
//
//------------------------------------------------------------------------------
CBaseEditObj::CBaseEditObj()
{
	// Init misc. editor data.
	m_Type				= Node_Object;
	m_pRegion			= NULL;

#ifdef DIRECTEDITOR_BUILD
	m_bSearchForDims	= TRUE;
	m_pModelMgr			= NULL;
	m_pClassImageFile	= NULL;

	//default the radius to something very large. Rendering will update this to be
	//a much more reasonable value.
	SetVisibleRadius(10000.0f);
#endif
}

//------------------------------------------------------------------------------
//
//  CBaseEditObj::~CBaseEditObj()
//
//  Purpose:		Destructor
//
//------------------------------------------------------------------------------
CBaseEditObj::~CBaseEditObj()
{
#ifdef DIRECTEDITOR_BUILD
	ReleaseModelHandle();
#endif

	Term();
}


//------------------------------------------------------------------------------
//
//  CBaseEditObj::CBaseEditObj( const CBaseEditObj& src )
//
//  Purpose:	Copy Constructor
//
//------------------------------------------------------------------------------
CBaseEditObj::CBaseEditObj( const CBaseEditObj& src )
{
	// Call assignment operator...
	this->operator=( src );
}

//------------------------------------------------------------------------------
//
//  CBaseEditObj& CBaseEditObj::operator=( const CBaseEditObj& src )
//
//  Purpose:	Assignment operator
//
//------------------------------------------------------------------------------
CBaseEditObj& CBaseEditObj::operator=( const CBaseEditObj& src )
{
//	m_ArrowColor = src.m_ArrowColor;
//	m_BoxColor = src.m_BoxColor;
	m_Vert = src.m_Vert;

	m_PropList.CopyValues(( CPropList * ) &src.m_PropList );
	SetClassName(src.GetClassName() );

#ifdef DIRECTEDITOR_BUILD
	SetNodeLabel(src.GetNodeLabel());	
	SetVisibleRadius(src.GetVisibleRadius());
#endif // DIRECTEDITOR_BUILD

	return *this;
}

void CBaseEditObj::SetPos(const LTVector &v)
{
	CWorldNode::SetPos(v);
}

void CBaseEditObj::SetOr(const LTVector &v)
{
	CWorldNode::SetOr(v);
}

void CBaseEditObj::DoCopy(CWorldNode *pOther)
{
	m_Vert = pOther->AsObject()->m_Vert;

#ifdef DIRECTEDITOR_BUILD
	CBaseEditObj* pObj = pOther->AsObject();

	//copy over the class icon
	m_pClassImageFile = pObj->m_pClassImageFile;


	//copy over the model handle if we need to
	if(pObj->m_pModelMgr && pObj->GetModelHandle().IsValid())
	{
		//they have a handle, so let us grab a reference and store it
		m_pModelMgr = pObj->m_pModelMgr;
		const char* pszModelName = m_pModelMgr->GetFilename(pObj->GetModelHandle());
		m_pModelMgr->AddModel(pszModelName, m_ModelHandle);
	}
#endif


	CWorldNode::DoCopy(pOther);
}

#ifdef DIRECTEDITOR_BUILD
/************************************************************************/
// Initializes the dims for this object
void CBaseEditObj::InitDims()
{
	// Terminate the dims array
	TermDims();

	// Remember we've got our dims, in case something calls us while we're handling this function
	m_bSearchForDims=FALSE;

	// Search each property
	int i;
	for( i=0; i < m_PropList.m_Props.GetSize( ); i++ )
	{
		// Get the property
		CBaseProp *pProp = m_PropList.m_Props[i];
		ASSERT(pProp);

		// Make sure that this property has the dims flag set
		if (!(pProp->m_PropFlags & PF_DIMS))
		{
			// Go to the next property
			continue;
		}

		
		// [4/26/00 KLS] Check for special static list dims specification...
		if (pProp->m_PropFlags & PF_STATICLIST)
		{
			ClassDef *pDef = GetProject()->FindClassDef(GetClassName());

			if(pDef)
			{
				if (pDef->m_PluginFn)
				{
					IObjectPlugin* pPlugin = pDef->m_PluginFn();
					if (pPlugin)
					{
						char szFilename[255];
						szFilename[0] = '\0';
						LTVector vDims(0, 0, 0);

						if (LT_OK == pPlugin->PreHook_Dims(GetProject()->m_BaseProjectDir, 
							((CStringProp * )pProp)->m_String, szFilename, sizeof(szFilename), vDims))
						{
							if (szFilename[0])
							{
								// Get the filename for the model
								CString fileName = dfm_BuildName( GetProject()->m_BaseProjectDir, szFilename);

								// Get the dims
								if (GetProjectBar( )->GetModelDims((char *)(LPCTSTR)fileName, &vDims))
								{					
									// Add the dims and its flags
									m_dimsArray.Add(vDims);
									m_dimsFlagArray.Add(pProp->m_PropFlags & (PF_DIMS | PF_LOCALDIMS));
								}	
							}
							else if (vDims.x > 0.0f && vDims.y > 0.0f && vDims.z > 0.0f)
							{
								// Add the dims and its flags
								m_dimsArray.Add(vDims);
								m_dimsFlagArray.Add(pProp->m_PropFlags & (PF_DIMS | PF_LOCALDIMS));
							}

							// Ultra-mega-hack to feed the filename back into any hidden PF_MODEL properties
							CStringProp *pFilenameProp = (CStringProp*)m_PropList.GetPropByFlagsAndType(PF_HIDDEN | PF_MODEL, PT_STRING);
							if (pFilenameProp)
							{
								pFilenameProp->SetString(szFilename);
								OnPropertyChanged(pFilenameProp, true, NULL);
							}
						}
						else if (vDims.x > 0.0f && vDims.y > 0.0f && vDims.z > 0.0f)
						{
							// Add the dims and its flags
							m_dimsArray.Add(vDims);
							m_dimsFlagArray.Add(pProp->m_PropFlags & (PF_DIMS | PF_LOCALDIMS));
						}
						else
						{
							ASSERT(FALSE);
						}
					}
					else
					{
						ASSERT(FALSE);
					}
				}
				else
				{
					ASSERT(FALSE);
				}
			}
		}
		else
		{
			// Handle the vector property
			switch (pProp->m_Type)
			{
			case LT_PT_VECTOR:
				{
					// Add the dims and its flags
					m_dimsArray.Add(((CVectorProp * )pProp)->m_Vector);
					m_dimsFlagArray.Add(pProp->m_PropFlags & (PF_DIMS | PF_LOCALDIMS));
					break;
				}
			case LT_PT_STRING:
				{
					// Get the filename for the model
					CString fileName = dfm_BuildName( GetProject()->m_BaseProjectDir, (( CStringProp * )pProp )->m_String );

					// Get the dims
					CVector vDims;
					if( GetProjectBar( )->GetModelDims((char *)(LPCTSTR)fileName, &vDims ))
					{					
						// Add the dims and its flags
						m_dimsArray.Add(vDims);
						m_dimsFlagArray.Add(pProp->m_PropFlags & (PF_DIMS | PF_LOCALDIMS));
					}	
					break;
				}
			default:
				{
					ASSERT(FALSE);
					break;
				}
			}
		}
	}
}

//updates the image used for this object
void CBaseEditObj::UpdateObjectClassImage()
{
	//clear out the old object class image
	m_pClassImageFile = NULL;

	//we need to build up a texture name for this object of the form
	// directory\classname.dtx
	CString sFile(GetApp()->GetOptions().GetDisplayOptions()->GetClassIconsDir());
	sFile.TrimLeft("\\/");
	sFile.TrimRight("\\/");
	sFile += "\\";

	//now add our class name
	sFile += GetClassName();
	sFile += ".dtx";
	
	//try and load in the image
	if(dfm_GetFileIdentifier(GetFileMgr(), sFile, &m_pClassImageFile) != DFM_OK)
	{
		//we have failed
		m_pClassImageFile = NULL;		
	}
}


//determines if the designated file exists
static BOOL DoesFileExist(const char* pszFileName)
{
	CFile InFile;
	if(InFile.Open(pszFileName, CFile::modeRead) == TRUE)
	{
		InFile.Close();
		return TRUE;
	}

	return FALSE;
}

//retreives the name of the file that should be used
bool CBaseEditObj::UpdateModelHandle(const char* pszFileString, CModelMgr& ModelMgr)
{
	//build up the actual file name
	CString sFileString(pszFileString);

	CString sFilename;

	//see if this is absolute or not
	if(CHelpers::IsFileAbsolute(pszFileString))
	{
		sFilename = pszFileString;
	}
	else
	{
		sFilename = dfm_BuildName( GetProject()->m_BaseProjectDir, pszFileString);
	}

	//take off all the whitepsace on the string to be safe
	sFilename.TrimLeft();
	sFilename.TrimRight();

	//now we need to get the extension
	char pszFile[MAX_PATH];
	char pszExtension[MAX_PATH];

	CHelpers::ExtractFileNameAndExtension(sFilename, pszFile, pszExtension);

	//see if we have a valid extension, and if we don't, we need to try different files
	if((stricmp(pszExtension, "lta") != 0) && (stricmp(pszExtension, "ltc") != 0))
	{
		//first check for an LTA extension
		sFilename.Format("%s.lta", pszFile);
		if(::DoesFileExist(sFilename) == FALSE)
		{
			//no LTA extension
			sFilename.Format("%s.ltc", pszFile);
		}
	}

	//see if this filename happens to match the model we are currently using
	const char* pszOldFilename = ModelMgr.GetFilename(GetModelHandle());

	if(pszOldFilename)
	{
		if(stricmp(sFilename, pszOldFilename) == 0)
		{
			//the files match, don't bother trying to add it
			return true;
		}
	}

	//make sure that we add the reference first, to prevent releasing a model, to only
	//load it back again
	CModelHandle NewHandle;
	ModelMgr.AddModel(sFilename, NewHandle);

	if(!NewHandle.IsValid())
	{
		//we couldn't add the file, so release our old handle
		ReleaseModelHandle();

		//notify that there was a failure
		return false;
	}

	//now release our stuff
	ReleaseModelHandle();

	SetModelHandle(NewHandle);
	m_pModelMgr		= &ModelMgr;

	return true;
}

// releases the current model handle
void CBaseEditObj::ReleaseModelHandle()
{
	//release the old handle
	if(m_pModelMgr)
	{
		m_pModelMgr->ReleaseHandle(GetModelHandle());
		m_pModelMgr = NULL;
	}
}

// Notification that a property of this object has changed
void CBaseEditObj::OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers)
{
	//see if this property had a model associated with it
	if((pProperty->m_PropFlags & PF_MODEL) && (pProperty->m_Type == LT_PT_STRING))
	{
		//the model name was changed we should update the handle
		UpdateModelHandle(((CStringProp*)pProperty)->GetString(), GetApp()->GetModelMgr());

		//update the views to match
		CRegionDoc* pDoc = GetActiveRegionDoc();
		if( pDoc )
		{
			pDoc->RedrawAllViews();
		}
		
	}

	// [10/01/01 ARP] Notify the object...

	if( bNotifyGame && (pProperty->m_PropFlags & PF_NOTIFYCHANGE) )
	{
		ClassDef *pDef = GetProject()->FindClassDef(GetClassName());
		IObjectPlugin* pPlugin = LTNULL;

		while( pDef )
		{
			if( pDef->m_PluginFn )
			{			
				pPlugin = pDef->m_PluginFn();
				if( pPlugin )
				{
					GenericProp	gpPropValue;
					CString		sModifiers( pModifiers );

					// Setup the new value for this property...

					gp_Init( &gpPropValue );
					pProperty->SetupGenericProp( &gpPropValue );

					// Give the object a chance to do some debugging on the new value...

					pPlugin->PreHook_PropChanged( GetName(),
												  pProperty->GetName(),
												  pProperty->m_Type,
												  gpPropValue,
												  this,
												  sModifiers );

					delete pPlugin;
				}

				break;
			}

			// The class didn't have the plugin fn, see if the parent does
			pDef = pDef->m_ParentClass;
		}
	}
}

////////////////////////////////////
//	ILTPreInterface implementation

LTRESULT CBaseEditObj::Parse( ConParse *pParse )
{
	return pParse->Parse() ? LT_OK : LT_FINISHED;
}

LTRESULT CBaseEditObj::CPrint( const char *pMsg, ... )
{
	va_list marker;
	char msg[500];

	va_start(marker, pMsg);
	vsprintf(msg, pMsg, marker);
	va_end(marker);

	AddDebugMessage(msg);

	return LT_OK;
}

LTRESULT CBaseEditObj::ShowDebugWindow( bool bShow )
{
	GetDebugDlg()->ShowWindow( bShow );

	return LT_OK;
}

LTRESULT CBaseEditObj::GetObject( const char* pObjName, CBaseEditObj **ppObj )
{
	// Get the document
	CRegionDoc* pDoc = GetActiveRegionDoc();
	if (!pDoc)
	{		
		return LT_ERROR;
	}

	// Get the region
	CEditRegion* pRegion = pDoc->GetRegion();
	if (!pRegion)
	{	
		return LT_ERROR;
	}

	CPrefabMgr* pPrefabMgr = pRegion->GetPrefabMgr();
	if( !pPrefabMgr )
	{
		return LT_ERROR;
	}

	// Check if this object belongs to a prefab...

	char szName[MAX_STRINGPROP_LEN] = {0};
	SAFE_STRCPY( szName, pObjName );

	char* pPrefabName = strtok( szName, "." );

	CWorldNode* pNode = pRegion->FindNodeByName( pPrefabName );
	if( pNode && pNode->GetType() == Node_PrefabRef )
	{
		// We have a prefab ref. Now get its actual loaded prefab and look for the object in it's region...
		
		CPrefabRef* pPrefabRef = (CPrefabRef*)pNode;
		if( pPrefabRef )
		{
			char* pObjectName = strtok( NULL, "" );

			if( pPrefabMgr->FindObjectsByNameInPrefab( pPrefabRef->GetPrefabFilename(), pObjectName, ppObj, 1 ) > 0 )
			{
				if( ppObj )
				{
					return LT_OK;
				}
			}
		}
	}

	// Try to find the object

	if( pRegion->FindObjectsByName( pObjName, ppObj, 1 ) > 0 )
	{
		if( ppObj )
		{
			return LT_OK;
		}
	}

	return LT_NOTFOUND;
}

LTRESULT CBaseEditObj::FindObject( const char *pName )
{
	CBaseEditObj *pObj;
	return GetObject( pName, &pObj );
}

const char* CBaseEditObj::GetObjectClass( const char *pObjName )
{
	CBaseEditObj *pObj = LTNULL;
	if( GetObject( pObjName, &pObj ) == LT_OK )
	{
		return (pObj ? pObj->GetClassName() : LTNULL);
	}
	
	return LTNULL;
}

const char* CBaseEditObj::GetProjectDir( )
{
	CEditProjectMgr *pEditProjMgr = GetProject();
	if( !pEditProjMgr ) return LTNULL;

	return (pEditProjMgr->m_BaseProjectDir.IsEmpty() ? LTNULL : (const char*)pEditProjMgr->m_BaseProjectDir);
}	

const char* CBaseEditObj::GetWorldName( )
{
	// Get the document
	CRegionDoc *pDoc = GetActiveRegionDoc();
	if( !pDoc )
		return LTNULL;

	return (pDoc->m_WorldName.IsEmpty() ? LTNULL : (const char*)pDoc->m_WorldName );
}

LTRESULT CBaseEditObj::GetPropGeneric( const char* pObjName, const char* pPropName, GenericProp *pProp )
{
	gp_Init( pProp );
	
	if( !pObjName[0] || !pPropName[0] )
		return LT_NOTFOUND;

	CBaseEditObj *pObject = LTNULL;
	if( GetObject( pObjName, &pObject ) == LT_OK )
	{
		if( !pObject )
			return LT_ERROR;

		CPropList *pProps = pObject->GetPropertyList();
			
		if( !pProps )
			return LT_NOTFOUND;

		for( uint32 nCurrProp = 0; nCurrProp < pProps->GetSize(); ++nCurrProp )
		{
			// Get the property...
			
			CBaseProp *pCurProp = pProps->GetAt(nCurrProp);
			
			if( !_stricmp( pCurProp->GetName(), pPropName ))
			{
				pCurProp->SetupGenericProp( pProp );
				
				return LT_OK;
			}
		}
	}

	return LT_NOTFOUND;
}

#endif // DIRECTEDITOR_BUILD
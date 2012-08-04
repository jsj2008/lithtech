//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditProjectMgr.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : December 16 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "oldtypes.h"
#include "objectimporter.h"
#include "editprojectmgr.h"
#include "resource.h"
#include "texture.h"
#include "edithelpers.h"
#include "iobjectplugin.h"
#include "deditinternal.h"
#include "objectselfilterdlg.h"

#define TEXTURE_TYPECODE		1


static CString edExtension;
static CString datExtension;
static CString texExtension;
static CString sndExtension;
static CString mdlExtension;
static CString sprExtension;



// called by the file manager before deleting a file identifier to clean up user data
static void EPM_GoingAwayFn(DFileIdent *pIdent, void *pUser)
{
	// file identifier is a texture, delete the texture
	if(pIdent->m_pUser && pIdent->m_UserType == 0)
	{
		dib_DestroyDibTexture((CTexture*)pIdent->m_pUser);
	}
}


static char* _CopyStringPointer(char *pIn)
{
	char *pRet;

	if(pIn)
	{
		pRet = (char*)malloc(strlen(pIn) + 1);
		strcpy(pRet, pIn);
	}
	else
	{
		pRet = (char*)malloc(1);
		pRet[0] = 0;
	}

	return pRet;
}


static void _ExtractClassHeirarchy(ClassDef *pClass, ClassDef **pHeirarchy, int *pnInHeirarchy)
{
	int i, nInHeirarchy;
	ClassDef *pCur;

	
	// Count the number of classes.
	nInHeirarchy = 0;
	pCur = pClass;
	while(pCur)
	{
		++nInHeirarchy;
		pCur = pCur->m_ParentClass;
	}

	*pnInHeirarchy = nInHeirarchy;

	// Now put them in the heirarchy.
	pCur = pClass;
	--nInHeirarchy;
	pCur = pClass;
	while(pCur)
	{
		pHeirarchy[nInHeirarchy] = pCur;
		--nInHeirarchy;
		pCur = pCur->m_ParentClass;
	}
}




CProjectClass::~CProjectClass()
{
	DeleteAndClearArray( m_Children );
}



CEditProjectMgr::CEditProjectMgr()
{
	m_hFileMgr = NULL;
	m_bProjectDirectorySet = FALSE;
	m_ClassDefs = NULL;
	m_nClassDefs = 0;

	m_pTemplateObjectImporter=new CObjectImporter;

	m_hModule = NULL;
}


CEditProjectMgr::~CEditProjectMgr()
{
	Term();	

	delete []m_pTemplateObjectImporter;
	m_pTemplateObjectImporter=NULL;
}


void CEditProjectMgr::OnOpenProject(const char *pBaseDir)
{
	Term();

	ASSERT(!m_hFileMgr);
	
	m_DibMgr.Init( AfxGetInstanceHandle(), GetDesktopWindow() );
	m_BaseProjectDir = pBaseDir;
	m_bProjectDirectorySet = TRUE;
	m_hFileMgr = dfm_Open((char*)pBaseDir);

	UpdateClasses();

	//update the filter dialog to reflect the new classes
	GetObjectSelFilterDlg()->UpdateClassListing();
}


void CEditProjectMgr::Term()
{
	if(m_hFileMgr)
	{
		// EPM_GoingAwayFn is at the top of this file.
		dfm_Close(m_hFileMgr, EPM_GoingAwayFn, this);
		m_hFileMgr = NULL;
	}

	UnloadBinaries();
	m_DibMgr.Term( );
	m_bProjectDirectorySet = FALSE;

	//update the filter dialog to reflect no classes
	GetObjectSelFilterDlg()->ClearClassListing();
}

void CEditProjectMgr::UpdateBUTFile()
{
	//get the resource directory for the currently open project, and add on
	//our filename to get the BUT file
	CString sBUTFile = m_BaseProjectDir + "\\ClassHlp.but";

	//now create a but manager to load this
	CButeMgr* pNewMgr = new CButeMgr;

	//bail if memory failed
	if(pNewMgr == NULL)
		return;

	if (!pNewMgr->Parse(sBUTFile))
	{
		//failed to find a new but file, no worries, just bail
		delete pNewMgr;
		return;
	}

	//this worked, we want to override the old but file with this one
	GetApp()->SetClassHelpButeAgg(pNewMgr);
	
}

void CEditProjectMgr::UpdateClasses()
{
	DeleteAndClearArray(m_Classes);

	// Load all binaries.
	LoadBinaries();

	// Make the class tree.
	MakeClassTree();

	// Update the BUT file if necessary
	UpdateBUTFile();
}


void CEditProjectMgr::LoadBinaries()
{
	CString dllFilename, str;
	DWORD i, j;
	int status, version, nClasses;
	ClassDef **pClasses, *pInClass, *pOutClass;
	HKEY hKey;
	DWORD valType, dataBufSize;
	BYTE dataBuf[261];


	UnloadBinaries();


	if(!m_bProjectDirectorySet)
		return;

	// Look in the project directory for the object DLL.
	dllFilename = dfm_BuildName(m_BaseProjectDir, "object.lto");
	status = cb_LoadModule((char*)(LPCTSTR)dllFilename, NULL, &m_hModule, &version);

	if(status != CB_NOERROR)
	{
		if(status == CB_CANTFINDMODULE)
		{
			
			// If we can't load it, let's check out the error.
			DWORD dwErrorCode = GetLastError();

			// Allocacte a string containing a human readable description of the error
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);

            // Print out to the DEdit debug window.
			GetDebugDlg()->AddMessage(" Failed to load:\n %s\n"
									  " System Error Code: %d\n"
									  " Description: %s\n", dllFilename.GetBuffer(), dwErrorCode, (LPCTSTR)lpMsgBuf);

			// deallocate the string
			LocalFree( lpMsgBuf );

			str.Format(IDS_CANTFINDCLASSMODULE, dllFilename.GetBuffer());
		}
		else if(status == CB_NOTCLASSMODULE)
		{
			str.Format(IDS_INVALIDCLASSMODULE, dllFilename.GetBuffer());
		}
		else if(status == CB_VERSIONMISMATCH)
		{
			str.Format(IDS_INVALIDCLASSMODULEVERSION, dllFilename.GetBuffer(), SERVEROBJ_VERSION, version);
		}

		AppMessageBox(str, MB_OK);
		return;
	}
	
	// Get the ClassDefs.
	nClasses = cb_GetNumClassDefs(m_hModule);
	pClasses = cb_GetClassDefs(m_hModule);

	// Copy the class data over.
	m_ClassDefs = (ClassDef*)malloc(sizeof(ClassDef) * nClasses);
	m_nClassDefs = nClasses;

	// Prepare some temporary data for editstring lists
	const DDWORD cMaxStrings = 512;
	const DDWORD cMaxStringLen = 256;
	char* aszStrings[cMaxStrings];
	{for ( int iString = 0 ; iString < cMaxStrings ; iString++ )
	{
		aszStrings[iString] = new char[cMaxStringLen];
	}}

	int cProps = 0;

	// Read in all class definitions
	for(i=0; i < m_nClassDefs; i++)
	{
		pInClass = pClasses[i];
		pOutClass = &m_ClassDefs[i];
		
		memcpy(pOutClass, pInClass, sizeof(ClassDef));

		pOutClass->m_ClassName = _CopyStringPointer(pInClass->m_ClassName);

		// {BP 2/11/98}
		// Only allocate if there are some props to deal with...
		if( pInClass->m_nProps )
		{
			pOutClass->m_Props = (PropDef*)malloc(sizeof(PropDef) * pInClass->m_nProps);
			memcpy(pOutClass->m_Props, pInClass->m_Props, sizeof(PropDef) * pInClass->m_nProps);

			cProps += pInClass->m_nProps;

			for(j=0; j < pInClass->m_nProps; j++)
			{
				PropDef* pPropDef = &pOutClass->m_Props[j];
				pPropDef->m_PropName = _CopyStringPointer(pInClass->m_Props[j].m_PropName);
				pPropDef->m_DefaultValueString = _CopyStringPointer(pInClass->m_Props[j].m_DefaultValueString);
				
				// {BL 11/06/99}
				// See if this property supports EditStringLists

				ClassDef* pDef = pOutClass;
				while(pDef)
				{
					if(pDef->m_PluginFn && ((pPropDef->m_PropFlags & PF_STATICLIST) || (pPropDef->m_PropFlags & PF_DYNAMICLIST)) )
					{
						IObjectPlugin* pPlugin = pDef->m_PluginFn();
						if( pPlugin )
						{
							DDWORD cStrings = 0;
							if ( LT_OK == pPlugin->PreHook_EditStringList(m_BaseProjectDir, pPropDef->m_PropName,
																		  aszStrings, &cStrings, cMaxStrings, cMaxStringLen) )
							{
								DEditInternal* pDEditInternal = new DEditInternal;
								pPropDef->m_pDEditInternal = pDEditInternal;

								pDEditInternal->m_cStrings = cStrings;

								if( cStrings <= 0 )
 								{
 									pDEditInternal->m_aszStrings = NULL;
 								}
 								else
								{
									pDEditInternal->m_aszStrings = new char*[cStrings];
								
									for ( uint32 iString = 0 ; iString < cStrings ; iString++ )
									{
										pDEditInternal->m_aszStrings[iString] = _CopyStringPointer(aszStrings[iString]);
									}
								}

								delete pPlugin;
								break;
							}

							delete pPlugin;
						}
					}
					
					pDef = pDef->m_ParentClass;
				}
			}
		}
		else
			pOutClass->m_Props = NULL;
	}

	for ( int iString = 0 ; iString < cMaxStrings ; iString++ )
	{
		delete [] aszStrings[iString];
	}

	// Setup the parent info.
	for(i=0; i < m_nClassDefs; i++)
	{
		pInClass = pClasses[i];
		pOutClass = &m_ClassDefs[i];

		if(pInClass->m_ParentClass)
		{
			pOutClass->m_ParentClass = FindClassDef(pInClass->m_ParentClass->m_ClassName);
			ASSERT(pOutClass->m_ParentClass);
		}
		else
		{
			pOutClass->m_ParentClass = NULL;
		}
	}		

	LoadTemplateClasses();
}

void CEditProjectMgr::LoadTemplateClasses()
{	
	// Clear the currently loaded templates
	DeleteAndClearArray(m_TemplateClasses);

	// Load the template objects
	CObjectImporter *pImporter=GetTemplateObjectImporter();
	if (!pImporter)
	{
		return;
	}

	if (!pImporter->LoadObjectFile("lithtech.cls"))
	{
		return;
	}

	// Create each class
	int i;
	for (i=0; i < pImporter->GetNumObjects(); i++)
	{
		// Allocate the class
		TemplateClass *pClass=new TemplateClass;		

		// Get the class name and parent type
		CString sName=pImporter->GetObjectName(i);
		CString sParentType=pImporter->GetObjectType(i);

		SAFE_STRCPY(pClass->m_ClassName, sName);
		SAFE_STRCPY(pClass->m_ParentClassName, sParentType);
		m_TemplateClasses.Append(pClass);
	}
}


void CEditProjectMgr::UnloadBinaries()
{
	DWORD i, j;
	ClassDef *pClass;

	DeleteAndClearArray(m_TemplateClasses);
	DeleteAndClearArray(m_Classes);

	if(m_ClassDefs)
	{
		for(i=0; i < m_nClassDefs; i++)
		{
			pClass = &m_ClassDefs[i];

			for(j=0; j < pClass->m_nProps; j++)
			{
				free(pClass->m_Props[j].m_PropName);
				free(pClass->m_Props[j].m_DefaultValueString);

				// {BL 11/07/99}
				// Deallocate all edit string lists
				DEditInternal* pDEditInternal = pClass->m_Props[j].m_pDEditInternal;
				if ( pDEditInternal && pDEditInternal->m_aszStrings && (pDEditInternal->m_cStrings > 0) )
				{
					for ( int iString = 0 ; iString < pDEditInternal->m_cStrings ; iString++ )
					{
						delete pDEditInternal->m_aszStrings[iString];
						pDEditInternal->m_aszStrings[iString] = NULL;
					}

					delete [] pDEditInternal->m_aszStrings;
					pDEditInternal->m_aszStrings = NULL;
					pDEditInternal->m_cStrings = 0;
				}
				delete pDEditInternal;
			}
		
			free(pClass->m_ClassName);

			// {BP 2/11/98}
			// Only deallocate if there were props allocated...
			if( pClass->m_Props )
				free(pClass->m_Props);
		}

		free(m_ClassDefs);
	
		m_ClassDefs = NULL;
		m_nClassDefs = 0;
	}


	// [4/26/00 KLS] make sure we free this bad boy...
	if (m_hModule)
	{
		cb_UnloadModule(m_hModule);
		m_hModule = NULL;
	}
}


void CEditProjectMgr::MakeClassTree()
{
	int i, j;
	uint32 iClass;
	ClassDef *pClass, *pHeir;
	CMoArray<CProjectClass*> *pArray;
	CProjectClass *pFound, *pProjectClass;
	ClassDef *heirarchy[512];
	int nInHeirarchy;


	if(!m_ClassDefs)
		return;
	
	
	for(i=0; i < m_nClassDefs; i++)
	{
		pClass = &m_ClassDefs[i];
		pArray = &m_Classes;

		_ExtractClassHeirarchy(pClass, heirarchy, &nInHeirarchy);
		for(j=0; j < nInHeirarchy; j++)
		{
			pHeir = heirarchy[j];
		
			if( !(pFound = FindClassInArray(pHeir, pArray)) )
			{
				pFound = new CProjectClass;
				
				pFound->m_pClass = pHeir;
				pFound->m_pClass->DEditSetClassHook(pFound);

				pArray->Append(pFound);
			}

			pArray = &pFound->m_Children;
		}
	}

	for(iClass=0; iClass < m_Classes; iClass++)
	{
		pProjectClass = m_Classes[iClass];

		RecurseAndBuildClassDefPropList(pProjectClass);
	}
}


CProjectClass* CEditProjectMgr::FindClassInArray(ClassDef *pClass, CMoArray<CProjectClass*> *pArray)
{
	for( DWORD i=0; i < pArray->GetSize(); i++ )
		if(pArray->Get(i)->m_pClass == pClass)
			return pArray->Get(i);

	return NULL;
}


void CEditProjectMgr::LoadProjectFile( CAbstractIO &file )
{
	DWORD		extra=0;


	if( file.GetLen() == 0 )
	{
	}
	else
	{
		file >> extra >> extra >> extra >> extra;
	}
}


void CEditProjectMgr::SaveProjectFile( CAbstractIO &file )
{
	DWORD		extra=0;

	file << extra << extra << extra << extra;
}


ClassDef* CEditProjectMgr::FindClassDef(const char *pName)
{
	DWORD i;

	for(i=0; i < m_nClassDefs; i++)
	{
		if(strcmp(m_ClassDefs[i].m_ClassName, pName) == 0)
		{
			return &m_ClassDefs[i];
		}
	}

	return NULL;
}


PropDef* CEditProjectMgr::FindPropInList(CMoArray<PropDef*> &propList, char *pPropName, DWORD *pIndex)
{
	DWORD i;

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


void CEditProjectMgr::RecurseAndBuildClassDefPropList(
	CProjectClass *pProjectClass)
{
	uint32 i;

	BuildClassDefPropList(
		pProjectClass->m_pClass,
		pProjectClass->m_Props);

	for(i=0; i < pProjectClass->m_Children; i++)
	{
		RecurseAndBuildClassDefPropList(pProjectClass->m_Children[i]);
	}
}


void CEditProjectMgr::BuildClassDefPropList(
	ClassDef *pClass, 
	CMoArray<PropDef*> &propList)
{
	int i, chainLen;
	DWORD j, propIndex, curPropIndex;
	ClassDef *pCurClass;
	CMoArray<ClassDef*> classChain;
	PropDef *pProp, *pAlreadyThere;
	CMoArray<PropDef*> unhiddenProps, hiddenProps;


	propList.Term();
	propList.SetCacheSize(128);
	hiddenProps.SetCacheSize(128);
	unhiddenProps.SetCacheSize(128);


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
	for(i=0; i < classChain; i++)
	{
		pCurClass = classChain[i];

		for(j=0; j < pCurClass->m_nProps; j++)
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
	for(i=0; i < propList; i++)
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
	for(i=0; i < unhiddenProps; i++)
		propList[curPropIndex++] = unhiddenProps[i];

	for(i=0; i < hiddenProps; i++)
		propList[curPropIndex++] = hiddenProps[i];
}


CMoArray<PropDef*>* CEditProjectMgr::GetClassDefProps(
	ClassDef *pClass)
{
	CProjectClass *pProjectClass;

	
	if(pProjectClass = pClass->DEditGetClassHook())
	{
		return &pProjectClass->m_Props;
	}
	else
	{
		return NULL;
	}
}


CProjectClass* CEditProjectMgr::FindClassInTree( const char *pName )
{
	return RecurseAndFindClass(m_Classes, pName);
}


CProjectClass* CEditProjectMgr::RecurseAndFindClass( CMoArray<CProjectClass*> &classes, const char *pName )
{
	DWORD				i;
	CProjectClass		*pClass;


	for( i=0; i < classes; i++ )
		if(strcmp(classes[i]->m_pClass->m_ClassName, pName) == 0)
			return classes[i];

	for( i=0; i < classes; i++ )
	{
		pClass = RecurseAndFindClass( classes[i]->m_Children, pName );
		if( pClass )
			return pClass;
	}
	
	return NULL;
}


TemplateClass* CEditProjectMgr::FindTemplateClass(const char *pName)
{
	DWORD i;

	for(i=0; i < m_TemplateClasses; i++)
	{
		if(stricmp(m_TemplateClasses[i]->m_ClassName, pName) == 0)
			return m_TemplateClasses[i];
	}

	return NULL;
}


/************************************************************************/
// Fills sDestBaseClass in with the base class of sClassName.  FALSE is returned if
// sClassName could not be found.
BOOL CEditProjectMgr::GetBaseClassName(CString sClassName, CString &sDestBaseClass)
{	
	// Find this class
	CProjectClass *pClass=FindClassInTree(sClassName);
	if (!pClass)
	{
		return FALSE;
	}
	
	// Get the base class name
	ASSERT(pClass->m_pClass);
	ClassDef *pBaseClass=pClass->m_pClass->m_ParentClass;
	if (pBaseClass && pBaseClass->m_ClassName)
	{
		sDestBaseClass=pBaseClass->m_ClassName;
	}
	else
	{
		sDestBaseClass=sClassName;
	}

	return TRUE;
}


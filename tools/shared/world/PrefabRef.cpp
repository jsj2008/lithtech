//------------------------------------------------------
// PrefabRef.cpp
//
// Prefab reference object implementation
//
// Author: Kevin Francis
// Created: 03/15/2001
// Modification History: 
//		03/15/2001 - First implementation
//
//------------------------------------------------------

#include "bdefs.h"

#include "prefabref.h"
#include "editobjects.h"
#include "node_ops.h"
#include "geomroutines.h"
#include "lteulerangles.h"
#include "editbrush.h"

#ifdef DIRECTEDITOR_BUILD
#include "edit_actions.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Some recursive functions I couldn't find anywhere else in DEdit for doing
// heirarchy-type manipulations

struct SInstantiateParams
{
	SInstantiateParams(CEditRegion *pRegion) : 
		m_pRegion(pRegion) 
	{
		m_pName			= NULL;
		m_nNameLen		= 0;
		m_pParent		= NULL;
		m_ErrorCallback	= NULL;

		m_vPos.Init(0, 0, 0);
		m_mTransform.Identity();
	}

	SInstantiateParams(const SInstantiateParams& rhs)
	{
		m_pName = NULL;
		*this = rhs;
	}

	~SInstantiateParams()
	{
		delete [] m_pName;
	}

	//assignment operator
	const SInstantiateParams& operator=(const SInstantiateParams& rhs)
	{
		m_pRegion		= rhs.m_pRegion;
		m_mTransform	= rhs.m_mTransform;
		m_vPos			= rhs.m_vPos;
		m_ErrorCallback	= rhs.m_ErrorCallback;

		//copy over the name
		m_nNameLen		= rhs.m_nNameLen;

		delete [] m_pName;
		m_pName = NULL;
		
		if(rhs.m_pName)
		{
			m_pName = new char [m_nNameLen + 1];
			strcpy(m_pName, rhs.m_pName);		
		}

#ifdef DIRECTEDITOR_BUILD
		m_pUndo			= rhs.m_pUndo;
#endif
		return *this;
	}

	//adds a prefab to the stack. This accumulates the transformation and names
	//appropriately
	void PushPrefab(CPrefabRef* pPrefabRef)
	{
		//first off, append the name
		if(m_pName && (strlen(m_pName) != 0))
		{
			//already a name in there, so we need to add a period
			AppendToString(".");
		}
		AppendToString(pPrefabRef->GetName());
		
		//now update the transformation

		//rotate
		LTMatrix mRotation;
		gr_SetupMatrixEuler(pPrefabRef->GetOr(), mRotation.m);

		//translate
		LTMatrix mTranslation;
		mTranslation.Identity();
		mTranslation.SetTranslation(pPrefabRef->GetPos());

		m_mTransform = m_mTransform * mTranslation * mRotation;	
		
		//save the translation
		m_vPos += pPrefabRef->GetPos();
	}

	//appends text onto the name string
	void AppendToString(const char* pszString)
	{
		//figure out the new length
		uint32 nNewLen = strlen(pszString) + m_nNameLen;

		//allocate the new memory
		char* pNewName = new char[nNewLen + 1];
		pNewName[0] = '\0';

		if(pNewName)
		{
			//copy over the old string
			if(m_pName)
				strncpy(pNewName, m_pName, nNewLen + 1);

			//append on the new stirng
			strncat(pNewName, pszString, nNewLen + 1);
		}
		else
		{
			nNewLen = 0;
		}

		//free the old string and set this one up as it
		delete [] m_pName;
		m_pName		= pNewName;
		m_nNameLen	= nNewLen;
	}

	//reports an error to the registered callback function 
	void ReportError(const char* pszString, ...) const
	{
		if(m_ErrorCallback)
		{
			char pszBuffer[1024];
			va_list marker;
			va_start(marker, pszString);
			LTVSNPrintF(pszBuffer, sizeof(pszBuffer), pszString, marker);
			va_end(marker);

			m_ErrorCallback(pszBuffer);
		}
	}


	// The edit region
	CEditRegion *		m_pRegion;
	// Length of the prefab name (to avoid recalculating it at every node)
	uint32				m_nNameLen;
	// The name of the prefab (to avoid having to search the parameter list at every node)
	char *				m_pName;
	// The base transform of the prefab
	LTMatrix			m_mTransform;
	// Position of the prefab
	LTVector			m_vPos;

	//the parent that the new prefabs should be placed under
	CWorldNode*			m_pParent;

	CPrefabRef::TInstantiateErrorCallback	m_ErrorCallback;


#ifdef DIRECTEDITOR_BUILD
	PreActionList *		m_pUndo;
#endif
};

// Convert #'s in the parameters to the name of the prefabref
// and add the parent's name to the beginning of the child's name
static void AddNameToChild(const SInstantiateParams &sParams, CWorldNode *pNode)
{
	// Add on to the child name
	const char *pChildName = pNode->GetName();
	char aTempBuffer[MAX_STRINGPROP_LEN+1];
	strcpy(aTempBuffer, sParams.m_pName);
	strcat(aTempBuffer, ".");
	strcat(aTempBuffer, pChildName);
	pNode->SetName(aTempBuffer);

	// Replace # in the properties
	CPropList *pPropList = pNode->GetPropertyList();
	for (uint32 nCurProp = 0; nCurProp < (uint32)pPropList->GetSize(); ++nCurProp)
	{
		CBaseProp *pProp = pPropList->GetAt(nCurProp);
		if (pProp->GetType() != LT_PT_STRING)
			continue;

		CStringProp *pStringProp = (CStringProp*)pProp;
		char *pValue;
		while ((pValue = strchr(pStringProp->GetString(), '#')) != NULL)
		{
			// Delete the #
			strcpy(pValue, &pValue[1]);
			if (sParams.m_nNameLen)
			{
				int nStrLen = strlen(pStringProp->GetString());
				// Figure how much we want to insert
				int nCopyLen = sParams.m_nNameLen;
				if ((nCopyLen + nStrLen) > MAX_STRINGPROP_LEN)
				{
					sParams.ReportError("Error in node \'%s\': Unable to expand property \'%s\' with a value of \'%s\'. The final string is too long.", pNode->GetName(), pStringProp->GetName(), pStringProp->m_String);
					nCopyLen = MAX_STRINGPROP_LEN - nStrLen;
				}
				// Make some space
				memmove(&pValue[nCopyLen], pValue, strlen(pValue) + 1);
				// Copy the name in
				memcpy(pValue, sParams.m_pName, nCopyLen);
			}
		}
	}
}

static void TransformChild(SInstantiateParams &sParams, CWorldNode *pNode)
{
	// Rotate the child
	pNode->Rotate(sParams.m_mTransform, LTVector(0.0f, 0.0f, 0.0f));
	// Handle the movement of a brush
	if(pNode->GetType() == Node_Brush)
	{
		CEditBrush *pBrush = pNode->AsBrush();
		pBrush->MoveBrush(pNode->GetPos() + pBrush->GetUpperLeftCornerPos());
	}
}

static CWorldNode* InternalInstantiatePrefab(SInstantiateParams& Params, CPrefabRef* pRef);
static void RecurseAndInstantiate(SInstantiateParams &sParams, CWorldNode *pDestRoot, const CWorldNode *pSrcRoot);

static CWorldNode* InternalInstantiatePrefab(SInstantiateParams& Params, CPrefabRef* pRef)
{
	//create the new instantiate parameters
	SInstantiateParams NewParams(Params);

	//allow this prefab to 
	NewParams.PushPrefab(pRef);

	// Add a null node to stick everything under
	CWorldNode *pResult = new CWorldNode;

	//safety check
	if(pResult == NULL)
	{
		return NULL;
	}

	//save the new parent
	NewParams.m_pParent	= pResult;

	pResult->SetNodeLabel(pRef->GetName());

	no_InitializeNewNode(Params.m_pRegion, pResult, Params.m_pParent);

#ifdef DIRECTEDITOR_BUILD
	if (Params.m_pUndo)
		Params.m_pUndo->AddTail(new CPreAction(ACTION_ADDEDNODE, pResult));
#endif

	
	RecurseAndInstantiate(NewParams, pResult, pRef->GetPrefabTree());

	return pResult;
}

static void RecurseAndInstantiate(SInstantiateParams &sParams, CWorldNode *pDestRoot, const CWorldNode *pSrcRoot)
{
	if((pSrcRoot == NULL) || (pDestRoot == NULL))
		return;

	GPOS iCurChild = pSrcRoot->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		CWorldNode *pChild = pSrcRoot->m_Children.GetNext(iCurChild);

		//see if this is a prefab
		if(pChild->GetType() == Node_PrefabRef)
		{
			//if this is a prefab, we need to instantiate it
			InternalInstantiatePrefab(sParams, (CPrefabRef*)pChild);
		}
		else
		{
			//this is not a prefab
			CWorldNode *pCopiedChild = pChild->AllocateSameKind();
			pCopiedChild->DoCopy(pChild);

			if(pCopiedChild->GetType() != Node_Null)
			{
				AddNameToChild(sParams, pCopiedChild);
				TransformChild(sParams, pCopiedChild);
			}


			no_InitializeNewNode(sParams.m_pRegion, pCopiedChild, pDestRoot);

#ifdef DIRECTEDITOR_BUILD
			if (sParams.m_pUndo)
			{
				sParams.m_pUndo->AddTail(new CPreAction(ACTION_ADDEDNODE, pCopiedChild));
			}
#endif
			RecurseAndInstantiate(sParams, pCopiedChild, pChild);
		}

	}
}

//////////////////////////////////////////////////////////////////////////////
// CPrefabRef object implementation

CPrefabRef::CPrefabRef() :
	m_pPrefabTree(LTNULL),
	m_sPrefabFilename(""),
	m_vMin(-2.5f, -2.5f, -2.5f),
	m_vMax(2.5f, 2.5f, 2.5f)
{
	m_Type = Node_PrefabRef;

	// Set the class name
	SetClassName("PrefabRef");
	
	// Add the default properties
	m_PropList.m_Props.Add(new CStringProp(g_NameName));
	m_PropList.m_Props.Add(new CVectorProp(g_PosName));
	m_PropList.m_Props.Add(new CRotationProp(g_AnglesName));
}

CPrefabRef::CPrefabRef(const CPrefabRef &cOther)
{
	m_Type = Node_PrefabRef;

	DoCopy(const_cast<CPrefabRef *>(&cOther));
}

CPrefabRef::~CPrefabRef()
{
}

void CPrefabRef::DoCopy(CWorldNode *pOther)
{
	ASSERT(pOther->m_Type == Node_PrefabRef);

	// Copy the base stuff
	CWorldNode::DoCopy(pOther);

	CPrefabRef *pOtherRef = (CPrefabRef*)pOther;

	m_pPrefabTree		= pOtherRef->m_pPrefabTree;
	m_sPrefabFilename	= pOtherRef->m_sPrefabFilename;
	m_vMin				= pOtherRef->m_vMin;
	m_vMax				= pOtherRef->m_vMax;
}
	
LTVector CPrefabRef::GetPos()
{
	return CWorldNode::GetPos();
}

void CPrefabRef::SetPos(const LTVector &v)
{
	CWorldNode::SetPos(v);
}

LTVector CPrefabRef::GetOr()
{
	return CWorldNode::GetOr();
}

void CPrefabRef::SetOr(const LTVector &v)
{
	CWorldNode::SetOr(v);
}

#ifdef DIRECTEDITOR_BUILD
CWorldNode *CPrefabRef::InstantiatePrefab(CEditRegion *pRegion, TInstantiateErrorCallback pErrorCallback, PreActionList *pUndo)
#else
CWorldNode *CPrefabRef::InstantiatePrefab(CEditRegion *pRegion, TInstantiateErrorCallback pErrorCallback)
#endif
{
	if (!m_pPrefabTree)
		return LTNULL;

	SInstantiateParams sParams(pRegion);
	sParams.m_pParent = GetParent();
	sParams.m_ErrorCallback = pErrorCallback;

#ifdef DIRECTEDITOR_BUILD
	sParams.m_pUndo = pUndo;
#endif

	
	CWorldNode* pResult = InternalInstantiatePrefab(sParams, this);


	return pResult;

}

#ifdef DIRECTEDITOR_BUILD
#include "regiondoc.h"
#include "editregion.h"
#include "edithelpers.h"
#include "genericprop_setup.h"
#include "prefabmgr.h"
#include "iobjectplugin.h"
#include "mainfrm.h"

void CPrefabRef::OnPropertyChanged( CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers )
{
	// Currently all this method does is notify the game code so just bail
	// if wea aren't supposed to notify the game...

	if( !bNotifyGame )
		return;

	CMainFrame *pFrame = GetMainFrame();
	CRegionDoc* pDoc = GetActiveRegionDoc();
	if( !pFrame || !pDoc )
	{
		return;
	}
	
	// Get the region
	CEditRegion* pRegion = pDoc->GetRegion();
	if (!pRegion)
	{	
		return;
	}

	CPrefabMgr* pPrefabMgr = pRegion->GetPrefabMgr();
	if( !pPrefabMgr )
	{
		return;
	}
	
	const CEditRegion *pPrefabRegion = pPrefabMgr->GetPrefabRegion( GetPrefabFilename() );
	if( !pPrefabRegion )
	{
		return;
	}

	CString		sModifiers( pModifiers );

	// Develop the modifier string so the game knows how to handle the change...

	if( pFrame->ShouldIgnoreMsgsInPrefabs() )
	{
		sModifiers += "IMsgErs;"; // ignore message errors 
	}
 

	CBaseEditObj *pObject = NULL;
	for( int i = 0; i < pPrefabRegion->m_Objects.GetSize(); ++i )
	{
		pObject = pPrefabRegion->m_Objects[i];
		
		CPropList *pProps = pObject->GetPropertyList();
		if( !pProps )
		{
			continue;
		}

		for( uint32 nCurrProp = 0; nCurrProp < pProps->GetSize(); ++nCurrProp )
		{
			// Get the reference property...
			
			CBaseProp *pRefProp = pProps->GetAt(nCurrProp);

			if( pRefProp->m_PropFlags & PF_NOTIFYCHANGE )
			{
				ClassDef *pDef = GetProject()->FindClassDef( pObject->GetClassName() );
				IObjectPlugin* pPlugin = LTNULL;

				while( pDef )
				{
					if( pDef->m_PluginFn )
					{			
						pPlugin = pDef->m_PluginFn();
						if( pPlugin )
						{
							GenericProp	gpPropValue;
							
							// Create a new property of the same type so we can change the new property's values instead of the originals...

							CBaseProp	*pTempProp = pRefProp->CreateSameKind();
							
							if( pRefProp->m_Type == PT_STRING )
							{
								// Copy into the temp property so we don't mess with the reference properties values..

								pTempProp->Copy( pRefProp );

								// Replace any #'s with the prefab name...
								
								CStringProp *pStringProp = (CStringProp*)pTempProp;
								char *pValue;
								while( (pValue = strchr(pStringProp->GetString(), '#') ) != NULL)
								{
									// Delete the #
									strcpy(pValue, &pValue[1]);
									int nNameLen = strlen( GetName() );
									if( nNameLen )
									{
										int nStrLen = strlen( pStringProp->GetString() );
										
										// Figure how much we want to insert
										int nCopyLen = nNameLen;
										if( (nCopyLen + nStrLen) > MAX_STRINGPROP_LEN )
										{
											nCopyLen = MAX_STRINGPROP_LEN - nStrLen;
										}
										
										// Make some space
										memmove(&pValue[nCopyLen], pValue, strlen(pValue) + 1);
										// Copy the name in
										memcpy(pValue, GetName(), nCopyLen);
									}
								}

								pRefProp = pTempProp;	
							}

							// Setup the new value for this property...

							gp_Init( &gpPropValue );
							pRefProp->SetupGenericProp( &gpPropValue );
							

							CString sName = GetName();
							sName += '.';
							sName += pObject->GetName();

							// Give the object a chance to do some debugging on the new value...

							pPlugin->PreHook_PropChanged( sName,
														  pRefProp->GetName(),
														  pRefProp->m_Type,
														  gpPropValue,
														  pObject,
														  sModifiers );

							delete pPlugin;
							delete pTempProp;
						}

						break;
					}

					// The class didn't have the plugin fn, see if the parent does
					pDef = pDef->m_ParentClass;
				}
		
			}
		}		
	}
}

#endif
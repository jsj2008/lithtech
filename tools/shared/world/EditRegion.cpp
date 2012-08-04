//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ------------------------------------------------------------------ //
//
//	FILE	  : EditRegion.cpp
//
//	PURPOSE	  : Implements the CEditRegion class.
//
//	CREATED	  : October 5 1996
//
//
// ------------------------------------------------------------------ //


// Includes....
#include "bdefs.h"
#include <stdarg.h>
#include "oldtypes.h"
#include "editregion.h"
#include "editpoly.h"
#include "geomroutines.h"
#include "node_ops.h"
#include "abstractio.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "streamsim.h"


#ifdef DIRECTEDITOR_BUILD
	#include "dedit.h"
	#include "regiondoc.h"
	#include "edithelpers.h"
	#include "nodeview.h"
	#include "edit_actions.h"	
	#include "texture.h"
#endif

#define MIN_DUPLICATE_DIST	0.001f
#define CURRENT_TBW_VERSION	1

CPropListContainer g_propListContainer;
CPropListContainer g_loadPropListContainer;

// ------------------------------------------------------------------ //
// Helper functions.
// ------------------------------------------------------------------ //

//file marker utilities
static uint32 PlaceMarker(CAbstractIO& Stream)
{
	uint32 nPos = Stream.GetCurPos();

	//just write out a dummy 4 bytes for now
	Stream << nPos;

	return nPos;
}

static void UpdateMarker(CAbstractIO& Stream, uint32 nMarker)
{
	uint32 nPos = Stream.GetCurPos();
	Stream.SeekTo(nMarker);

	Stream << nPos;

	Stream.SeekTo(nPos);
}

static CWorldNode* RecurseAndFindNodeByID(CWorldNode *pRoot, uint32 nID)
{
	CWorldNode *pRet;
	GPOS pos;

	if(pRoot->GetUniqueID() == nID)
		return pRoot;

	for(pos=pRoot->m_Children; pos; )
	{
		if(pRet = RecurseAndFindNodeByID(pRoot->m_Children.GetNext(pos), nID))
			return pRet;
	}

	return NULL;
}

//disable this warning. It is complaining about the this being used since the object
//could be in an incomplete state, but since it only stores it, it is safe
#pragma warning(disable:4355)

CEditRegion::CEditRegion()
{
	m_pStringHolder = &m_DefaultStringHolder;
	m_pInfoString = "";

	m_Selections.SetCacheSize( 100 );
	m_Objects.SetCacheSize( 100 );
	m_PathNodes.SetCacheSize( 50 );

	m_vMarker.Init( 0.0f, 0.0f, 0.0f );
	m_nLastNavigatorPos=0;

	m_RootNode.m_Flags |= NODEFLAG_WORLDROOT;

#ifdef DIRECTEDITOR_BUILD
	m_pActiveParentNode=GetRootNode();
#endif

}

//reenable the warning
#pragma warning(default:4355)



CEditRegion::~CEditRegion()
{
	Term();
}


void CEditRegion::Term()
{
	ClearSelections();
	ClearPathNodes();
	m_RootNode.Term();

	// Must SetSize() on these because just deleting them won't remove them from the list.
	m_Brushes.RemoveAll();
	m_Objects.SetSize(0);

	// Remove the navigator position items
	unsigned int i;
	for (i=0; i < m_NavigatorPosArray.GetSize(); i++)
	{
		delete m_NavigatorPosArray[i];
	}
	m_NavigatorPosArray.SetSize(0);
}


void LoadPropList(CLTANode* pPropList, CPropListContainer& container )
{
	// CLTANode* pPropList = PairCdrNode(propListArrayNode->GetElement(i));
	static char	propName[256];
	CPropList*	pDestPropList = new CPropList;

	const uint32 proplistSize = pPropList->GetNumElements();
	uint32 k = 0;
	// (  string "Name"( )( data "Prop59") )
	for( k=0; k < proplistSize; k++ )
	{
		
		BYTE				propCode;
		CBaseProp			*pProp;
		CLTANode			*pPropNode = pPropList->GetElement(k);		
		
		const char* typeString = GetString(pPropNode->GetElement(0));

		//default the property code to something invalid
		propCode = LT_NUM_PROPERTYTYPES;
		//do a quick binary split, to cut the number of strcmp's in half. Note, the
		//value is found by sorting the strings, and getting the first letter of
		//the first value in the upper half
		if(typeString[0] < 'r')
		{
			//search the lower half of the strings
			if( strcmp(typeString,"bool") == 0 )
				propCode = LT_PT_BOOL;
			else if( strcmp(typeString,"color") == 0 )
				propCode = LT_PT_COLOR;
			else if( strcmp(typeString,"flags") == 0 )
				propCode = LT_PT_FLAGS;
			else if( strcmp(typeString,"longint") == 0 )
				propCode = LT_PT_LONGINT;
		}
		else
		{
			//search the upper half of the strings
			if( strcmp(typeString,"real") == 0 )
				propCode = LT_PT_REAL;
			else if( strcmp(typeString,"rotation") == 0 )
				propCode = LT_PT_ROTATION;
			else if( strcmp(typeString,"string") == 0 )
				propCode = LT_PT_STRING;
			else if( strcmp(typeString,"vector") == 0 )
				propCode = LT_PT_VECTOR;
		}

		strncpy(propName, GetString(pPropNode->GetElement(1)), 255);

		if( (pProp = CreatePropFromCode(propCode, propName)) )
		{
			pProp->m_PropFlags = 0;
			CLTANode* pFlagNode = pPropNode->GetElement(2);

			const uint32 listSize = pFlagNode->GetNumElements();
			for( uint32 numFlags = 0; numFlags < listSize; numFlags++ )
			{
				const char* flagsString = pFlagNode->GetElement(numFlags)->GetValue();
				
				//again do a binary split, see comments above
				if(flagsString[0] < 'g')
				{
					if( strcmp(flagsString, "beziernexttangent") == 0 )	
						pProp->m_PropFlags |= PF_BEZIERNEXTTANGENT;
					else if( strcmp(flagsString, "bezierprevtangent") == 0 )	
						pProp->m_PropFlags |= PF_BEZIERPREVTANGENT;
					else if( strcmp(flagsString, "dims") == 0 )					
						pProp->m_PropFlags |= PF_DIMS;
					else if( strcmp(flagsString, "dynamiclist") == 0 )			
						pProp->m_PropFlags |= PF_DYNAMICLIST;
					else if( strcmp(flagsString, "event") == 0 )
						pProp->m_PropFlags |= PF_EVENT;
					else if( strcmp(flagsString, "fieldofview") == 0 )			
						pProp->m_PropFlags |= PF_FIELDOFVIEW;
					else if( strcmp(flagsString, "filename") == 0 )				
						pProp->m_PropFlags |= PF_FILENAME;
					else if( strcmp(flagsString, "fovradius") == 0 )			
						pProp->m_PropFlags |= PF_FOVRADIUS;
					else if( strcmp(flagsString, "compositetype") == 0 )
						pProp->m_PropFlags |= PF_COMPOSITETYPE;
					else if( strcmp(flagsString, "distance") == 0 )
						pProp->m_PropFlags |= PF_DISTANCE;
				}
				else
				{
					if(strncmp(flagsString, "group", 5) == 0)
					{
						if( strcmp(flagsString, "groupowner") == 0 )	
							pProp->m_PropFlags |= PF_GROUPOWNER;
						else
							pProp->m_PropFlags |= PF_GROUP(atoi(flagsString + 5));
					}
					else if( strcmp(flagsString, "hidden") == 0)				
						pProp->m_PropFlags |= PF_HIDDEN;
					else if( strcmp(flagsString, "localdims") == 0 )			
						pProp->m_PropFlags |= PF_LOCALDIMS;
					else if( strcmp(flagsString, "model") == 0 )			
						pProp->m_PropFlags |= PF_MODEL;
					else if( strcmp(flagsString, "notifychange" ) == 0 )
						pProp->m_PropFlags |= PF_NOTIFYCHANGE;
					else if( strcmp(flagsString, "objectlink") == 0 )			
						pProp->m_PropFlags |= PF_OBJECTLINK;
					else if( strcmp(flagsString, "orthofrustum") == 0 )				
						pProp->m_PropFlags |= PF_ORTHOFRUSTUM;
					else if( strcmp(flagsString, "radius") == 0 )				
						pProp->m_PropFlags |= PF_RADIUS;
					else if( strcmp(flagsString, "staticlist") == 0 )			
						pProp->m_PropFlags |= PF_STATICLIST;
					else if( strcmp(flagsString, "textureeffect") == 0 )				
						pProp->m_PropFlags |= PF_TEXTUREEFFECT;
				}
			}
			
			pProp->LoadDataLTA( pPropNode );
			// pNode->m_PropList.m_Props.Append( pProp );
			pDestPropList->m_Props.Append(pProp);
		}
	}
	container.AddPropList( pDestPropList, true );
}

void LoadPropListTBW(CAbstractIO& InFile, CPropListContainer& container )
{
	static char	propName[256];
	CPropList*	pDestPropList = new CPropList;

	//read in the number of properties
	uint32 nNumProps;
	InFile >> nNumProps;

	for(uint32 k=0; k < nNumProps; k++ )
	{
		uint8 nPropCode;
		InFile >> nPropCode;

		InFile.ReadString(propName, sizeof(propName));

		CBaseProp* pProp = CreatePropFromCode(nPropCode, propName);

		if( pProp )
		{
			uint32 nPropFlags;
			InFile >> nPropFlags;
			pProp->m_PropFlags = nPropFlags;
			
			pProp->LoadData( InFile );

			pDestPropList->m_Props.Append(pProp);
		}
	}
	container.AddPropList( pDestPropList, true );
}

static void UpdatePrefabLocationsV1ToV2(CWorldNode* pNode)
{
	//bail if not a valid node
	if(!pNode)
		return;

	//update if it is a prefab
	if(pNode->GetType() == Node_PrefabRef)
	{
		//we need to get the orientation to rotate the offset by
		LTMatrix mObjTrans;
		gr_SetupMatrixEuler(pNode->GetOr(), mObjTrans.m);

		CPrefabRef* pRef = (CPrefabRef*)pNode;
		pNode->SetPos(pNode->GetPos() - mObjTrans * pRef->GetPrefabCenter());
	}

	//recurse on the children
	GPOS ChildPos = pNode->m_Children;
	while(ChildPos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(ChildPos);
		UpdatePrefabLocationsV1ToV2(pChild);
	}
}

//called after a level is loaded and set up to update any version incompatibilities.
bool CEditRegion::PostLoadUpdateVersion(uint32 nVersion, bool& bModified)
{
	uint32 nFinalVersion = nVersion;

	if(nFinalVersion == 1)
	{
		//the difference between 1 and 2 is the prefab locations were offset
		UpdatePrefabLocationsV1ToV2(GetRootNode());
		nFinalVersion = 2;
	}

	bModified = (nFinalVersion != nVersion);
	return (nFinalVersion == REGION_LTA_VERSION) ? TRUE : FALSE;
}

RegionLoadStatus CEditRegion::LoadFile( const char* filename, CEditProjectMgr* pProject, uint32& nVersion, bool& bBinary)
{
	//get the extension from the filename
	const char* pszExtension = filename;

	const char* pszCurr = filename;

	while(*pszCurr)
	{
		if(*pszCurr == '.')
			pszExtension = pszCurr;

		pszCurr++;
	}

	//alright, now classify the file
	if(stricmp(pszExtension, ".tbw") == 0)
	{
		bBinary = true;
		return LoadTBW(filename, pProject, nVersion);
	}
	else
	{
		bBinary = false;
		return LoadLTA(filename, pProject, nVersion);
	}
}

RegionLoadStatus CEditRegion::LoadLTA(const char* filename, CEditProjectMgr* pProject, uint32& nFileVersion )
{
	RegionLoadStatus	status = REGIONLOAD_INVALIDFILE;
	Term();

	m_LoadErrorCode = -1;

	//default the version to version 1
	nFileVersion = 1;

	CLTAReader Reader;
	CLTALoadOnlyAlloc Allocator(1024 * 512);

	if(Reader.Open(filename, CLTAUtil::IsFileCompressed(filename))) 
	{
		//------------------------------
		// let's start with the header
		CLTANode* pHeader = CLTANodeReader::LoadNode(&Reader, "header", &Allocator);
		
		if(pHeader)
		{
			CLTANodeIterator NodeIter(pHeader);

			CLTANode* pInfoString = NodeIter.FindNextList("infostring");

			m_pInfoString = m_pStringHolder->AddString((char*)
								PairCdr(pInfoString));

			//now load in the file version
			CLTANodeIterator VersionIter(pHeader);

			CLTANode* pVersionString = VersionIter.FindNextList("versioncode");

			if(pVersionString && pVersionString->IsList()  && (pVersionString->GetNumElements() >= 2))
			{
				nFileVersion = atoi(PairCdr(pVersionString));			
			}

		}

		//clean up that block
		pHeader = NULL;
		Allocator.FreeAllMemory();

		//--------------------------------
		//Load in the polyhedron list

		CLTANode* pPolyhedronList = CLTANodeReader::LoadNode(&Reader, "polyhedronlist", &Allocator);

		if(pPolyhedronList)
		{
			CLTANode* pPolyhedronListHead = pPolyhedronList->GetElement(1);

			//start out at 1 to skip over the name element
			for(uint32 nCurrPoly = 0; nCurrPoly < pPolyhedronListHead->GetNumElements(); nCurrPoly++)
			{
				CLTANode* pPolyhedron = pPolyhedronListHead->GetElement(nCurrPoly);
			
				//see if there are any more polyhedrons
				ASSERT(pPolyhedron);

				CEditBrush *pBrush = new CEditBrush;
				// m_QuickBrushIndexer[i] = pBrush;
				m_QuickBrushIndexer.Push(pBrush);
				AddBrush(pBrush);
				
				if( !pBrush->LoadLTA(pPolyhedron->GetElement(1), m_pStringHolder) )
				{
					m_LoadErrorCode = 1;
					break;
				}

				//go ahead and clean up this poly, cut down on memory usage
				//so it will hopefully remain faily constant
				//pPolyhedron->Free();
			}
			
			//clean up the list of polyhedrons
			pPolyhedronList		= NULL;
			pPolyhedronListHead = NULL;
		}
		else
		{
			//missed it, need to start over at the beginning of the file
			Reader.Close();
			Reader.Open(filename, CLTAUtil::IsFileCompressed(filename));
		}

		//clean up
		pPolyhedronList = NULL;
		Allocator.FreeAllMemory();


		//--------------------------------
		// let's do the globablproplist

		{
			g_loadPropListContainer.Init();

			//get the main list of global properties
			CLTANode* pPropList = CLTANodeReader::LoadNode(&Reader, "globalproplist", &Allocator);

			//make sure we found it
			if(pPropList)
			{
				//okay, not point to the head of the list we will use
				CLTANode* pPropListHead = pPropList->GetElement(1);
				ASSERT(pPropListHead);

				//now run through and load in every property

				for(uint32 nCurrProp = 0; nCurrProp < pPropListHead->GetNumElements(); nCurrProp++)
				{		
					CLTANode* pProp = pPropListHead->GetElement(nCurrProp);

					//see if we hit the end
					if(pProp == NULL)
					{
						break;
					}

					LoadPropList(pProp->GetElement(1), g_loadPropListContainer );

					//unload the associated memory
					//pProp->Free();
				}

				//clear out the global lists
				pPropList		= NULL;
				pPropListHead	= NULL;
			}

			pPropList = NULL;
			Allocator.FreeAllMemory();

		}


		//-------------------------------
		// let's do the nodehierarchy
		
		//reset the file
		Reader.Close();
		Reader.Open(filename, CLTAUtil::IsFileCompressed(filename));

		CLTANode* pNodehierarchy = CLTANodeReader::LoadNode(&Reader, "nodehierarchy", &Allocator);
		if( pNodehierarchy )
		{
			CLTANodeIterator NodeIter(pNodehierarchy);

			if( !RecurseAndLoadNodeLTA(	NodeIter.FindNextList("worldnode" ),
										&m_RootNode, 
										pProject) )
			{
				Term();
				return REGIONLOAD_INVALIDFILE;
			}

			pNodehierarchy = NULL;
		}
		else
		{
			//missed it, need to reset the file
			Reader.Close();
			Reader.Open(filename, CLTAUtil::IsFileCompressed(filename));
		}

		g_loadPropListContainer.UnloadFromMemory();

		//clean up the nodes
		pNodehierarchy = NULL;
		Allocator.FreeAllMemory();



		//----------------------------------
		// Load the navigator positions
		
		CLTANode* pNavigatorposList = CLTANodeReader::LoadNode(&Reader, "navigatorposlist", &Allocator);
		if( pNavigatorposList )
		{
			m_NavigatorPosArray.LoadLTA( PairCdrNode(pNavigatorposList) );
			pNavigatorposList = NULL;
		}

		//clean up the nodes
		pNavigatorposList = NULL;
		Allocator.FreeAllMemory();

		m_RootNode.m_Flags |= NODEFLAG_WORLDROOT;
		// Update stuff.
		status = REGIONLOAD_OK;
	}
	
	Reader.Close();
	
	m_QuickBrushIndexer.Term();	

	return status;
}

void CEditRegion::SaveTBW( CAbstractIO& OutFile )
{
	//give brushes indices
	uint32 i=0;
	for(LPOS pos=m_Brushes; pos; )
	{
		m_Brushes.GetNext(pos)->m_RegionBrushIndex = i++;
	}


	//------------------------------
	// let's start with the header
	//write the version
	uint32 nVersion = CURRENT_TBW_VERSION;
	OutFile << nVersion;

	//setup markers for the different sections
	uint32 nPolyhedronMarker	= PlaceMarker(OutFile);
	uint32 nPropertyMarker		= PlaceMarker(OutFile);
	uint32 nNodeMarker			= PlaceMarker(OutFile);
	uint32 nNavigatorMarker		= PlaceMarker(OutFile);

	//now write out the info string
	OutFile.WriteString(m_pInfoString);

	//--------------------------------
	//Load in the polyhedron list

	{
		UpdateMarker(OutFile, nPolyhedronMarker);

		g_propListContainer.Init();

		//write the number of brushes that we have
		uint32 nNumBrushes = m_Brushes.GetSize();
		OutFile << nNumBrushes;

		for(LPOS pos = m_Brushes; pos; )
			m_Brushes.GetNext(pos)->SaveTBW(OutFile);
	}

	//-------------------------------
	// let's do the nodehierarchy
	{
		UpdateMarker(OutFile, nNodeMarker);

		RecurseAndSaveNodeTBW(OutFile, &m_RootNode);
	}

	//-------------------------------
	// the property lists
	{
		UpdateMarker(OutFile, nPropertyMarker);

		g_propListContainer.SaveTBW(OutFile);
	}

	//----------------------------------
	// Save the navigator positions
	{
		UpdateMarker(OutFile, nNavigatorMarker);
		m_NavigatorPosArray.SaveTBW( OutFile );
	}
}		


RegionLoadStatus CEditRegion::LoadTBW(const char* filename, CEditProjectMgr* pProject, uint32& nFileVersion )
{
	RegionLoadStatus	status = REGIONLOAD_INVALIDFILE;
	Term();

	m_LoadErrorCode = -1;

	//default the version to version 1
	nFileVersion = 1;

	//open up the file for reading
	CMoFileIO InFile;
	if(!InFile.Open(filename, "rb"))
	{
		return REGIONLOAD_INVALIDFILEVERSION;
	}

	//------------------------------
	// let's start with the header
	//read in the version
	InFile >> nFileVersion;

	//verify that the version matches
	if(nFileVersion != CURRENT_TBW_VERSION)
	{
		InFile.Close();
		return REGIONLOAD_INVALIDFILEVERSION;
	}

	//read in marker positions
	uint32 nPolyhedronMarker;
	uint32 nPropertyMarker;
	uint32 nNodeMarker;
	uint32 nNavigatorMarker;

	InFile >> nPolyhedronMarker;
	InFile >> nPropertyMarker;
	InFile >> nNodeMarker;
	InFile >> nNavigatorMarker;

	//now read in the info string
	char pszTempInfoString[512];
	InFile.ReadString(pszTempInfoString, sizeof(pszTempInfoString));
	m_pInfoString = m_pStringHolder->AddString(pszTempInfoString);

	//--------------------------------
	//Load in the polyhedron list

	InFile.SeekTo(nPolyhedronMarker);
	{
		//read in the number of brushes that we have
		uint32 nNumBrushes = 0;
		InFile >> nNumBrushes;

		//start out at 1 to skip over the name element
		for(uint32 nCurrPoly = 0; nCurrPoly < nNumBrushes; nCurrPoly++)
		{
			CEditBrush *pBrush = new CEditBrush;
			m_QuickBrushIndexer.Push(pBrush);
			AddBrush(pBrush);
			
			if( !pBrush->LoadTBW(InFile, m_pStringHolder) )
			{
				m_LoadErrorCode = 1;
				break;
			}
		}
	}

	//--------------------------------
	// let's do the globablproplist

	InFile.SeekTo(nPropertyMarker);
	{
		g_loadPropListContainer.Init();

		uint32 nNumProperties;
		InFile >> nNumProperties;

		//now run through and load in every property
		for(uint32 nCurrProp = 0; nCurrProp < nNumProperties; nCurrProp++)
		{		
			LoadPropListTBW(InFile, g_loadPropListContainer );
		}
	}

	//-------------------------------
	// let's do the nodehierarchy
	InFile.SeekTo(nNodeMarker);
	{
		if( !RecurseAndLoadNodeTBW(	InFile, &m_RootNode, pProject) )
		{
			Term();
			return REGIONLOAD_INVALIDFILE;
		}
	}

	g_loadPropListContainer.UnloadFromMemory();

	//----------------------------------
	// Load the navigator positions
	InFile.SeekTo(nNavigatorMarker);
	{
		m_NavigatorPosArray.LoadTBW(InFile);
	}

	//setup the root node so it can't be removed
	m_RootNode.m_Flags |= NODEFLAG_WORLDROOT;

	// Update stuff.
	status = REGIONLOAD_OK;
	
	m_QuickBrushIndexer.Term();	

	return status;
}

void CEditRegion::SaveHeaderLTA(CLTAFile* pFile, uint32 level, uint32 versionCode)
{
	PrependTabs(pFile, level );
	pFile->WriteStr("( header (");
		PrependTabs(pFile, level+1 );
		pFile->WriteStrF("( versioncode %d )", versionCode );
		if( strlen(m_pInfoString) )
		{
			PrependTabs(pFile, level+1 );
			pFile->WriteStrF("( infostring \"%s\" )", m_pInfoString );
		}
	PrependTabs(pFile, level );
	pFile->WriteStr(") )");
}

void CEditRegion::SaveLTA(CLTAFile* pFile )
{
	uint32			i;
	LPOS pos;
	
	// uint32 versionCode=g_LTAVersion;
	
	pFile->WriteStr("\n( world" );
	
	SaveHeaderLTA(pFile, 1, REGION_LTA_VERSION );
	
	// Give the brushes indices.
	i=0;
	for(pos=m_Brushes; pos; )
	{
		m_Brushes.GetNext(pos)->m_RegionBrushIndex = i++;
	}

	PrependTabs(pFile, 1);
	pFile->WriteStr("( polyhedronlist (" );
		for(pos=m_Brushes; pos; )
		   m_Brushes.GetNext(pos)->SaveLTA(pFile, 2);
	PrependTabs(pFile, 1);
	pFile->WriteStr(") )" );

	PrependTabs(pFile, 1);
	g_propListContainer.Init();
	pFile->WriteStr("( nodehierarchy " );
		RecurseAndSaveNodeLTA( pFile, &m_RootNode,2);
	PrependTabs(pFile, 1);

	pFile->WriteStr(")" );

	g_propListContainer.SaveLTA( pFile, 1 );
	// g_propListContainer.UnloadFromMemory();

	// Save the navigator positions	
	m_NavigatorPosArray.SaveLTA(pFile, 1);

	pFile->WriteStr("\n)");
}		

BOOL LoadNodePropertiesLTA(CLTANode* pParseNode,		CWorldNode *pNode, 
						   CPropListContainer& container, CEditProjectMgr* pProject	 )
{
	pNode->SetClassName(PairCdr( CLTAUtil::ShallowFindList(pParseNode, "name" ) ));
	uint32 propID = GetUint32(PairCdrNode(CLTAUtil::ShallowFindList(pParseNode, "propid")));

	CPropList* propList = container.GetElem(propID);

	for( uint32 i = 0; i < propList->m_Props.GetSize(); i++ )
	{

		CBaseProp *pProp = CreatePropFromCode(propList->m_Props[i]->m_Type, propList->m_Props[i]->m_Name);
		if( pProp )
		{
			pProp->m_PropFlags = propList->m_Props[i]->m_PropFlags;
			pProp->Copy(propList->m_Props[i]);
			
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
							pProp->m_pPropDef = pProject->FindPropInList(*pPropList, pProp->m_Name);
						}
					}
				}
			#endif
				pNode->m_PropList.m_Props.Append( pProp );	
			
			#ifdef DIRECTEDITOR_BUILD
				//we need to give the object a chance to react to the change but don't send the chage to the game code...
				pNode->OnPropertyChanged(pProp, false, NULL);
			#endif
		}
		

	}

	return TRUE;
}

BOOL CEditRegion::RecurseAndLoadNodeLTA( CLTANode* pParseNode, CWorldNode *pNode, CEditProjectMgr* pProject )
{
	CWorldNode *pChild;
	CLTANode* pChildListNode = PairCdrNode(CLTAUtil::ShallowFindList(pParseNode, "childlist") );
	uint32 nChildren = pChildListNode ? pChildListNode->GetNumElements() : 0;

	for( uint32 i=0; i < nChildren; i++ )
	{
		//the current child
		CLTANode* pCurrChild = pChildListNode->GetElement(i);

		const char* pNodeType = PairCdr(pCurrChild->GetElement(1));//PairCdr(CLTAUtil::ShallowFindList(pChildListNode->GetElement(i), "type" ) );
	
		if( strcmp(pNodeType, "null" ) == 0 )
		{
			pChild = new CWorldNode;
		}
		else if( strcmp(pNodeType, "brush" ) == 0 )
		{
			uint32 index = GetUint32(PairCdrNode(pCurrChild->GetElement(2)));//GetUint32(PairCdrNode(CLTAUtil::ShallowFindList(pChildListNode->GetElement(i), "brushindex" ) ));
			ASSERT( index != BAD_INDEX );
			if(index >= m_Brushes.GetSize())
			{
				m_LoadErrorCode = 3;
				return FALSE;
			}
			pChild = m_QuickBrushIndexer[index];
		}
		else if( strcmp(pNodeType, "object" ) == 0 )
		{
			pChild = new CBaseEditObj;
		}
		else if( strcmp(pNodeType, "prefabref" ) == 0 )
		{
			const char *pFilename = PairCdrNode(pCurrChild->GetElement(2))->GetValue(); // prefabfile
			const char *pName = PairCdrNode(pCurrChild->GetElement(3))->GetValue(); // prefabname

			pChild = GetPrefabMgr()->CreateUnboundRef(this, pNode, pFilename, pName);

			//read in the position
			CLTANode* pPosNode = pCurrChild->GetElement(4);
			if(pPosNode->GetNumElements() == 4)
			{
				LTVector vPos;
				vPos.x = (float)atof(pPosNode->GetElement(1)->GetValue());
				vPos.y = (float)atof(pPosNode->GetElement(2)->GetValue());
				vPos.z = (float)atof(pPosNode->GetElement(3)->GetValue());
				pChild->SetPos(vPos);
			}

			//read in the orientations
			CLTANode* pOrNode = pCurrChild->GetElement(5);
			if(pOrNode->GetNumElements() == 4)
			{
				LTVector vOr;
				vOr.x = (float)atof(pOrNode->GetElement(1)->GetValue());
				vOr.y = (float)atof(pOrNode->GetElement(2)->GetValue());
				vOr.z = (float)atof(pOrNode->GetElement(3)->GetValue());
				pChild->SetOr(vOr);
			}
			
			if (!pChild)
			{
				m_LoadErrorCode = 6;
				return FALSE;
			}
		}
		else if (strcmp(pNodeType, "patch") == 0)
		{
			//This is to support the old patches. We won't do anything, just ignore it
		}
		else if( strcmp(pNodeType, "prefabref" ) == 0 )
		{
			const char *pFilename = PairCdrNode(pCurrChild->GetElement(2))->GetValue(); // prefabfile
			const char *pName = PairCdrNode(pCurrChild->GetElement(3))->GetValue(); // prefabname

			pChild = GetPrefabMgr()->CreateUnboundRef(this, pNode, pFilename, pName);

			//read in the position
			CLTANode* pPosNode = pCurrChild->GetElement(4);
			if(pPosNode->GetNumElements() == 4)
			{
				LTVector vPos;
				vPos.x = (float)atof(pPosNode->GetElement(1)->GetValue());
				vPos.y = (float)atof(pPosNode->GetElement(2)->GetValue());
				vPos.z = (float)atof(pPosNode->GetElement(3)->GetValue());
				pChild->SetPos(vPos);
			}

			//read in the orientations
			CLTANode* pOrNode = pCurrChild->GetElement(5);
			if(pOrNode->GetNumElements() == 4)
			{
				LTVector vOr;
				vOr.x = (float)atof(pOrNode->GetElement(1)->GetValue());
				vOr.y = (float)atof(pOrNode->GetElement(2)->GetValue());
				vOr.z = (float)atof(pOrNode->GetElement(3)->GetValue());
				pChild->SetOr(vOr);
			}
			
			if (!pChild)
			{
				m_LoadErrorCode = 6;
				return FALSE;
			}
		}
		else
		{
			m_LoadErrorCode = 4;
			return FALSE;
		}

		// Note : PrefabRef objects are created pre-attached
		if( pChild->GetType() != Node_PrefabRef)
		{
			pNode->m_Children.Append( pChild );
			pChild->SetParent( pNode );
		}

		RecurseAndLoadNodeLTA( pCurrChild, pChild, pProject);
		//! new
		// delete pChildListNode->GetElement(i);
		// pChildListNode->GetElement(i) = 0;
		// pChildListNode->GetElement(i)->deleteChildren();
	}

	
	if(!LoadNodePropertiesLTA(CLTAUtil::ShallowFindList(pParseNode, "properties" ), pNode, 
								g_loadPropListContainer, pProject ))
	{
		m_LoadErrorCode = 5;
		return FALSE;
	}

	//now if this is an object, we need to add this object (it needs to wait until down
	//here because it uses the class name)
	if(pNode->GetType() == Node_Object)
	{
		AddObject(pNode->AsObject());
	}
	
	CLTANode* pFlagNode = PairCdrNode(CLTAUtil::ShallowFindList(pParseNode, "flags"));
	if( pFlagNode )
	{
		uint32 listSize = pFlagNode->GetNumElements();
		for( uint32 flagIndex = 0; flagIndex < listSize; flagIndex++ )
		{
			const char* pFlagString = pFlagNode->GetElement(flagIndex)->GetValue();
			if( strcmp(pFlagString,"selected") == 0 )
				pNode->m_Flags |= NODEFLAG_SELECTED;
			else if( strcmp(pFlagString,"showmodel") == 0 )
				pNode->m_Flags |= NODEFLAG_SHOWMODEL;
			else if( strcmp(pFlagString,"hidden") == 0 )
				pNode->m_Flags |= NODEFLAG_HIDDEN;
			else if( strcmp(pFlagString,"frozen") == 0 )
				pNode->m_Flags |= NODEFLAG_FROZEN;
			else if( strcmp(pFlagString,"path") == 0 )
				pNode->m_Flags |= NODEFLAG_PATH;
			else if( strcmp(pFlagString,"worldroot") == 0 )
				pNode->m_Flags |= NODEFLAG_WORLDROOT;
			else if( strcmp(pFlagString,"expanded") == 0 )
				pNode->m_Flags |= NODEFLAG_EXPANDED;
		}
	}

	if (pNode->GetType() == Node_Null)
	{
		CLTANode* pLabelName = PairCdrNode(CLTAUtil::ShallowFindList(pParseNode, "label"));
		if(pLabelName)
		{

			pNode->SetNodeLabel(GetString(pLabelName));
		}
	}
	
	if(pNode->IsFlagSet(NODEFLAG_SELECTED))
		SelectNode( pNode );
     
	if(pNode->IsFlagSet(NODEFLAG_PATH))
		AddNodeToPath(pNode);

	return TRUE;		
}


void CEditRegion::RecurseAndSaveNodeLTA( CLTAFile* pFile, CWorldNode *pNode, uint32 level )
{
	CWorldNode	*pChild;
	GPOS pos;

	PrependTabs(pFile, level );
	pFile->WriteStr("( worldnode ");

		PrependTabs(pFile, level+1);

		pFile->WriteStr("( type");
		if( pNode->m_Type == Node_Null )
			pFile->WriteStr(" null ");
		else if(pNode->m_Type == Node_Brush)
			pFile->WriteStr(" brush ");
		else if(pNode->m_Type == Node_Object)
			pFile->WriteStr(" object ");
		else if(pNode->m_Type == Node_PrefabRef)
			pFile->WriteStrF(" prefabref ");
		else
			pFile->WriteStr(" invalid ");
		pFile->WriteStr(")");
			
		if( pNode->m_Type == Node_Brush )
		{
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( brushindex %d )", pNode->AsBrush()->m_RegionBrushIndex);
		}
		else if( pNode->m_Type == Node_PrefabRef )
		{
			CPrefabRef *pPrefab = (CPrefabRef*)pNode;
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( prefabfile \"%s\" )", pPrefab->GetPrefabFilename());
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( prefabname \"%s\" )", pPrefab->GetName());
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( position %f %f %f )", pPrefab->GetPos().x, pPrefab->GetPos().y, pPrefab->GetPos().z);
			PrependTabs(pFile, level+1);
			pFile->WriteStrF("( orientation %f %f %f )", pPrefab->GetOr().x, pPrefab->GetOr().y, pPrefab->GetOr().z);
		}

		const char *lpszLabel=pNode->GetNodeLabel();
		if( lpszLabel && strlen(lpszLabel) )
		{
			PrependTabs( pFile, level + 1 );
			pFile->WriteStrF("( label \"%s\" ) ", (char *)lpszLabel );
		}

		PrependTabs(pFile, level + 1);
		pFile->WriteStrF("( nodeid %d ) ", pNode->GetUniqueID() );
		PrependTabs(pFile, level + 1);
		pFile->WriteStr("( flags ( " );
		if( pNode->IsFlagSet(NODEFLAG_SELECTED) )
			pFile->WriteStr("selected " );
		if( pNode->IsFlagSet(NODEFLAG_HIDDEN) )
			pFile->WriteStr("hidden " );
		if( pNode->IsFlagSet(NODEFLAG_SHOWMODEL) )
			pFile->WriteStr("showmodel ");
		if( pNode->IsFlagSet(NODEFLAG_FROZEN) )
			pFile->WriteStr("frozen " );
		if( pNode->IsFlagSet(NODEFLAG_PATH) )
			pFile->WriteStr("path " );
		if( pNode->IsFlagSet(NODEFLAG_WORLDROOT) )
			pFile->WriteStr("worldroot " );
		if( pNode->IsFlagSet(NODEFLAG_EXPANDED) )
			pFile->WriteStr("expanded " );
		pFile->WriteStr(") ) ");
		
		SaveNodePropertiesLTA(pFile, pNode, level+1);

		if( pNode->m_Children.GetSize() )
		{
			PrependTabs(pFile, level + 1);
			pFile->WriteStr("( childlist ( " );
				for(pos=pNode->m_Children; pos; )
				{
					pChild = pNode->m_Children.GetNext(pos);
					RecurseAndSaveNodeLTA(pFile, pChild, level+2);
				}
			PrependTabs(pFile, level + 1);
			pFile->WriteStr(") )" );
		}
	PrependTabs(pFile, level);
	pFile->WriteStr(")");
}

bool LoadNodePropertiesTBW(CAbstractIO& InFile,	CWorldNode *pNode, CPropListContainer& container, CEditProjectMgr* pProject)
{
	//read in the class name
	char pszClassName[512];
	InFile.ReadString(pszClassName, sizeof(pszClassName));
	pNode->SetClassName(pszClassName);

	//read in the prop list identifier
	uint32 nPropID;
	InFile >> nPropID;

	CPropList* propList = container.GetElem(nPropID);

	for( uint32 i = 0; i < propList->m_Props.GetSize(); i++ )
	{
		CBaseProp *pProp = CreatePropFromCode(propList->m_Props[i]->m_Type, propList->m_Props[i]->m_Name);
		if( pProp )
		{
			pProp->m_PropFlags = propList->m_Props[i]->m_PropFlags;
			pProp->Copy(propList->m_Props[i]);
			
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
							pProp->m_pPropDef = pProject->FindPropInList(*pPropList, pProp->m_Name);
						}
					}
				}
			#endif
				pNode->m_PropList.m_Props.Append( pProp );	
			
			#ifdef DIRECTEDITOR_BUILD
				//we need to give the object a chance to react to the change but don't send the chage to the game code...
				pNode->OnPropertyChanged(pProp, false, NULL);
			#endif
		}
		

	}

	//and now load our filtered properties on top of the other properties
	static const char* pszFilters[] = { "Name", "Pos", "Or" };
	static const uint32 knNumFilters = sizeof(pszFilters) / sizeof(pszFilters[0]);

	//and now write out our actual property data
	for(uint32 nCurrProp = 0; nCurrProp < knNumFilters; nCurrProp++)
	{
		CBaseProp* pProp = pNode->GetPropertyList()->GetProp(pszFilters[nCurrProp]);

		//read in the success
		uint8 nSuccess;
		InFile >> nSuccess;

		if(nSuccess)
		{
			pProp->LoadData(InFile);
		}
	}

	return true;
}

bool CEditRegion::RecurseAndLoadNodeTBW( CAbstractIO& InFile, CWorldNode *pNode, CEditProjectMgr* pProject )
{
	//load in the data for the node
	if(pNode->GetType() == Node_Null)
	{
		char pszLabel[512];
		InFile.ReadString(pszLabel, sizeof(pszLabel));

		pNode->SetNodeLabel(pszLabel);
	}

	//read in the node flags
	InFile >> pNode->m_Flags;

	//read in the property list
	//LoadNodeProperties(InFile, pNode, pProject);

	if(!LoadNodePropertiesTBW(InFile, pNode, g_loadPropListContainer, pProject ))
	{
		m_LoadErrorCode = 5;
		return false;
	}
	
	//now if this is an object, we need to add this object (it needs to wait until down
	//here because it uses the class name)
	if(pNode->GetType() == Node_Object)
	{
		AddObject(pNode->AsObject());
	}

	//perform some setup on this node	
	if(pNode->IsFlagSet(NODEFLAG_SELECTED))
		SelectNode( pNode );
 
	if(pNode->IsFlagSet(NODEFLAG_PATH))
		AddNodeToPath(pNode);

	//recurse into the children
	uint32 nNumChildren;
	InFile >> nNumChildren;

	for(uint32 nCurrChild = 0; nCurrChild < nNumChildren; nCurrChild++)
	{
		//get the type and allocate this node
		uint32 nChildType;
		InFile >> nChildType;

		//the child that we will be allocating
		CWorldNode* pChild = NULL;

		//create the node
		if(nChildType == Node_Brush)
		{
			//read in the brush index
			uint32 nBrushIndex;
			InFile >> nBrushIndex;

			if(nBrushIndex >= m_Brushes.GetSize())
			{
				m_LoadErrorCode = 3;
				return FALSE;
			}
			pChild = m_QuickBrushIndexer[nBrushIndex];
		}
		else if(nChildType == Node_PrefabRef)
		{
			//for a prefab, write out the file
			char pszFileBuff[_MAX_PATH];
			InFile.ReadString(pszFileBuff, sizeof(pszFileBuff));

			//the name
			char pszNameBuff[512];
			InFile.ReadString(pszNameBuff, sizeof(pszNameBuff));

			pChild = GetPrefabMgr()->CreateUnboundRef(this, pNode, pszFileBuff, pszNameBuff);

			//now the position
			LTVector vPos;
			InFile >> vPos.x >> vPos.y >> vPos.z;

			LTVector vOr;
			InFile >> vOr.x >> vOr.y >> vOr.z;

			pChild->SetPos(vPos);
			pChild->SetOr(vOr);
		}
		else if(nChildType == Node_Object)
		{
			pChild = new CBaseEditObj;
		}
		else if(nChildType == Node_Null)
		{
			pChild = new CWorldNode;
		}
		else
		{
			m_LoadErrorCode = 4;
			return false;
		}
		
		// Note : PrefabRef objects are created pre-attached
		if( pChild->GetType() != Node_PrefabRef)
		{
			pNode->m_Children.Append( pChild );
			pChild->SetParent( pNode );
		}

		//and recurse to load the children
		RecurseAndLoadNodeTBW(InFile, pChild, pProject);
		
	}

	return true;		
}

void SaveNodePropertiesTBW(CAbstractIO& OutFile, CWorldNode *pObj)
{
	//write out the class name
	OutFile.WriteString(pObj->GetClassName());

	static const char* pszFilters[] = { "Name", "Pos", "Or" };
	static const uint32 knNumFilters = sizeof(pszFilters) / sizeof(pszFilters[0]);

	//save the ID of the property list
	uint32 nPropID = g_propListContainer.AddPropList(&(pObj->m_PropList), knNumFilters, pszFilters);

	OutFile << nPropID;

	//and now write out our actual property data
	for(uint32 nCurrProp = 0; nCurrProp < knNumFilters; nCurrProp++)
	{
		CBaseProp* pProp = pObj->GetPropertyList()->GetProp(pszFilters[nCurrProp]);

		if(pProp)
		{
			//we have a property, save it out
			OutFile << (uint8)1;
			pProp->SaveData(OutFile);
		}
		else
		{
			//failed to find the prop, indicate this
			OutFile << (uint8)0;
		}
	}
}

void CEditRegion::RecurseAndSaveNodeTBW( CAbstractIO& OutFile, CWorldNode *pNode)
{
	//write out the type....unless this is the root node, of which we always assume is there
	//and we don't need to write the type, we just assume it is a null node
	if(!(pNode->GetFlags() & NODEFLAG_WORLDROOT))
	{
		OutFile << pNode->m_Type;
	}
			
	if( pNode->m_Type == Node_Brush )
	{
		//for brushes write out the index
		uint32 nBrushIndex = pNode->AsBrush()->m_RegionBrushIndex;
		OutFile << nBrushIndex;
	}
	else if( pNode->m_Type == Node_PrefabRef )
	{
		CPrefabRef* pPrefab = (CPrefabRef*)pNode;

		//for a prefab, write out the file
		OutFile.WriteString(pPrefab->GetPrefabFilename());

		//the name
		OutFile.WriteString(pPrefab->GetName());

		//the position
		LTVector vPos = pPrefab->GetPos();
		OutFile << vPos.x << vPos.y << vPos.z;

		//the orientation
		LTVector vOr = pPrefab->GetOr();
		OutFile << vOr.x << vOr.y << vOr.z;
	}
	else if(pNode->m_Type == Node_Null)
	{
		//write out the label
		OutFile.WriteString(pNode->GetNodeLabel());
	}

	//the flags
	uint32 nFlags = pNode->GetFlags();
	OutFile << nFlags;
	
	//SaveNodeProperties(OutFile, pNode);
	SaveNodePropertiesTBW(OutFile, pNode);

	//write out the number of children
	uint32 nNumChildren = pNode->m_Children.GetSize();
	OutFile << nNumChildren;

	for(GPOS pos=pNode->m_Children; pos; )
	{
		CWorldNode	*pChild = pNode->m_Children.GetNext(pos);
		RecurseAndSaveNodeTBW(OutFile, pChild);
	}
}


#ifdef DIRECTEDITOR_BUILD

static bool GetTextureDims(const char* pszFilename, uint32& nWidth, uint32& nHeight)
{
	DFileIdent* pFile;

	//get the file identifier
	dfm_GetFileIdentifier( GetFileMgr(), pszFilename, &pFile );

	//get the dimensions of the texture
	CTexture* pTexture = dib_GetDibTexture(pFile);

	if(pTexture)
	{
		nWidth	= pTexture->m_pDib->GetWidth();
		nHeight = pTexture->m_pDib->GetHeight();
		return true;
	}

	return false;
}

uint32 CEditRegion::RenameTexture(const char *pSrcTextureName, const char *pDestTextureName, bool bScaleOPQs)
{
	//we need to determine scales for the P and Q vectors
	float fPScale = 1.0f;
	float fQScale = 1.0f;

	if(bScaleOPQs)
	{
		uint32 nSrcWidth, nSrcHeight, nDestWidth, nDestHeight;
		if(	GetTextureDims(pSrcTextureName, nSrcWidth, nSrcHeight) &&
			GetTextureDims(pDestTextureName, nDestWidth, nDestHeight))
		{
			//we can calculate the scale
			fPScale = (float)nDestWidth / (float)nSrcWidth;
			fQScale = (float)nDestHeight / (float)nSrcHeight;
		}
	}

	//count of how many polygons changed
	uint32 nOccurences = 0;

	for(LPOS lPos=m_Brushes; lPos; )
	{
		CEditBrush *pBrush = m_Brushes.GetNext(lPos);
	
		for(uint32 i=0; i < pBrush->m_Polies; i++)
		{
			CEditPoly *pPoly = pBrush->m_Polies[i];

			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);

				if(Texture.m_pTextureName)
				{
					if(CHelpers::UpperStrcmp(pPoly->GetTexture(nCurrTex).m_pTextureName, pSrcTextureName))
					{
						//we just need to change over the texture
						Texture.m_pTextureName = m_pStringHolder->AddString(pDestTextureName);
						Texture.UpdateTextureID();

						//scale the PQ vectors
						Texture.SetP(pPoly->GetTexture(nCurrTex).GetP() * fPScale);
						Texture.SetQ(pPoly->GetTexture(nCurrTex).GetQ() * fQScale);

						++nOccurences;
					}
				}
			}
		}
	}

	return nOccurences;
}
#endif


CWorldNode* CEditRegion::CheckNodes(CWorldNode *pRoot)
{
	GPOS pos;
	int nChildrenIterated;
	CWorldNode *pNode;
	CBaseProp *pProp;
	uint32 i;

	if(!pRoot)
	{
		// This is just a little test to make sure the brushes point to valid data.
		CMatrix transform;
		LPOS lPos;
		CEditBrush *pBrush;
		uint32 k;

		transform.Identity();
		for(lPos=m_Brushes; lPos; )
		{
			pBrush = m_Brushes.GetNext(lPos);

			for( k=0; k < pBrush->m_Points; k++ )
			{
				CEditVert	&vert = pBrush->m_Points[k];

				transform.Apply4x4( vert, vert.m_Transformed );
			}
		}

		pRoot = GetRootNode();
	}

	// Check the props.
	for(i=0; i < pRoot->m_PropList.m_Props; i++)
	{
		pProp = pRoot->m_PropList.m_Props[i];

		if(pProp->m_Type < 0 || pProp->m_Type >= LT_NUM_PROPERTYTYPES)
			return pRoot;
	}

	nChildrenIterated = 0;
	for(pos=pRoot->m_Children; pos; )
	{
		pNode = CheckNodes(pRoot->m_Children.GetNext(pos));
		if(pNode)
			return pNode;

		++nChildrenIterated;
	}

	if(nChildrenIterated == (int)pRoot->m_Children.GetSize())
		return NULL;
	else
		return pRoot;
}


uint32 CEditRegion::GetTotalNumPolies()
{
	uint32 ret=0;
	LPOS pos;
	
	for(pos=m_Brushes; pos; )
		ret += m_Brushes.GetNext(pos)->m_Polies.GetSize();

	return ret;
}


uint32 CEditRegion::GetTotalNumPoints()
{
	uint32 ret=0;
	LPOS pos;
	
	for(pos=m_Brushes; pos; )
		ret += m_Brushes.GetNext(pos)->m_Points.GetSize();

	return ret;
}


uint32 CEditRegion::NumBrushSelections()
{
	uint32			i;
	uint32			num=0;

	for(i=0; i < m_Selections; i++)
		if(m_Selections[i]->GetType() == Node_Brush)
			++num;

	return num;
}


void CEditRegion::CleanupGeometry(CLinkedList<CEditBrush*> *pBrushes)
{
	FixInvalidBrushes(pBrushes);
	RemoveCollinearVertices(pBrushes);
	RemoveDuplicatePoints(pBrushes);
	FixInaccurateGeometry(pBrushes);
	UpdatePlanes(pBrushes);
}

// Remove invalid point references in polygons and remove invalid polygons
void CEditRegion::FixInvalidBrushes(CLinkedList<CEditBrush*> *pBrushes)
{
	if(!pBrushes)
		pBrushes = &m_Brushes;

	for (LPOS pos = pBrushes->GetHeadPosition(); pos; )
	{
		CEditBrush *pBrush = pBrushes->GetNext(pos);

		for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
		{
			CEditPoly *pPoly = pBrush->m_Polies[nPolyLoop];

			for (uint32 nPointLoop = 0; nPointLoop < pPoly->NumVerts(); ++nPointLoop)
			{
				// Remove invalid points
				if (pPoly->m_Indices[nPointLoop] >= pBrush->m_Points.GetSize())
				{
					pPoly->m_Indices.Remove(nPointLoop);
					--nPointLoop;
					UpdateBrushGeometry(pBrush);
				}
			}

			// If it's an invalid poly now, remove it
			if (pPoly->NumVerts() < 3)
			{
				delete pPoly;
				pBrush->m_Polies.Remove(nPolyLoop);
				--nPolyLoop;
				UpdateBrushGeometry(pBrush);
			}
		}
	}
}

void CEditRegion::RemoveCollinearVertices(CLinkedList<CEditBrush*> *pBrushes)
{
	uint32		i, k;
	CEditBrush	*pBrush;
	CEditPoly	*pPoly;
	LPOS pos;

	if(!pBrushes)
		pBrushes = &m_Brushes;

	for(pos=pBrushes->GetHeadPosition(); pos; )
	{
		pBrush = pBrushes->GetNext(pos);

		for( k=0; k < pBrush->m_Polies; k++ )
		{
			pPoly = pBrush->m_Polies[k];

			// Remove the vertices.
			pPoly->RemoveCollinearVertices();
			
			// Remove it if it went bad.
			if( pPoly->NumVerts() < 3 )
			{
				delete pPoly;
				pBrush->m_Polies.Remove( k );
				i = (uint32)-1;
				break;
			}
		}
	}
}


void CEditRegion::RemoveDuplicatePoints(CLinkedList<CEditBrush*> *pBrushes)
{
	int32			k;

	CEditBrush		*pBrush;
	CEditPoly		*pPoly;
	LPOS pos;

	if(!pBrushes)
		pBrushes = &m_Brushes;

	for(pos=pBrushes->GetHeadPosition(); pos; )
	{
		pBrush = pBrushes->GetNext(pos);

		// Stitch together any points that are in different polies
		pBrush->RemoveDuplicatePoints(0.01f);
 
		for( k=0; k < (int32)pBrush->m_Polies; k++ )
		{
			pPoly = pBrush->m_Polies[k];

			if (pPoly->NumVerts() > 2)
			{
				// Find duplicate indices and remove them
				for (uint32 nDupePointLoop = 0; nDupePointLoop < pPoly->NumVerts(); )
				{
					if (pPoly->Index(nDupePointLoop) == pPoly->NextIndex(nDupePointLoop))
					{
						pPoly->m_Indices.Remove(nDupePointLoop);
					}
					else
						++nDupePointLoop;
				}
			}

			if( pPoly->NumVerts() < 3 )
			{
				delete pPoly;
				pBrush->m_Polies.Remove( k );
				--k;
			}
		}
	}
}

static float SnapFloat(float fValue, float fThreshold)
{
	int iSnapInt = (int)(fValue + ((fValue < 0.0f) ? -fThreshold : fThreshold));
	if (fabs(fValue - iSnapInt) < fThreshold)
		return (float)iSnapInt;
	else
		return fValue;
}

// Snaps the vertices of a brush to integer values based on a threshold
void CEditRegion::FixInaccurateGeometry(CLinkedList<CEditBrush*> *pBrushes, float fThreshold)
{
	uint32			i;
	CEditBrush		*pBrush;
	LPOS pos;

	if(!pBrushes)
		pBrushes = &m_Brushes;

	for(pos=pBrushes->GetHeadPosition(); pos; )
	{
		pBrush = pBrushes->GetNext(pos);

		for( i = 0; i < pBrush->m_Points.GetSize(); i++)
		{
			CVector vPoint = pBrush->m_Points[i];
			if (vPoint.x != (int32)vPoint.x)
				vPoint.x = SnapFloat(vPoint.x, fThreshold);
			if (vPoint.y != (int32)vPoint.y)
				vPoint.y = SnapFloat(vPoint.y, fThreshold);
			if (vPoint.z != (int32)vPoint.z) 
				vPoint.z = SnapFloat(vPoint.z, fThreshold);
			pBrush->m_Points[i] = vPoint;
		}
	}
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditRegion::CopyRegion
//      PURPOSE:        Makes a copy of the input region.
// ----------------------------------------------------------------------- //

void CEditRegion::CopyRegion( CEditRegion *pOther )
{
	CEditBrush *pBrush, *pOtherBrush;
	LPOS pos;

	m_pStringHolder = pOther->m_pStringHolder;
	
	for(pos=pOther->m_Brushes; pos; )
	{
		pOtherBrush = pOther->m_Brushes.GetNext(pos);

		pBrush = new CEditBrush;
		m_Brushes.AddTail(pBrush);
		pBrush->CopyEditBrush(pOtherBrush);
	}
}


void CEditRegion::RemoveUnusedBrushes()
{
	LPOS curBrushPos, pos;
	CEditBrush *pBrush;

	for(pos=m_Brushes; pos; )
	{
		curBrushPos = pos;
		pBrush = m_Brushes.GetNext(pos);

		if(pBrush->m_Polies == 0)
		{
			no_DestroyNode(this, pBrush, FALSE);
		}
	}
}


#ifdef DIRECTEDITOR_BUILD
	void CEditRegion::UpdateTextureIDs( CEditProjectMgr *pProj )
	{
		LPOS pos;
		CEditBrush *pBrush;

		for(pos=m_Brushes; pos; )
		{
			pBrush = m_Brushes.GetNext(pos);

			for( uint32 k=0; k < pBrush->m_Polies; k++ )
				for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
					pBrush->m_Polies[k]->GetTexture(nCurrTex).UpdateTextureID();
		}
	}

#endif

// eliminates duplicate strings within the poly soup
void CEditRegion::UpdateTextureStrings()
{
	LPOS pos;
	CEditBrush *pBrush;
	uint32 i;
	CEditPoly *pPoly;

	for(pos=m_Brushes; pos; )
	{
		pBrush = m_Brushes.GetNext(pos);

		for(i=0; i < pBrush->m_Polies; i++)
		{
			pPoly = pBrush->m_Polies[i];
		
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				if(pPoly->GetTexture(nCurrTex).m_pTextureName)
					pPoly->GetTexture(nCurrTex).m_pTextureName = m_pStringHolder->AddString(pPoly->GetTexture(nCurrTex).m_pTextureName);
			}
		}
	}
}


void CEditRegion::UpdatePlanes(CLinkedList<CEditBrush*> *pBrushes)
{
	LPOS pos;
	CEditBrush *pBrush;

	if(!pBrushes)
		pBrushes = &m_Brushes;
	
	for(pos=pBrushes->GetHeadPosition(); pos; )
	{
		pBrush = pBrushes->GetNext(pos);

		//have the brush update its planes
		if(pBrush)
		{
			pBrush->UpdatePlanes();
		}
	}
}

void CEditRegion::GetBoxIntersectBrushes( CEditBrush *pTestBrush, CMoArray<CEditBrush*> &intersects )
{
	LPOS pos;
	CEditBrush *pBrush;

	intersects.Term();
	intersects.SetCacheSize( 100 );

	//get the source object's bounding box
	CBoundingBox SrcBox = pTestBrush->CalcBoundingBox();

	for(pos=m_Brushes; pos; )
	{
		pBrush = m_Brushes.GetNext(pos);

		if(pBrush != pTestBrush)
		{
			if( SrcBox.Intersects(pBrush->CalcBoundingBox()) )
				intersects.Append(pBrush);
		}
	}
}

/************************************************************************/
// This function takes a world node and adds it and its children nodes
// to an array.  It is useful if you want to work with a flattened array
// of nodes rather than a tree of nodes.
void CEditRegion::FlattenTreeToArray(CWorldNode *pRoot, CMoArray<CWorldNode*> &array)
{
	// Add the root node to the array
	array.Append(pRoot);

	// Add the children	
	GPOS pos=pRoot->m_Children;
	while (pos)
	{
		// Get the child node
		CWorldNode *pChild=pRoot->m_Children.GetNext(pos);

		// Recurse into the child node
		FlattenTreeToArray(pChild, array);
	}
}

/************************************************************************/
// Offset (move them in the world) the selected nodes by a vector
void CEditRegion::OffsetSelectedNodes(CVector vOffset)
{
	// Go through each selection and offset them in the world
	uint32 i;
	for (i=0; i < GetNumSelections(); i++)
	{
		// Get the selection
		CWorldNode *pSelection=GetSelection(i);

		// Switch on the type
		switch (pSelection->GetType())
		{
		case Node_Brush:
			{
				// Get the brush pointer...
				CEditBrush *pBrush = pSelection->AsBrush();
				ASSERT(pBrush);

				// Calculate the new position
				CVector vPosition=pBrush->GetUpperLeftCornerPos()+vOffset;

				// Move the brush
				pBrush->MoveBrush(vPosition);
				break;
			}
		case Node_Object:
		case Node_PrefabRef:
			{
				// Calculate the new position
				CVector vPosition=pSelection->GetUpperLeftCornerPos()+vOffset;

				// Move the object
				pSelection->SetPos(vPosition);
				break;
			}
		}
	}

}

// Scale everything in the region by a scaling factor
void CEditRegion::ScaleBy(float fScaleFactor)
{
	uint32 iLoop;

	// Scale all of the brushes
	for(LPOS iterateMPos = m_Brushes; iterateMPos; )
	{
		CEditBrush *pBrush = m_Brushes.GetNext(iterateMPos);

		// Scale all of the points around the origin
		for(iLoop = 0; iLoop < pBrush->m_Points; iLoop++)
			pBrush->m_Points[iLoop] *= fScaleFactor;
	
		// Update the bounding box
		pBrush->UpdateBoundingInfo();

		// Update the polies of the brush
		for(iLoop = 0; iLoop < pBrush->m_Polies; iLoop++)
		{
			CEditPoly *pPoly = pBrush->m_Polies[iLoop];
			// Scale the texture, too..
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);
				pPoly->SetTextureSpace(nCurrTex, Texture.GetO() * fScaleFactor, Texture.GetP() / fScaleFactor, Texture.GetQ() / fScaleFactor);
			}
			pPoly->UpdatePlane();
		}
	}

	// Scale the objects
	for(iLoop = 0; iLoop < m_Objects.GetSize(); iLoop++)
	{
		CBaseEditObj *pObject = m_Objects[iLoop];

		// Scale any dimension, distance, or radius properties 
		for(int iPropLoop = 0; iPropLoop < pObject->m_PropList.GetSize(); iPropLoop++)
		{
			CBaseProp *pProp = pObject->m_PropList.GetAt(iPropLoop);

			// Handle a dimension or vector distance property
			if ((pProp->GetFlags() & (PF_DIMS | PF_LOCALDIMS | PF_DISTANCE)) && (pProp->m_Type == LT_PT_VECTOR))
			{
				((CVectorProp *)pProp)->m_Vector *= fScaleFactor;
#ifdef DIRECTEDITOR_BUILD
				pObject->OnPropertyChanged(pProp, true, NULL);
#endif
			}

			// Handle a radius or scalar distance property
			else if ((pProp->GetFlags() & (PF_RADIUS | PF_FOVRADIUS | PF_DISTANCE)) && (pProp->m_Type == LT_PT_REAL))
			{
				((CRealProp *)pProp)->m_Value *= fScaleFactor;
#ifdef DIRECTEDITOR_BUILD
				pObject->OnPropertyChanged(pProp, true, NULL);
#endif
			}
			// Otherwise, it's an unknown property and we can't chance trying to scale it...
		}
	}
}

CBaseEditObj* CEditRegion::GetFirstSelectedObject()
{
	for( uint32 i=0; i < m_Selections; i++ )
	{
		if(m_Selections[i]->GetType() == Node_Object)
		{
			return m_Selections[i]->AsObject();
		}
	}

	return NULL;
}


void CEditRegion::DoSelectionOperation( CWorldNode *pNode, BOOL bMultiSelect, BOOL bDoSubtree )
{
	if( pNode->IsFlagSet(NODEFLAG_SELECTED) )
	{
		// Unselect everything.
		if( !bMultiSelect )
			ClearSelections();

		if( bDoSubtree )
			RecurseAndUnselect( pNode );
		else
			UnselectNode( pNode );
	}
	else
	{
		// Select everything.
		if( !bMultiSelect )
			ClearSelections();

		if( bDoSubtree )
			RecurseAndSelect( pNode );
		else
			SelectNode( pNode );
	}
}


void CEditRegion::RecurseAndSelect( CWorldNode *pNode, BOOL bUpdateSelectionArray )
{
	GPOS pos;

	SelectNode( pNode, bUpdateSelectionArray );

	for(pos=pNode->m_Children; pos; )
		RecurseAndSelect(pNode->m_Children.GetNext(pos), bUpdateSelectionArray);
}


void CEditRegion::RecurseAndUnselect( CWorldNode *pNode, BOOL bUpdateSelectionArray )
{
	GPOS pos;
	
	UnselectNode( pNode, bUpdateSelectionArray );

	for(pos=pNode->m_Children; pos; )
		RecurseAndUnselect(pNode->m_Children.GetNext(pos), bUpdateSelectionArray);
}

void CEditRegion::RecurseAndInverseSelect( CWorldNode *pNode, BOOL bUpdateSelectionArray )
{		
	// Check to see if the node is selected
	if (pNode->IsFlagSet(NODEFLAG_SELECTED))
	{
		// Unselect the node
		UnselectNode(pNode, bUpdateSelectionArray);
	}
	else
	{
		// Select the node
		SelectNode(pNode, bUpdateSelectionArray);
	}

	GPOS pos;
	for(pos=pNode->m_Children; pos; )
		RecurseAndInverseSelect(pNode->m_Children.GetNext(pos), bUpdateSelectionArray);
}

void CEditRegion::RecurseAndInverseHide( CWorldNode *pNode )
{
	// Check to see if the node is selected
	if (!pNode->IsFlagSet(NODEFLAG_SELECTED))
	{
#ifdef DIRECTEDITOR_BUILD
		pNode->HideNode();
#endif
	}
	
	// Recurse through the children
	GPOS pos=pNode->m_Children;
	while (pos)
	{
		RecurseAndInverseHide(pNode->m_Children.GetNext(pos));
	}
}

void CEditRegion::RecurseAndInverseUnhide( CWorldNode *pNode )
{
	// Check to see if the node is selected
	if (!pNode->IsFlagSet(NODEFLAG_SELECTED))
	{		
#ifdef DIRECTEDITOR_BUILD
		pNode->ShowNode();
#endif
	}
	
	// Recurse through the children
	GPOS pos=pNode->m_Children;
	while (pos)
	{
		RecurseAndInverseUnhide(pNode->m_Children.GetNext(pos));
	}
}

void CEditRegion::SelectNode( CWorldNode *pNode, BOOL bUpdateSelectionArray )
{
	//sanity check
	ASSERT(pNode);

	//don't bother to atempt to select a frozen node
	if(pNode->IsFlagSet(NODEFLAG_FROZEN))
	{
		return;
	}

	// Determine if the selection array should be used
	if (bUpdateSelectionArray)
	{
		// Find the selection
		uint32	index = m_Selections.FindElement( pNode );
		if( index == BAD_INDEX )
		{
			// Update the node flag
			pNode->EnableFlag(NODEFLAG_SELECTED);	

			// Add the selection to the selection array
			m_Selections.Append( pNode );
#ifdef DIRECTEDITOR_BUILD
			GetNodeView( )->UpdateNodeImage( pNode, FALSE );
#endif
		}
	}
	else
	{
		// Just update the node flag
		pNode->EnableFlag(NODEFLAG_SELECTED);		
#ifdef DIRECTEDITOR_BUILD
		GetNodeView( )->UpdateNodeImage( pNode, FALSE );
#endif
	}
}


void CEditRegion::UnselectNode( CWorldNode *pNode, BOOL bUpdateSelectionArray )
{
	if(!pNode->IsFlagSet(NODEFLAG_SELECTED))
		return;

	// Determine if the selection array should be used
	if (bUpdateSelectionArray)
	{
		// Find the selection
		uint32	index = m_Selections.FindElement( pNode );
		if( index != BAD_INDEX )
		{
			// Update the node flag
			pNode->ClearFlag(NODEFLAG_SELECTED);	

			// Add the selection to the selection array
			m_Selections.Remove( index );
#ifdef DIRECTEDITOR_BUILD
			GetNodeView( )->UpdateNodeImage( pNode, FALSE );
#endif
		}
	}
	else
	{
		// Just update the node flag
		pNode->ClearFlag(NODEFLAG_SELECTED);	
#ifdef DIRECTEDITOR_BUILD
		GetNodeView( )->UpdateNodeImage( pNode, FALSE );
#endif
	}
}

void CEditRegion::FreezeNode( CWorldNode *pNode, bool bFreeze, bool bDeselect, bool bRecurse )
{
	if(!pNode)
		return;

	//we can't freeze selected nodes, so unselect
	if(bFreeze && bDeselect && pNode->IsFlagSet(NODEFLAG_SELECTED))
	{
		UnselectNode(pNode);
	}

	//we can now unfreeze this node
	pNode->SetFlag(NODEFLAG_FROZEN, bFreeze);

	//see if we need to recurse
	if(bRecurse)
	{
		GPOS ChildPos = pNode->m_Children;

		while(ChildPos)
		{
			FreezeNode(pNode->m_Children.GetNext(ChildPos), bFreeze, bDeselect, bRecurse);
		}
	}
}

/************************************************************************/
// Updates the selection array
void CEditRegion::UpdateSelectionArray()
{
	// Clear the selection array
	m_Selections.SetSize(0);

	// Build the selection array
	RecurseUpdateSelectionArray(GetRootNode());
}

/************************************************************************/
// Generates a unique name for the specified object
// bUpdateRefProps			- Updates referencing properties
// bUpdateRefSelectedOnly	- Only updates referencing objects that are
//							  selected.
BOOL CEditRegion::GenerateUniqueNameForNode(CWorldNode *pNode)
{
	// Get the object name
	const char *nodeName=pNode->GetName();
	ASSERT(nodeName);	

	// Get the length of the name string
	int nNameLength=strlen(nodeName);

	// Here is a sample name:  Trigger441
	//                                 ^- Numeric component

	// Get the numeric component of the name
	int nIndex=nNameLength-1;
	while (nIndex > 0 && isdigit(nodeName[nIndex]))
	{
		nIndex--;
	}

	// Get the number as an integer
	int nObjectNumber=atoi(nodeName+nIndex+1);

	// Get the non-number portion of the string
	const int bufferSize=128;

	char lpszNonNumber[bufferSize];
	lpszNonNumber[0]='\0';
	strncpy(lpszNonNumber, nodeName, LTMIN(nIndex+1, bufferSize-1));
	lpszNonNumber[LTMIN(nIndex+1, bufferSize-1)]='\0';

	// Build the new name, incrementing the number component until
	// the name is unique.
	char lpszNewName[256];
	sprintf(lpszNewName, "%s%02d", lpszNonNumber, nObjectNumber);
	while (FindNodeByName(lpszNewName) != NULL)
	{
		nObjectNumber++;
		sprintf(lpszNewName, "%s%02d", lpszNonNumber, nObjectNumber);
	}

	// Set the name
	pNode->SetName(lpszNewName);

#ifdef DIRECTEDITOR_BUILD
	// Update the label in the node view
	GetNodeView()->UpdateTreeItemLabel(pNode);
#endif

	return TRUE;
}


#ifdef DIRECTEDITOR_BUILD

/************************************************************************/
// Generates unique names for the selected nodes.
// pOldNamesArray	- Filled with the names of the objects that have been changed
// pNewNamesArray	- Filled with the new names of the objects that have been changed
// pActionList		- Stores the undo information
// bStoreUndoOnly	- Set this to TRUE to only store the UNDO information about the nodes.
void CEditRegion::GenerateUniqueNamesForSelected(CStringArray *pOldNamesArray, CStringArray *pNewNamesArray, PreActionList *pActionList, BOOL bStoreUndoOnly)
{
	// Go through the selected objects
	int i;
	for (i=0; i < GetNumSelections(); i++)
	{
		// Get the node
		CWorldNode *pNode=GetSelection(i);

		// Check to see if the node is an object
		if ((pNode->GetType() != Node_Object) && (pNode->GetType() != Node_PrefabRef))
		{
			// Continue on to the next node
			continue;
		}

		// Get the current name
		char aCurName[256];
		char *pNodeName = pNode->GetName();

		strncpy(aCurName, pNodeName, sizeof(aCurName));
		aCurName[sizeof(aCurName) - 1] = 0;

		// Put an invalid character in the name so it doesn't show up in our search
		char cSaveStart = pNodeName[0];
		if (!cSaveStart)
		{
			pNodeName[1] = 0;
		}
		pNodeName[0] = 1;

		// Check to see if there are other nodes with this name
		bool bNeedsRename = FindNodeByName(aCurName) != NULL;

		// Put the name back to how it was..
		pNodeName[0] = cSaveStart;

		if (!bNeedsRename)
		{
			// This object already has a unique name
			continue;
		}

		// Setup the undo information
		if (bStoreUndoOnly)
		{
			ASSERT(pActionList);
			pActionList->AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));

			// Go to the next node since we are only storing the undo information
			continue;
		}

		// Generate a unique name for the object
		CString sOriginalName;
		if (pOldNamesArray)
		{
			// Store the original name
			sOriginalName=pNode->GetName();			
		}				

		// Generate a unique name for the object
		if (GenerateUniqueNameForNode(pNode))
		{
			// Add the original name to the array
			if (pOldNamesArray)
			{
				pOldNamesArray->Add(sOriginalName);
			}

			// Add the new name to the array
			if (pNewNamesArray)
			{
				pNewNamesArray->Add(pNode->GetName());
			}
		}
	}
}

/************************************************************************/
// Updates properties for referencing nodes in the world.
//
// bSelectedOnly		- Indicates that only the selected objects should be modified
// lpszOldName			- The original name of the object
// lpszNewName			- The updated name for the object
// pPropertyNameArray	- Filled in with the object::property names that are changed so that a report dialog can be made
// pOriginalValues		- The original values for a property that has changed
// pUpdatedValues		- The updated values for the property that has changed
// pActionList			- Stores the undo information
// bStoreUndoOnly		- Set this to TRUE to only store the UNDO information about the nodes.
void CEditRegion::UpdateObjectsReferenceProps(BOOL bSelectedOnly, const char *lpszOldName, const char *lpszNewName,
											  CStringArray *pPropertyNameArray, CStringArray *pOriginalValues, CStringArray *pUpdatedValues,
											  PreActionList *pActionList, BOOL bStoreUndoOnly)
{
	int i;
	for (i=0; i < m_Objects.GetSize(); i++)
	{
		// Get the object
		CBaseEditObj *pCurrentObject=m_Objects[i];

		// Make sure that this object is selected if it needs to be
		if (bSelectedOnly && !pCurrentObject->IsFlagSet(NODEFLAG_SELECTED))
		{
			continue;
		}

		// Update the object properties
		UpdateNodeRefProps(pCurrentObject, lpszOldName, lpszNewName, pPropertyNameArray, pOriginalValues, pUpdatedValues, pActionList, bStoreUndoOnly);	
	}
}

/************************************************************************/
// Updates properties that reference a node that has its name
// changed. For example, when an objects name has changed, properties
// that reference the object must also be updated.
//
// pNode				- The node that is to be updated
// lpszOldName			- The original name of the object
// lpszNewName			- The updated name for the object
// pPropertyNameArray	- Filled in with the object::property names that are changed so that a report dialog can be made
// pOriginalValues		- The original values for a property that has changed
// pUpdatedValues		- The updated values for the property that has changed
// pActionList			- Stores the undo information
// bStoreUndoOnly		- Set this to TRUE to only store the UNDO information about the nodes.
//
// Returns:		TRUE	- The node had updated properties
//				FALSE	- The node did not need to be updated
BOOL CEditRegion::UpdateNodeRefProps(CWorldNode *pNode, const char *lpszOldObjectName, const char *lpszNewObjectName,
									 CStringArray *pPropertyNameArray, CStringArray *pOriginalValues, CStringArray *pUpdatedValues,
									 PreActionList *pActionList, BOOL bStoreUndoOnly)
{
	// Check the parameters
	if (!pNode || !lpszOldObjectName || !lpszNewObjectName)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Get the property list
	CPropList *pPropList=pNode->GetPropertyList();
	if (!pPropList)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// This is changed to TRUE if the object has been updated
	BOOL bUpdated=FALSE;

	// Check each property
	int i;
	for (i=0; i < pPropList->GetSize(); i++)
	{
		// Get the property
		CBaseProp *pProp=pPropList->GetAt(i);

		// Check to see if the property is a string property
		// and if it contains the PF_OBJECTLINK flag.
		if (!(pProp->GetType() == LT_PT_STRING && pProp->m_PropFlags & PF_OBJECTLINK))
		{
			// Go to the next property
			continue;
		}

		// Get the string property
		CStringProp *pStringProp=(CStringProp *)pProp;

		// Compare the names
		if (strcmp(pStringProp->GetString(), lpszOldObjectName) == 0)
		{			
			// Check to see if we should only be storing undo information
			if (bStoreUndoOnly)
			{
				ASSERT(pActionList);
				pActionList->AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));

				// The undo information has been stored and the property values
				// don't actually change.
				return TRUE;
			}

			// Add the report information for the property name
			if (pPropertyNameArray)
			{
				CString sString;
				sString.Format("%s::%s", pNode->GetName(), pStringProp->GetName());
				pPropertyNameArray->Add(sString);
			}

			// Add the report information for the original property value
			if (pOriginalValues)
			{
				pOriginalValues->Add(pStringProp->GetString());
			}
			
			// Update the string property with the new name
			pStringProp->SetString(lpszNewObjectName);

			// Add the report information for the updated property value
			if (pUpdatedValues)
			{
				pUpdatedValues->Add(pStringProp->GetString());
			}

			// Set the update flag
			bUpdated=TRUE;
		}
	}

	// Return whether or not the object was updated
	return bUpdated;
}

#endif

/************************************************************************/
// Updates the selection array recursively
void CEditRegion::RecurseUpdateSelectionArray(CWorldNode *pParentNode)
{
	// Check to see if this node is selected
	if (pParentNode->IsFlagSet(NODEFLAG_SELECTED))
	{
		// Add the node to the selection array
		m_Selections.Append(pParentNode);
	}

	// Update all of the children
	GPOS pos=pParentNode->m_Children;
	while (pos)
	{
		RecurseUpdateSelectionArray(pParentNode->m_Children.GetNext(pos));
	}	
}

CWorldNode* CEditRegion::AddNullNode(CWorldNode *pParentNode)
{
	// Setup the parent
	if (pParentNode == NULL)
	{
		pParentNode=&m_RootNode;
	}

	CWorldNode *pRet=new CWorldNode;;	

	pRet->SetParent( pParentNode );
	pParentNode->m_Children.Append( pRet );
#ifdef DIRECTEDITOR_BUILD
	GetNodeView( )->AddNode( pRet );
#endif
	
	return pRet;
}


void CEditRegion::AddNodeToRoot( CWorldNode *pNode )
{
	no_AttachNode(this, pNode, &m_RootNode);
}


void CEditRegion::AttachNode( CWorldNode *pChild, CWorldNode *pParent )
{
	no_AttachNode(this, pChild, pParent);
}


void CEditRegion::DetachNode( CWorldNode *pNode, BOOL bAttachToRoot )
{
	no_DetachNode(this, pNode, bAttachToRoot);
}


void CEditRegion::ClearSelections()
{
	for( uint32 i=0; i < m_Selections; i++ )
		m_Selections[i]->ClearFlag(NODEFLAG_SELECTED);

	m_Selections.SetSize(0);

#ifdef DIRECTEDITOR_BUILD
	if( this == GetNodeView( )->GetDoc( )->GetRegion( ))
		GetNodeView( )->UpdateNodeImage( GetRootNode( ));
#endif
}

void CEditRegion::ClearPathNodes()
{
	for( uint32 i=0; i < m_PathNodes.GetSize(); i++ )
		m_PathNodes[i]->ClearFlag(NODEFLAG_PATH);

	m_PathNodes.SetSize(0);

#ifdef DIRECTEDITOR_BUILD
	if( this == GetNodeView( )->GetDoc( )->GetRegion( ))
		GetNodeView( )->UpdateNodeImage( GetRootNode( ));
#endif
}


CWorldNode* CEditRegion::FindNodeByID(uint32 id)
{
	return RecurseAndFindNodeByID(GetRootNode(), id);
}


static CWorldNode* RecurseAndFindNodeByName(CWorldNode *pRoot, char *pName)
{
	CWorldNode *pRet;
	GPOS pos;
	CStringProp *pNameProp;

	if(pNameProp = (CStringProp*)pRoot->m_PropList.GetMatchingProp(g_NameName, LT_PT_STRING))
	{
		if(strcmp(pNameProp->m_String, pName) == 0)
		{
			return pRoot;
		}
	}

	for(pos=pRoot->m_Children; pos; )
	{
		if(pRet = RecurseAndFindNodeByName(pRoot->m_Children.GetNext(pos), pName))
			return pRet;
	}

	return NULL;
}

CWorldNode* CEditRegion::FindNodeByName(char *pName)
{
	return RecurseAndFindNodeByName(GetRootNode(), pName);
}


uint32 CEditRegion::FindObjectsByName(const char *pName, CBaseEditObj **objectList, uint32 maxObjects)
{
	uint32 i, nFound;
	CBaseEditObj *pObj;
	char *pObjName;

	nFound = 0;
	for(i=0; i < m_Objects; i++)
	{
		pObj = m_Objects[i];
		if(pObjName = pObj->GetName())
		{
			if(_stricmp(pObjName, pName) == 0)
			{
				if(nFound >= maxObjects)
					return nFound;

				if (objectList != NULL)
				{
					objectList[nFound] = pObj;
				}

				nFound++;
			}
		}
	}

	return nFound;
}


#ifdef DIRECTEDITOR_BUILD
// Returns the node which is marked as the "active parent" which means that
// new nodes get added to this parent.  The root node is returned if an
// active parent cannot be found.
CWorldNode *CEditRegion::GetActiveParentNode(CWorldNode *pStartNode)
{
	// Check the root node if we are at the start
	BOOL bRoot=FALSE;
	if (pStartNode == NULL)
	{
		pStartNode=GetRootNode();
		bRoot=TRUE;
	}

	if (pStartNode == m_pActiveParentNode)
	{
		return pStartNode;
	}

	// Check the children
	GPOS gPos;
	for (gPos=pStartNode->m_Children; gPos; )
	{
		CWorldNode *pNode=GetActiveParentNode(pStartNode->m_Children.GetNext(gPos));
		if (pNode)
		{
			// We found the node so return it up the stack
			return pNode;
		}		
	}

	// If we still haven't found the node after checking all of them, then set
	// the active parent to the root
	if (bRoot)
	{
		// Set the active parent to the root node
		SetActiveParentNode(GetRootNode());
		return m_pActiveParentNode;			
	}

	// We didn't find the node
	return NULL;
}

// Sets the active parent node
void CEditRegion::SetActiveParentNode(CWorldNode *pNode)
{
	m_pActiveParentNode=pNode;	
}

static BOOL UpdateObjectClassIconsRecurse(CWorldNode* pNode)
{
	ASSERT(pNode);
	if(pNode == NULL)
	{
		return FALSE;
	}

	//update this node
	if(pNode->GetType() == Node_Object)
	{
		pNode->AsObject()->UpdateObjectClassImage();
	}

	//recurse through each of its children
	for(GPOS pos = pNode->m_Children; pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(pos);
		UpdateObjectClassIconsRecurse(pChild);
	}
	
	return TRUE;
}

//called in order to have all objects in the region update the icon that they are using for
//the class icon representation. This should be called when the icon directory changes
BOOL CEditRegion::UpdateObjectClassIcons()
{
	//run through the list of nodes, if we encounter an object, tell it to update it 
	//class icon
	return UpdateObjectClassIconsRecurse(GetRootNode());
}


static void RecurseAndRefreshAllNodeProperties( CWorldNode *pRoot, const char *pModifiers )
{
	GPOS pos;

	if( (pRoot->m_Type == Node_Object) || (pRoot->m_Type == Node_PrefabRef) )
	{
		pRoot->RefreshAllProperties( pModifiers );
	}

	for( pos = pRoot->m_Children; pos; )
	{
		RecurseAndRefreshAllNodeProperties( pRoot->m_Children.GetNext(pos), pModifiers );
	}

	return;
}

void CEditRegion::UpdateAllObjectProperties( const char *pModifiers  )
{
	RecurseAndRefreshAllNodeProperties( GetRootNode(), pModifiers );
}


#endif


//called to add an object to the object list
void CEditRegion::AddObject(CBaseEditObj* pObject)
{
	//add this object onto the list
	m_Objects.Append(pObject);

	//make the object point to us
	pObject->m_pRegion = this;
}

//called to remove an object from the object list
void CEditRegion::RemoveObject(CBaseEditObj* pObject)
{
	//remove it from our internal lists
	m_Objects.Remove(m_Objects.FindElement(pObject));

	//clear its reference to us
	pObject->m_pRegion = NULL;
}

//called when a brush is added
void CEditRegion::AddBrush(CEditBrush* pBrush)
{
	//add it to our list
	m_Brushes.AddTail(pBrush);

	//have the brush refer to us
	pBrush->m_pRegion = this;
}

//called when a brush is removed
void CEditRegion::RemoveBrush(CEditBrush* pBrush)
{
	//now remove it from our internal lists
	LPOS lFound = m_Brushes.Find(pBrush);

	ASSERT(lFound);
	m_Brushes.RemoveAt(lFound);

	//clear the brush's reference to us
	pBrush->m_pRegion = NULL;
}

//called when a brush is modified, whether it be moved, or the vertices changed, etc.
void CEditRegion::UpdateBrushGeometry(CEditBrush* pBrush)
{
	//update the bouding information
	pBrush->UpdateBoundingInfo();

	//first off, update the planes
	pBrush->UpdatePlanes();
}

//updates everything about a brush, general case.
void CEditRegion::UpdateBrush(CEditBrush* pBrush)
{
	//the updating geometry currently handles everything we need
	UpdateBrushGeometry(pBrush);
}

//called when a node is updated. It will forward it to the appropriate updater
void CEditRegion::UpdateNode(CWorldNode* pNode)
{
	//also notify the region of the change if it is an applicable type
	if(pNode->GetType() == Node_Brush)
		UpdateBrush(pNode->AsBrush());
}

//this will remove a node from the path list and clear out any related flags
void CEditRegion::RemoveNodeFromPath(CWorldNode* pNode)
{
	//see if we are even in the path
	if(!pNode->IsFlagSet(NODEFLAG_PATH))
		return;

	uint32 nIndex = m_PathNodes.FindElement(pNode);
	if(nIndex != BAD_INDEX)
	{
		pNode->ClearFlag(NODEFLAG_PATH);
		m_PathNodes.Remove(nIndex);
	}
	else
	{
		assert("Error: Tried to remove a node that claimed to be a path but wasn\'t in the path list");
	}
}

//this will add a node to the list of nodes for paths
void CEditRegion::AddNodeToPath(CWorldNode* pNode)
{
	if(m_PathNodes.FindElement(pNode) == BAD_INDEX)
	{
		pNode->EnableFlag(NODEFLAG_PATH);
		m_PathNodes.Append(pNode);
	}
}


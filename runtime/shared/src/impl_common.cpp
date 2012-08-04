#include "bdefs.h"

#include "impl_common.h"
#include "de_objects.h"
#include "objectmgr.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "de_mainworld.h"
#include "packetdefs.h"
#include "syscounter.h"
#include "geomroutines.h"
#include "stringmgr.h"
#include "sysfile.h"
#include "ltengineobjects.h"

#ifndef __LINUX
    #include "renderstruct.h"
#endif

// holders
#include "compress.h"
static ICompress* compress;
define_holder(ICompress, compress);



struct IC_FileEntry
{
	FileEntry	m_Entry;
	char		m_StringData[1];
};

class PIWStruct
{
public:
	LTVector	m_Point;
	LTBOOL		m_bInside;
};






//---------------------------------------------------------------------------//


// ------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------ //



bool ci_IsPointInsideBSP
(
	const Node*		pRoot,
	const LTVector&	P
)
{
	for(;;)
	{
		if(pRoot->m_Flags & NF_OUT)
			return LTFALSE;

		if(pRoot->m_Flags & NF_IN)
			return LTTRUE;

		const LTPlane& plane = *pRoot->m_pPoly->GetPlane();
		const float d = plane.DistTo(P);//distance to plane
		const bool b = d > 0;

		pRoot = pRoot->m_Sides[ b ];
	}
}


// Callback function for ic_IsPointInsideWorld.
void ic_PointInsideWorldCB(WorldTreeObj *pObj, void *pUser)
{
	LTObject *pObject;
	PIWStruct *pStruct;
	LTVector vTransformedPoint;
	WorldModelInstance *pWM;


	if(pObj->GetObjType() != WTObj_DObject)
		return;

	pObject = (LTObject*)pObj;
	if(pObject->IsMainWorldModel())
	{
		pStruct = (PIWStruct*)pUser;
		pWM = pObject->ToWorldModel();

		MatVMul_H(&vTransformedPoint, &pWM->m_BackTransform, &pStruct->m_Point);

		if( !ci_IsPointInsideBSP( pWM->m_pOriginalBsp->GetRootNode(), vTransformedPoint ) )
		{
			pStruct->m_bInside = LTFALSE;
		}
	}
}


bool ic_IsPointInsideWorld(WorldTree *pWorldTree, const LTVector *pPoint)
{
	PIWStruct theStruct;

	theStruct.m_Point = *pPoint;
	theStruct.m_bInside = LTTRUE;
	
	pWorldTree->FindObjectsOnPoint(pPoint, ic_PointInsideWorldCB, &theStruct);
	return theStruct.m_bInside != 0;
}

// ------------------------------------------------------------------ //
// Interface implementation functions.
// ------------------------------------------------------------------ //

void ic_StartCounter(LTCounter *pCounter)
{
	cnt_StartCounterFinal(reinterpret_cast<CounterFinal&>(*pCounter));
}


uint32 ic_EndCounter(LTCounter *pCounter)
{
	return cnt_EndCounterFinal(reinterpret_cast<CounterFinal&>(*pCounter));
}


bool ic_UpperStrcmp(const char *pStr1, const char *pStr2)
{
	return CHelpers::UpperStrcmp(pStr1, pStr2) != 0;
}


// ----------------------------------------------------------------------- //
// Looks thru the model animations and finds one with the given number.
// Returns (uint32)-1 if it can't find it or if it's a bad object.
// ----------------------------------------------------------------------- //

HMODELANIM ic_GetAnimIndex(HOBJECT hObj, const char *pAnimName)
{
	Model *pModel;
	ModelAnim *pAnim = NULL;
	LTObject *pObj;
	uint32 index;

	pObj = (LTObject*)hObj;
	if(!pObj || pObj->m_ObjectType != OT_MODEL)
		return (HMODELANIM)-1;

	pModel = pObj->ToModel()->GetModelDB() ;
	if( !pModel )
		return (HMODELANIM)-1;

	pAnim = pModel->FindAnim(pAnimName, &index);

	if(pAnim)
		return index;
	else
		return (HMODELANIM)-1;
}

const char *ic_GetAnimName (HOBJECT hObj, HMODELANIM hAnim)
{
	if ( hAnim < 0 )
		return LTNULL;

	Model *pModel;
	LTObject *pObj;

	pObj = (LTObject*)hObj;
	if(!pObj || pObj->m_ObjectType != OT_MODEL)
		return LTNULL;

	pModel = pObj->ToModel()->GetModelDB();
	

	if ( hAnim >= pModel->NumAnims() )
		return LTNULL;

	return pModel->GetAnim(hAnim)->GetName();
}


void ic_FreeString(HSTRING hString)
{
	if(hString)
		str_FreeString(hString);
}
 

static LTBOOL ic_FindFileInList(IC_FileEntry *pList, char *pName)
{
	IC_FileEntry *pCur;

	pCur = pList;
	while(pCur)
	{
		if(stricmp(pCur->m_Entry.m_pBaseFilename, pName) == 0)
			return LTTRUE;

		pCur = (IC_FileEntry*)pCur->m_Entry.m_pNext;
	}

	return LTFALSE;
}


FileEntry* ic_GetFileList(HLTFileTree **trees, int nTrees, const char *pDirName)
{	
	IC_FileEntry *pEntry, *pList;
	LTFindInfo findInfo;
	unsigned long allocSize;
	char fullName[_MAX_PATH + 1];
	int i;

	pList = LTNULL;
	
	// Go thru the file trees, adding unique files (earlier ones override later ones).
	for(i=0; i < nTrees; i++)
	{
        findInfo.m_pInternal = LTNULL;
		while(df_FindNext(trees[i], pDirName, &findInfo))
		{
			if(!ic_FindFileInList(pList, findInfo.m_Name))
			{
				if(pDirName[0] == 0)
					LTStrCpy(fullName, findInfo.m_Name, sizeof(fullName));
				else
					LTSNPrintF(fullName, sizeof(fullName), "%s\\%s", pDirName, findInfo.m_Name);

				allocSize = (sizeof(IC_FileEntry)-1) + (strlen(findInfo.m_Name)+1) + (strlen(fullName)+1);
				LT_MEM_TRACK_ALLOC(pEntry = (IC_FileEntry*)dalloc(allocSize),LT_MEM_TYPE_MISC);
				
				pEntry->m_Entry.m_pBaseFilename = pEntry->m_StringData;
				pEntry->m_Entry.m_pFullFilename = pEntry->m_StringData + strlen(findInfo.m_Name) + 1;

				memcpy(pEntry->m_Entry.m_pBaseFilename, findInfo.m_Name, strlen(findInfo.m_Name)+1);
				memcpy(pEntry->m_Entry.m_pFullFilename, fullName, strlen(fullName)+1);

				pEntry->m_Entry.m_Type = (findInfo.m_Type == DIRECTORY_TYPE) ? TYPE_DIRECTORY : TYPE_FILE;

				pEntry->m_Entry.m_pNext = (FileEntry*)pList;
				pList = pEntry;
			}
		}
	}

	return (FileEntry*)pList;
}


void ic_FreeFileList(FileEntry *pList)
{
	FileEntry *pCur, *pNext;
	
	pCur = pList;
	while(pCur)
	{
		pNext = pCur->m_pNext;
		dfree(pCur);
		pCur = pNext;
	}
}


float ic_Random(LTFLOAT min, LTFLOAT max)
{
	LTFLOAT randNum = (LTFLOAT)rand() / RAND_MAX;
	
	return min + (max - min) * randNum;
}




#include "bdefs.h"

#include "servermgr.h"
#include "classmgr.h"
#include "s_object.h"
#include "classbind.h"
#include "bindmgr.h"
#include "dhashtable.h"
#include "ltobjectcreate.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);


#define FUNCTION_POINTER(theClass, theOffset) (*(void**)(((char*)theClass + theOffset)))

// Class tick related variables...
uint32 g_Ticks_ClassUpdate = 0;
extern int32 g_CV_ShowClassTicks;
extern char	*g_CV_ShowClassTicksSpecific;

// ----------------------------------------------------------------------- //
// CClassDatafunctions.
// ----------------------------------------------------------------------- //

CClassData::CClassData()
{
	m_nTicksThisUpdate	= 0;
	m_nUpdated			= 0;
	m_nTotal			= 0;
	m_nTickAveCnt		= 0;
	m_nTotalTicks		= 0;
	m_nMaxTicks			= 0;
	m_bDisplayedTicks	= LTFALSE;
}

CClassData::~CClassData()
{
	DestroyObjectTickData();
}

// Reset data that is recalculated every update...
void CClassData::ResetPerUpdateTickData()
{
    m_nTicksThisUpdate = 0;
    m_nUpdated = 0;
    m_nTotal = 0;
    m_bDisplayedTicks = LTFALSE;

	// Clear our relative per object data...
	if (IsDisplayingObjectTicks())
	{
		ResetPerUpdateObjectTickData();
	}
	else
	{
		DestroyObjectTickData();
	}

    // See if it is time to re-calculate our average / max...
    // Do this every 20 seconds or so...

    if (m_nTickAveCnt >= 600)
    {
        m_nTickAveCnt = 0;
        m_nTotalTicks = 0;
        m_nMaxTicks = 0;

		if (IsDisplayingObjectTicks())
		{
			ResetObjectAveMaxData();
		}
    }
}

// Reset all tick count data...
void CClassData::ResetAllTickData()
{
    m_nTicksThisUpdate = 0;
    m_nUpdated = 0;
    m_nTotal = 0;
    m_bDisplayedTicks = LTFALSE;
    m_nTickAveCnt = 0;
    m_nTotalTicks = 0;
	m_nMaxTicks = 0;

	DestroyObjectTickData();
}

// Update the class ticks associated with this object...
void CClassData::UpdateTicks(LTObject* pObj, uint32 nTicksThisUpdate)
{
	// Ticks for this class this update...
	m_nTicksThisUpdate += nTicksThisUpdate;

	// Number of objects of this class type updated...
	if (nTicksThisUpdate)
	{
		++m_nUpdated;
	}

	// If we're currently keeping instance tick information, update it...
	//
	if (IsDisplayingObjectTicks())
	{
		UpdateObjectDataTicks(pObj, nTicksThisUpdate);
	}

}

void CClassData::ResetPerUpdateObjectTickData()
{
	for (TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin(); 
		 iter != m_ObjectTickDataList.end(); iter++)
	{
		(*iter)->m_nTicksThisUpdate	= 0;
		(*iter)->m_bUpdatedData		= LTFALSE;
		(*iter)->m_bDisplayedTicks	= LTFALSE;
	}
}

void CClassData::DestroyObjectTickData()
{
	TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin();
	while (iter != m_ObjectTickDataList.end())
	{
		delete(*iter);
		iter++;
	}
	m_ObjectTickDataList.clear();
}

void CClassData::ResetObjectAveMaxData()
{
	for (TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin(); 
		 iter != m_ObjectTickDataList.end(); iter++)
	{
		(*iter)->m_nMaxTicks	= 0;
		(*iter)->m_nTickAveCnt	= 0;
		(*iter)->m_nTotalTicks	= 0;
	}
}

void CClassData::GetObjectName(LTObject* pObj, char* pObjectNameBuffer, uint32 nBufferLen)
{
	if (!pObj || !pObjectNameBuffer || nBufferLen < 16) return;

	pObjectNameBuffer[0] = '\0';

    if (pObj->sd->m_hName)
    {
        char* pName = (char*)hs_GetElementKey(pObj->sd->m_hName, LTNULL);
        LTStrCpy(pObjectNameBuffer, pName, nBufferLen);
    }
 
	if (!pObjectNameBuffer[0])
	{
		LTStrCpy(pObjectNameBuffer, "No Name", nBufferLen);
	}
}


CObjectTickData* CClassData::FindObjectTickData(const char* szObjectName)
{
	if (!szObjectName || !szObjectName[0]) return LTNULL;

	for (TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin(); 
		 iter != m_ObjectTickDataList.end(); iter++)
	{
		if (stricmp((*iter)->m_ObjectName, szObjectName) == 0)
		{
			return((*iter));
		}
	}

	return LTNULL;
}

void CClassData::UpdateObjectDataTicks(LTObject* pObj, uint32 nTicksThisUpdate)
{
	if (!pObj) return;

	// Get the object's name...

	char szName[64] = {0};
	GetObjectName(pObj, szName, sizeof(szName));


	// Find the object data associated with this object...

	CObjectTickData* pObjectTickData = FindObjectTickData(szName);


	// Add a new object tick data if necessary...

	if (!pObjectTickData)
	{
		pObjectTickData = new CObjectTickData;
		LTStrCpy(pObjectTickData->m_ObjectName, szName, sizeof(pObjectTickData->m_ObjectName));

		m_ObjectTickDataList.push_back(pObjectTickData);
	}

	// Set the number of ticks this object ate this update...

	if (!pObjectTickData->m_bUpdatedData)
	{
		pObjectTickData->m_nTicksThisUpdate = nTicksThisUpdate;
		pObjectTickData->m_bUpdatedData = LTTRUE;
	}
}

void CClassData::CalculateTotalObjectFrameTicks()
{
	// Loop over our object tick data list and update each object...

	for (TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin(); 
		 iter != m_ObjectTickDataList.end(); iter++)
	{
		// Increment our counter for determining the average number of ticks for this class type...
		(*iter)->m_nTickAveCnt++;

		// Calculate the total so far...
		(*iter)->m_nTotalTicks += (*iter)->m_nTicksThisUpdate;

		// See if we have a new max...
		if ((*iter)->m_nMaxTicks < (*iter)->m_nTicksThisUpdate)
		{
			(*iter)->m_nMaxTicks = (*iter)->m_nTicksThisUpdate;
		}
	}
}




// Determine if we are displaying object ticks for this class...
LTBOOL CClassData::IsDisplayingObjectTicks()
{
	LTBOOL bRet = LTFALSE;

	// First see if we're showing class specific ticks...

	if (2 == g_CV_ShowClassTicks && g_CV_ShowClassTicksSpecific)
	{
		// Now see if we are the class that is currently being displayed...

		char szClassName[64] = {0};
		GetClassName(szClassName, sizeof(szClassName), LTFALSE);

		if (stricmp(g_CV_ShowClassTicksSpecific, szClassName) == 0)
		{
			bRet = LTTRUE;
		}
	}

	return bRet;
}

void CClassData::CalculateTotalFrameTicks()
{
	// Increment our counter for determining the average number of ticks for this class type...
	m_nTickAveCnt++;

	// Calculate the total so far...
    m_nTotalTicks += m_nTicksThisUpdate;

	// See if we have a new max...
    if (m_nMaxTicks < m_nTicksThisUpdate)
    {
		m_nMaxTicks = m_nTicksThisUpdate;
    }

	if (IsDisplayingObjectTicks())
	{
		CalculateTotalObjectFrameTicks();
	}
}

void CClassData::DisplayTicks(uint32 & nTicks, uint32 & nAveTicks,
							  uint32 & nMaxTicks, uint32 & nTotalObjs)
{
	nTicks		= m_nTicksThisUpdate;
	nAveTicks	= GetAveTicks();
	nMaxTicks	= m_nMaxTicks;
	nTotalObjs	= m_nUpdated;
	
	char szClassName[32];
	GetClassName(szClassName, sizeof(szClassName), LTTRUE);

	dsi_ConsolePrint("%s|%4d | %4d  |           %6d | %6d | %6d",
		szClassName, m_nTotal, m_nUpdated, nAveTicks, nMaxTicks, nTicks);

	m_bDisplayedTicks = LTTRUE;
}

void CClassData::DisplayObjectTicks()
{
	char szObjectName[48] = {0}; // 32 wasn't big enough...

	// Loop over our object tick data list and display info for each object...
	// Display the ticks sorted base on average ticks (lowest to highest)...

	for (TObjectTickDataList::iterator iter = m_ObjectTickDataList.begin(); 
		 iter != m_ObjectTickDataList.end(); iter++)
	{
		CObjectTickData* pCurObjectTickData = LTNULL;

		// Find the lowest object ticks average not yet displayed...

		for (TObjectTickDataList::iterator it = m_ObjectTickDataList.begin(); 
			 it != m_ObjectTickDataList.end(); it++)
		{
			CObjectTickData* pObjectTickData = (*it);

			if (!pObjectTickData->m_bDisplayedTicks)
			{
				if (pObjectTickData->m_nTotalTicks)
				{
					if (!pCurObjectTickData)
					{
						pCurObjectTickData = pObjectTickData;
					}
					else
					{
						uint32 nAve1 = pObjectTickData->GetAveTicks();
						uint32 nAve2 = pCurObjectTickData->GetAveTicks();

						if (nAve1 <= nAve2)
						{
							pCurObjectTickData = pObjectTickData;
						}
					}
				}
				else
				{
					pObjectTickData->m_bDisplayedTicks = LTTRUE;
				}
			}
		}


		if (pCurObjectTickData)
		{
			// Pad the name with spaces...

			LTStrCpy(szObjectName, pCurObjectTickData->m_ObjectName, sizeof(szObjectName));

			for (uint32 i = strlen(szObjectName); i < sizeof(szObjectName) - 2; i++)
			{
				szObjectName[i] = ' ';
			}

			dsi_ConsolePrint("%s          %6d | %6d | %6d", szObjectName, 
				pCurObjectTickData->GetAveTicks(), pCurObjectTickData->m_nMaxTicks, 
				pCurObjectTickData->m_nTicksThisUpdate);

			pCurObjectTickData->m_bDisplayedTicks = LTTRUE;
		}
	}

}


void CClassData::GetClassName(char* pClassNameBuffer, uint32 nBufferLen, LTBOOL bPadWithSpace)
{
	if (!pClassNameBuffer || nBufferLen < 16) return;

	LTStrCpy(pClassNameBuffer, m_pClass->m_ClassName, nBufferLen);

	if (bPadWithSpace)
	{
		for (uint32 i = strlen(pClassNameBuffer); i < nBufferLen - 2; i++)
		{
			pClassNameBuffer[i] = ' ';
		}
	}
}


// ----------------------------------------------------------------------- //
// CClassMgr functions.
// ----------------------------------------------------------------------- //

CClassMgr::CClassMgr()
{
    memset(this, 0, sizeof(CClassMgr));
}

CClassMgr::~CClassMgr()
{
    Term();
}

LTBOOL CClassMgr::Init() {
    memset(this, 0, sizeof(CClassMgr));

    return LTTRUE;
}

void CClassMgr::Term()
{
    int i;

	cb_UnloadModule(m_ClassModule);

    if (m_hServerResourceModule)
    {
        bm_UnbindModule(m_hServerResourceModule);
        m_hServerResourceModule = LTNULL;
    }

    if (m_hClassNameHash)
    {
        hs_DestroyHashTable(m_hClassNameHash);
        m_hClassNameHash = LTNULL;
    }

    if (m_ClassDatas)
    {
        for (i=0; i < m_nClassDatas; i++)
        {
            sb_Term(&m_ClassDatas[i].m_ObjectBank);
			// Note : Don't clear the back-pointer in the ClassDef, since the module's already been unloaded
            // m_ClassDatas[i].m_pClass->m_pInternal[m_ClassIndex] = LTNULL;
        }

 
        delete [] m_ClassDatas;
        m_ClassDatas = LTNULL;
    }
}

CClassData* CClassMgr::FindClassData(const char *pName)
{
    HHashElement *hElement = hs_FindElement(m_hClassNameHash, pName, strlen(pName));

    if (hElement)
    {
        return (CClassData*)hs_GetElementUserData(hElement);
    }
    else
    {
        return LTNULL;
    }
}


void CClassMgr::ShowTickCounts()
{
#ifndef _FINAL

    if (g_CV_ShowClassTicks)  
    {
        // Add up the total number of all the classes...
 
		LTLink *pHead = &(g_pServerMgr->m_Objects.m_Head);
        for (LTLink* pCur=pHead->m_pNext; pCur != pHead; pCur=pCur->m_pNext)
        {
            LTObject* pObj = (LTObject*)pCur->m_pData;
            CClassData* pClassData = (CClassData*)pObj->sd->m_pClass->m_pInternal[m_ClassIndex];
            pClassData->IncrementTotalObjectCount();
        }

		CalculateVerboseTickInfo();

		//  Check to see if we are only showing ticks for a specific class...

		if (2 == g_CV_ShowClassTicks && g_CV_ShowClassTicksSpecific)
		{
			DisplayClassTickCounts(g_CV_ShowClassTicksSpecific);
		}
		else if (1 == g_CV_ShowClassTicks)  // Show ALL class tick counts.
		{
			DisplayTickCounts();
		}
    }
    else
    {
		// If it's disabled, just do calculate simple tick info

		CalculateSimpleTickInfo();
    }

#endif // _FINAL
}

void CClassMgr::ClearTickCounts()
{
#ifndef _FINAL
    // Clear tick counts for each class.
    for (int i=0; i < m_nClassDatas; i++)
    {
		m_ClassDatas[i].ResetPerUpdateTickData();
    }
#endif // _FINAL
}

// Display tick count information for ALL active object classes...
void CClassMgr::DisplayTickCounts()
{
#ifndef _FINAL
	
	g_Ticks_ClassUpdate = 0;
	uint32 nClassTicksAverage = 0;
	uint32 nClassTicksMax = 0;
	uint32 nTotalObjsUpdated = 0;

	dsi_ConsolePrint("------------------------- Class Tick Counts -------------------------------------");
	dsi_ConsolePrint("Class                (OBJECTS) Total Updated   (TICKS) Average    Max     Frame");
	dsi_ConsolePrint("---------------------------------------------------------------------------------");

	// Display the class data (sorted), with the lowest values
	// first...

	CClassData* pCurClass = &m_ClassDatas[0];

	uint32 nTotalObjects = 0;

	for (int32 j=0; j < m_nClassDatas; j++)
	{
		pCurClass = LTNULL;

		// Update the total object count...

		nTotalObjects += m_ClassDatas[j].GetTotalObjects();

		// Find the lowest class ticks average not yet displayed...

		for (int32 i=0; i < m_nClassDatas; i++)
		{
			CClassData* pClassData = &m_ClassDatas[i];

			if (!pClassData->HasDisplayedTicks())
			{
				if (pClassData->GetTotalTicks() || pClassData->GetTotalObjects())
				{
					if (!pCurClass)
					{
						pCurClass = pClassData;
					}
					else
					{
						uint32 nAve1 = pClassData->GetAveTicks();
						uint32 nAve2 = pCurClass->GetAveTicks();

						if (nAve1 <= nAve2)
						{
							pCurClass = pClassData;
						}
					}
				}
				else
				{
					pClassData->SetDisplayedTicks(LTTRUE);
				}
			}
		}

		if (pCurClass)
		{
			// Display this classes ticks and get the relative information to add to our
			// global display...

			uint32 nUpdateTicks = 0, nAveTicks = 0, nMaxTicks = 0, nTotalObjs = 0;
			pCurClass->DisplayTicks(nUpdateTicks, nAveTicks, nMaxTicks, nTotalObjs);

			// Update the totals...
			g_Ticks_ClassUpdate += nUpdateTicks;
			nClassTicksAverage	+= nAveTicks;
			nClassTicksMax		+= nMaxTicks;
			nTotalObjsUpdated	+= nTotalObjs;
		}
	}

	dsi_ConsolePrint(" ");
	dsi_ConsolePrint("FRAME TOTALS:        (OBJECTS) Total Updated   (TICKS) Average    Max     Frame");
	dsi_ConsolePrint("                               %4d |  %4d             %6d | %6d | %6d",
		nTotalObjects, nTotalObjsUpdated, nClassTicksAverage, nClassTicksMax, g_Ticks_ClassUpdate);
	dsi_ConsolePrint("---------------------------------------------------------------------------------");
	dsi_ConsolePrint(" ");

#endif // _FINAL
}

// Display detailed tick count information for ALL active objects of a particular class...
void CClassMgr::DisplayClassTickCounts(const char* pClassName)
{
#ifndef _FINAL

	if (!pClassName || !(*pClassName)) return;

	// Find the class specified...

	for (int32 i=0; i < m_nClassDatas; i++)
	{
		char szClassName[64] = {0};
		m_ClassDatas[i].GetClassName(szClassName, sizeof(szClassName), LTFALSE);

		if (stricmp(pClassName, szClassName) == 0)
		{
			// Display per-object tick information for this class...

			dsi_ConsolePrint("---------------------------------------------------------------------------------");
			dsi_ConsolePrint("'%s' INDIVIDUAL OBJECT TICK COUNTS:", szClassName);
			dsi_ConsolePrint("---------------------------------------------------------------------------------");
			dsi_ConsolePrint("Object Name                                     (TICKS) Average    Max     Frame");
			dsi_ConsolePrint("---------------------------------------------------------------------------------");

			m_ClassDatas[i].DisplayObjectTicks();

			dsi_ConsolePrint("---------------------------------------------------------------------------------");
			dsi_ConsolePrint("FRAME TOTALS          (OBJECTS) Total Updated   (TICKS) Average    Max     Frame");
			dsi_ConsolePrint("---------------------------------------------------------------------------------");

			// Display total class tick information...

			uint32 nTotalUpdate = 0, nAveUpdate = 0, nMaxTicks = 0, nTotalObjs = 0;
			m_ClassDatas[i].DisplayTicks(nTotalUpdate, nAveUpdate, nMaxTicks, nTotalObjs);

			dsi_ConsolePrint("---------------------------------------------------------------------------------");
			dsi_ConsolePrint(" ");
			break;
		}
	}

#endif // _FINAL
}

void CClassMgr::CalculateSimpleTickInfo()
{
#ifndef _FINAL

    g_Ticks_ClassUpdate = 0;

    for (int i=0; i < m_nClassDatas; i++)
    {
        g_Ticks_ClassUpdate += m_ClassDatas[i].GetTicksThisUpdate();

        // Clear the class ticks data...
		m_ClassDatas[i].ResetAllTickData();
    }

#endif // _FINAL
}

void CClassMgr::CalculateVerboseTickInfo()
{
#ifndef _FINAL

    for (int i=0; i < m_nClassDatas; i++)
    {
		m_ClassDatas[i].CalculateTotalFrameTicks();
    }

#endif // _FINAL
}


// ----------------------------------------------------------------------- //
// Static functions.
// ----------------------------------------------------------------------- //

static LTRESULT InitExtraClassData(CClassMgr *pClassMgr)
{
    ClassDef *pClass, **pClasses;
    int i, nClasses;
    CClassData *pClassData;
    HHashElement *hElement;

 
    
    pClasses = cb_GetClassDefs(&pClassMgr->m_ClassModule);
    nClasses = cb_GetNumClassDefs(&pClassMgr->m_ClassModule);


    if (nClasses > 0)
    {
        if (pClasses[0]->m_pInternal[0])
        {
            if (pClasses[0]->m_pInternal[1])
            {
                RETURN_ERROR(1, InitExtraClassData, LT_ERROR);
            }
            else
            {
                pClassMgr->m_ClassIndex = 1;
            }
        }
        else
        {
            pClassMgr->m_ClassIndex = 0;
        }

        pClassMgr->m_hClassNameHash = hs_CreateHashTable(50, HASH_RAW);

        LT_MEM_TRACK_ALLOC(pClassMgr->m_ClassDatas = new CClassData[nClasses],LT_MEM_TYPE_MISC);
        pClassMgr->m_nClassDatas = nClasses;

        memset(pClassMgr->m_ClassDatas, 0, sizeof(CClassData)*nClasses);

 
        for (i=0; i < nClasses; i++)
        {
            pClass = pClasses[i];
            
            pClassData = &pClassMgr->m_ClassDatas[i];
            
            LT_MEM_TRACK_ALLOC(sb_Init2(&pClassData->m_ObjectBank, pClass->m_ClassObjectSize, 1, 1), LT_MEM_TYPE_MISC);      
            pClassData->m_ClassID = (uint16)i;
            pClassData->m_pClass = pClass;
            pClass->m_pInternal[pClassMgr->m_ClassIndex] = pClassData;

            hElement = hs_AddElement(pClassMgr->m_hClassNameHash, pClass->m_ClassName, strlen(pClass->m_ClassName));
            if (!hElement)
                RETURN_ERROR(1, InitExtraClassData, LT_ERROR);

            hs_SetElementUserData(hElement, pClassData);
        }
    }

    return LT_ERROR;
}


static void SetupClassNullFunctions(ClassDef *pClass, uint32 offset)
{
    if (FUNCTION_POINTER(pClass, offset) == LTNULL)
    {
        if (pClass->m_ParentClass)
        {
            SetupClassNullFunctions(pClass->m_ParentClass, offset);
            FUNCTION_POINTER(pClass, offset) = FUNCTION_POINTER(pClass->m_ParentClass, offset);
        }
    }
}


static void SetupClassFunctions(CClassMgr *pClassMgr)
{
    ClassDef *pClass, **pClasses;
    int i, nClasses;

 
    pClasses = cb_GetClassDefs(&pClassMgr->m_ClassModule);
    nClasses = cb_GetNumClassDefs(&pClassMgr->m_ClassModule);
    
    // Make any LTNULL functions go to their parent class.
    for (i=0; i < nClasses; i++)
    {
        pClass = pClasses[i];


        SetupClassNullFunctions(pClass, (char*)&pClass->m_ConstructFn - (char*)pClass);
        SetupClassNullFunctions(pClass, (char*)&pClass->m_DestructFn - (char*)pClass);
    }
}


static LTRESULT CreateStaticObjects(CClassMgr *pClassMgr)
{
    int i;
    ObjectCreateStruct theStruct;
    LPBASECLASS pObject;
    ClassDef *pClass;
    LTRESULT dResult;

    for (i=0; i < pClassMgr->m_nClassDatas; i++)
    {
        theStruct.Clear();

        pClass = pClassMgr->m_ClassDatas[i].m_pClass;

 
        if (pClass->m_ClassFlags & CF_STATIC)
        {       
            pObject = sm_AllocateObjectOfClass(pClass);
            dResult = sm_AddObjectToWorld(pObject, 
                pClass, &theStruct, INVALID_OBJECTID, 
                OBJECTCREATED_NORMAL, 
                &pClassMgr->m_ClassDatas[i].m_pStaticObject);
        }
    }


    return LT_OK;
}




// ----------------------------------------------------------------------- //
// Exposed functions.
// ----------------------------------------------------------------------- //

LTRESULT LoadServerBinaries(CClassMgr *pClassMgr)
{
    ClassBindModule *pModule;
    LTRESULT dResult;


    ASSERT(!pClassMgr->m_hServerResourceModule);


    //Load the server module.
    if ((dResult = dsi_LoadServerObjects(pClassMgr)) != LT_OK) {
        return dResult;
    }

    //tell the server shell to do initialization.
    if ((dResult = i_server_shell->OnServerInitialized()) != LT_OK) {
        return dResult;
    }

 
    InitExtraClassData(pClassMgr);


    pModule = g_pServerMgr->GetClassModule();

    // Hook anything pointing to BaseClass functions.
    pClassMgr->m_pBaseClass = cb_FindClass(pModule, "BaseClass");
    if (!pClassMgr->m_pBaseClass)
    {
        sm_SetupError(LT_MISSINGCLASS, "%s", "BaseClass");
        RETURN_ERROR_PARAM(1, LoadServerBinaries, LT_MISSINGCLASS, "BaseClass");
    }

    SetupClassFunctions(pClassMgr);


    //make sure IServerShell is instantiated.
    if (i_server_shell == NULL) {
        sm_SetupError(LT_CANTCREATESERVERSHELL, "");
        RETURN_ERROR(1, LoadServerBinaries, LT_ERROR);
    }


 
    LTRESULT ret = CreateStaticObjects(pClassMgr);


    return ret;
}


//----------------------------------------------------------
//
// MODULE  : Movement.cpp
//
// PURPOSE : CMovement class
//
// CREATED : 9/23/97
//
//----------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include "movement.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "clientdebugline.h"
#include "objectutilities.h"

#include "windows.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD CMovement::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
			Update();
			break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CMovement::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::Init
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DBOOL CMovement::Init(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hObject || !pServerDE) return DFALSE;

	m_hObject = hObject;

	VEC_INIT(m_vLastPos);

    // See if it's a player object
	if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hObject), pServerDE->GetClass("CBaseCharacter"))) 
	{
		m_pOwner = (CBaseCharacter*)pServerDE->HandleToObject(hObject);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::Update
//
//	PURPOSE:	Update the object
//
// ----------------------------------------------------------------------- //

void CMovement::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

    // Save our last position...
	VEC_COPY(m_vLastPos,m_vPos);
	pServerDE->GetObjectPos(m_hObject, &m_vPos);

	// Retrieve object vectors for current frame..
	pServerDE->GetObjectRotation(m_hObject, &m_rRot);
	pServerDE->GetRotationVectors(&m_rRot, &m_vUp, &m_vRight, &m_vForward);

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::CalculatePath
//
//	PURPOSE:	Precalculate path to vDestPos
//
// ----------------------------------------------------------------------- //

DBOOL CMovement::CalculatePath(DVector vDestPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DVector vDims, vTest;
	DRotation rRot;

	//sanity check to make sure vDestPos is valid
	VEC_INIT(vTest);
	if(VEC_DIST(vTest, vDestPos) <= 0)
		return DFALSE;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;

//	LARGE_INTEGER start;
//	QueryPerformanceCounter(&start);

	//clear the path list out
	Term();

	//get the parent's dims
	pServerDE->GetObjectDims(m_hObject,&vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	if(!ClearToPoint(m_vPos, vDestPos,vDims, &IInfo))
	{
		VEC_ADDSCALED(IInfo.m_Point,IInfo.m_Point, IInfo.m_Plane.m_Normal, fDim - 0.1f)

		AddPosToPathList(IInfo.m_Point);

		//align a test rotation to the obstacles normal and retrieve the rotation vectors
		VEC_MULSCALAR(IInfo.m_Plane.m_Normal, IInfo.m_Plane.m_Normal, -1.0f);
		pServerDE->AlignRotation(&rRot, &(IInfo.m_Plane.m_Normal), &m_vUp);

		DVector vTurnPoint = FindShortestTurn(IInfo.m_Point, &rRot, fDim);

		if(VEC_DIST(vTurnPoint, IInfo.m_Point) <= 0.0f)
			return DFALSE;

		AddPosToPathList(vTurnPoint);

		DVector vU,vR,vF;
		pServerDE->GetRotationVectors(&rRot,&vU,&vR,&vF);

		vTurnPoint = FindTurn(vTurnPoint, vF, IInfo.m_Plane.m_Normal, fDim, fDim);
		
		if(VEC_DIST(vTurnPoint, vDestPos) <= 0.0f)
			return DFALSE;

		if(ClearToPoint(vTurnPoint, vDestPos, vDims, &IInfo))
		{
			AddPosToPathList(vTurnPoint);
			AddPosToPathList(vDestPos);

//			ConsolidatePath();
		}
		else
			return DFALSE;
	}
	else
		AddPosToPathList(vDestPos);

/*	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	pServerDE->DebugOut("Shortest Path Computed: %u ticks\r\n", (unsigned long)(end.QuadPart - start.QuadPart));
*/
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::FindPosAroundObj
//
//	PURPOSE:	Find nearest point on other side of object in vDir
//
// ----------------------------------------------------------------------- //

DVector CMovement::FindPosAroundObj(DVector vStart, DVector vDir)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return vStart;

	DVector vDims, vPoint, vColor;

	m_nWidthPoints = 10;

	//get the parent's dims
	pServerDE->GetObjectDims(m_hObject,&vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	VEC_ADDSCALED(vPoint, vStart, vDir, fDim);

	while(m_nWidthPoints)
	{
		if(pServerDE->GetPointShade(&vPoint,&vColor))
		{
			return vPoint;	
		}
		else
		{
			VEC_ADDSCALED(vPoint, vPoint, vDir, fDim);
		}

		m_nWidthPoints--;
	}

	return vStart;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::ClearToPoint
//
//	PURPOSE:	check if it is clear and wide enough to destination
//
// ----------------------------------------------------------------------- //

DBOOL CMovement::ClearToPoint(DVector vStart, DVector vDestPos, DVector vDims, IntersectInfo* IInfo)
{
	GlobalFilterFnData globalFilterFnData;
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	IntersectQuery IQuery;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GlobalFilterFn;
	globalFilterFnData.m_dwFlags = IGNORE_CHARACTER;
	globalFilterFnData.m_nIgnoreObjects = 1;
	globalFilterFnData.m_hIgnoreObjects = &m_hObject;
	IQuery.m_pUserData = &globalFilterFnData;	

	DVector vDir;
	DRotation rRot;

	VEC_SUB(vDir, vDestPos, vStart);
	VEC_NORM(vDir);

	pServerDE->AlignRotation(&rRot, &vDir, &m_vUp);

	DVector vU,vR,vF;
	pServerDE->GetRotationVectors(&rRot,&vU,&vR,&vF);

	//check the right side
	VEC_ADDSCALED(IQuery.m_From, vStart, vR, fDim);
	VEC_ADDSCALED(IQuery.m_To, vDestPos, vR, fDim);

	if(pServerDE->IntersectSegment(&IQuery, IInfo))
	{
		return DFALSE;
	}

	//check the left side
	VEC_ADDSCALED(IQuery.m_From, vStart, vR, (fDim * -1.0f));
	VEC_ADDSCALED(IQuery.m_To,vDestPos, vR, (fDim * -1.0f));

	if(pServerDE->IntersectSegment(&IQuery, IInfo))
	{
		return DFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::FindShortestTurn
//
//	PURPOSE:	find pos closest to turn around
//
// ----------------------------------------------------------------------- //

DVector CMovement::FindShortestTurn(DVector vStart, DRotation* prRot, DFLOAT fMoveLen)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return vStart;

	DVector vDir, vEnd, vCurPos;
	DVector vUp,vRight,vForward,vLeft;

	VEC_COPY(vCurPos, vStart);

	IntersectQuery IQuery;
	IntersectInfo ii;

	DFLOAT fLeftDist = 0.0f, fRightDist = 0.0f, fMaxDist = 0.0f;

	pServerDE->GetRotationVectors(prRot,&vUp,&vRight,&vForward);
	VEC_MULSCALAR(vLeft,vRight,-1.0f);		//create the left rotation vector

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	//get the farthest left we could travel
	VEC_COPY(IQuery.m_From,vCurPos);
	VEC_COPY(IQuery.m_Direction,vLeft);

	if(pServerDE->CastRay(&IQuery,&ii))
	{
		fMaxDist = fLeftDist = VEC_DIST(vCurPos,ii.m_Point);
	}

	//now get the farthest right
	VEC_COPY(IQuery.m_Direction,vRight);

	if(pServerDE->CastRay(&IQuery,&ii))
	{
		fRightDist = VEC_DIST(vCurPos,ii.m_Point);

		if(fRightDist > fMaxDist)
			fMaxDist = fRightDist;
	}

	//travel the obstacle in both directions looking for a clearing
	VEC_INIT(vDir);

	VEC_MULSCALAR(vEnd,vForward,fMoveLen + 5.0f);

	for(float fWidth = fMoveLen; !(fWidth >= fRightDist && fWidth >= fLeftDist); fWidth += fMoveLen)
	{
		//Check the right side
		if(fWidth < fRightDist)
		{
			VEC_ADDSCALED(IQuery.m_From,vCurPos,vRight,fWidth);
			VEC_ADD(IQuery.m_To,IQuery.m_From,vEnd);

			if(!pServerDE->IntersectSegment(&IQuery,&ii))
			{
				VEC_ADDSCALED(IQuery.m_From,IQuery.m_From,vRight,fWidth/2);
				pServerDE->AlignRotation(prRot, &vLeft, &m_vUp);
				return IQuery.m_From;
			}
		}

		//Check the left side
		if(fWidth < fLeftDist)
		{
			VEC_ADDSCALED(IQuery.m_From,vCurPos,vLeft,fWidth);
			VEC_ADD(IQuery.m_To,IQuery.m_From,vEnd);

			if(!pServerDE->IntersectSegment(&IQuery,&ii))
			{
				VEC_ADDSCALED(IQuery.m_From,IQuery.m_From,vLeft,fWidth/2);
				pServerDE->AlignRotation(prRot, &vRight, &m_vUp);
				return IQuery.m_From;
			}

		}
	}

	return vStart;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::FindTurn
//
//	PURPOSE:	find pos to turn around
//
// ----------------------------------------------------------------------- //

DVector CMovement::FindTurn(DVector vStart, DVector vTestDir, DVector vMoveDir, DFLOAT fMoveLen, 
							DFLOAT fTestLen)
{
	DVector vFinal,vCurPos;
	DBOOL	bStop = DTRUE;
	DFLOAT fMaxDist = 0.0f;

	VEC_INIT(vFinal);
	VEC_COPY(vCurPos, vStart);

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return vFinal;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	VEC_COPY(IQuery.m_From,vStart);
	VEC_COPY(IQuery.m_Direction,vMoveDir);

	//find maximum searchable distance in vMoveDir
	if(pServerDE->CastRay(&IQuery,&IInfo))
	{
		fMaxDist = VEC_DIST(vStart,IInfo.m_Point);
	}

	//loop til we find a spot to turn
	for(float fDist = 0.0f; !((fDist + fMoveLen) >= fMaxDist); fDist += fMoveLen)
	{
		VEC_ADDSCALED(vCurPos, vCurPos, vMoveDir, fMoveLen);

		VEC_COPY(IQuery.m_From,vCurPos);
		VEC_ADDSCALED(IQuery.m_To,vCurPos, vTestDir, fTestLen);

		if(!pServerDE->IntersectSegment(&IQuery, &IInfo))
		{
			VEC_ADDSCALED(vFinal, vCurPos, vMoveDir, fMoveLen);
			return vFinal;
		}
	}

	if(m_nNumPoints >= 10 || (VEC_DIST(vCurPos,vStart) <= 0.0))
		return vCurPos;

	//we can't turn here so we add to list and keep searching in new direction
	AddPosToPathList(vCurPos);

	DVector vNewMoveDir;
	VEC_MULSCALAR(vNewMoveDir, vTestDir, -1.0f);

	return FindTurn(vCurPos, vMoveDir, vNewMoveDir, fMoveLen, (fMaxDist - fDist) + 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::AddPosToPathList
//
//	PURPOSE:	add point to path list
//
// ----------------------------------------------------------------------- //

void CMovement::AddPosToPathList(DVector vPathpoint)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector* vPoint = new DVector;
	VEC_COPY(*vPoint, vPathpoint);

	DLink* pLink = new DLink;

	dl_AddTail(&m_PathList, pLink, (void*)vPoint);

	m_nNumPoints++;

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::ConsolidatePath
//
//	PURPOSE:	add point to path list
//
// ----------------------------------------------------------------------- //

void CMovement::ConsolidatePath()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	//check first from our pos to second path point

	DLink* pLink1 = m_PathList.m_Head.m_pNext;

	// add a little sanity checking (gk 9/2)
	if (pLink1->m_pData && pLink1->m_pNext->m_pData)
	{

		VEC_COPY(IQuery.m_From, m_vPos);
		VEC_COPY(IQuery.m_To, *((DVector*)pLink1->m_pNext->m_pData));

		if(!pServerDE->IntersectSegment(&IQuery, &IInfo))
		{
			dl_RemoveAt(&m_PathList,pLink1);
			delete pLink1;
		}
	}

	//loop through pathpoint and pathpoint + 2
	pLink1 = m_PathList.m_Head.m_pNext;

	if (pLink1->m_pData && pLink1->m_pNext->m_pData)
	{
		DLink* pLink2 = pLink1->m_pNext->m_pNext;
		
		while(pLink1->m_pData && pLink2->m_pData)
		{
			VEC_COPY(IQuery.m_From, *((DVector*)pLink1->m_pData));
			VEC_COPY(IQuery.m_To, *((DVector*)pLink2->m_pData));

			if(!pServerDE->IntersectSegment(&IQuery, &IInfo))
			{
				DLink *pObLink = pLink1->m_pNext;
				dl_RemoveAt(&m_PathList,pObLink);
				delete pObLink;
				pLink2 = pLink2->m_pNext;
			}
			else
			{
				pLink1 = pLink1->m_pNext;
				pLink2 = pLink2->m_pNext;
			}
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::DebugDrawPath
//
//	PURPOSE:	debug draw path
//
// ----------------------------------------------------------------------- //

void CMovement::DebugDrawPath()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	HCLASS hClass = pServerDE->GetClass( "CClientDebugLine" );
	if( !hClass )	return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY(ocStruct.m_Pos, m_vPos);
	ROT_COPY(ocStruct.m_Rotation, m_rRot);

	DLink* pLink1 = m_PathList.m_Head.m_pNext;
	DLink* pLink2 = pLink1->m_pNext;

	while(pLink1->m_pData && pLink2->m_pData)
	{
		CClientDebugLine* pDebugline = (CClientDebugLine*)pServerDE->CreateObject(hClass, &ocStruct);

		pDebugline->Setup((DVector*)pLink1->m_pData, (DVector*)pLink2->m_pData);

		pLink1 = pLink1->m_pNext;
		pLink2 = pLink2->m_pNext;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CMovement::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToMessageRotation(hWrite, &m_rRot);
	pServerDE->WriteToMessageVector(hWrite, &m_vRight);
	pServerDE->WriteToMessageVector(hWrite, &m_vUp);
	pServerDE->WriteToMessageVector(hWrite, &m_vForward);
	pServerDE->WriteToMessageVector(hWrite, &m_vPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);

	pServerDE->WriteToMessageDWord(hWrite, m_PathList.m_nElements);

	DLink *pLink = m_PathList.m_Head.m_pNext;

	for (unsigned long i=0; i < m_PathList.m_nElements; i++)
	{
		pServerDE->WriteToMessageVector(hWrite, (DVector*)pLink->m_pData);
		pLink = pLink->m_pNext;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMovement::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CMovement::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	pServerDE->ReadFromMessageRotation(hRead, &m_rRot);
	pServerDE->ReadFromMessageVector(hRead, &m_vRight);
	pServerDE->ReadFromMessageVector(hRead, &m_vUp);
	pServerDE->ReadFromMessageVector(hRead, &m_vForward);
	pServerDE->ReadFromMessageVector(hRead, &m_vPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);

	int nNumPoints = pServerDE->ReadFromMessageDWord(hRead);

	for (int i=0; i < nNumPoints; i++)
	{
		DVector vPos;
		pServerDE->ReadFromMessageVector(hRead, &vPos);
		AddPosToPathList(vPos);
	}
}

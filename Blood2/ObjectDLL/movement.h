
//----------------------------------------------------------
//
// MODULE  : Movement.h
//
// PURPOSE : CMovement class
//
// CREATED : 8/07/98
//
//----------------------------------------------------------

#ifndef __MOVEMENT_H
#define __MOVEMENT_H

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"

class CBaseCharacter;

class CMovement : public Aggregate
{
	public :

		CMovement()
		{
			m_pOwner = DNULL;
			m_hObject = DNULL;

			m_PathList.m_Head.m_pData = DNULL;
			dl_InitList(&m_PathList);

			m_nNumPoints = 0;
		}

		virtual ~CMovement()
		{
			Term();
		}

		DBOOL	Init(HOBJECT hObject);
		void	Term()
		{
			while(m_PathList.m_nElements)
			{
				DLink* pLink = m_PathList.m_Head.m_pNext;
				dl_RemoveAt(&m_PathList, pLink);
				delete pLink;
			}

			m_nNumPoints = 0;
		}

		DVector		GetPos()				{return m_vPos;}
		DVector		GetLastPos()			{return m_vLastPos;}
		DRotation	GetRotation()			{return m_rRot;}
		DVector		GetRightVector()		{return m_vRight;}
		DVector		GetUpVector()			{return m_vUp;}
		DVector		GetForwardVector()		{return m_vForward;}
		DVector*	GetNextPathPoint()		{return (DVector*)m_PathList.m_Head.m_pNext->m_pData;}

		DBOOL MoveToNextPathPoint()
		{
			DLink* pLink = m_PathList.m_Head.m_pNext;

			dl_RemoveAt(&m_PathList, pLink);
			delete pLink;

			if(m_PathList.m_nElements && m_PathList.m_Head.m_pNext->m_pData)
				return DTRUE;
			else
				return DFALSE;

		}

		DBOOL		CalculatePath(DVector vDestPos);
		DVector		FindPosAroundObj(DVector vStart, DVector vDir);
		DBOOL		ClearToPoint(DVector vStart, DVector vDestPos, DVector vDims, IntersectInfo* IInfo);
		DVector		FindShortestTurn(DVector vStart, DRotation* prRot, DFLOAT fMoveLen);
		DVector		FindTurn(DVector vStart, DVector vTestDir, DVector vMoveDir, DFLOAT fMoveLen, DFLOAT fTestLen);
		void		AddPosToPathList(DVector vPathpoint);
		void		ConsolidatePath();
		void		DebugDrawPath();

	protected:

		DDWORD	EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void	Update();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	private:

		// pointer to the owner's object (if it's a base character)
		CBaseCharacter*	m_pOwner;

		// @cmember the object I am associated with
		HOBJECT	m_hObject; 

		// Positions
		DRotation	m_rRot;						// Object's rotation
		DVector		m_vRight;					// Object's right vector
		DVector		m_vUp;						// Object's up vector
		DVector		m_vForward;				    // Object's forward
		
		DVector		m_vPos;						// Object's current position
		DVector		m_vLastPos;				    // Object's last position

		DList		m_PathList;
		int			m_nNumPoints;
		int			m_nWidthPoints;
};

#endif // __MOVEMENT_H
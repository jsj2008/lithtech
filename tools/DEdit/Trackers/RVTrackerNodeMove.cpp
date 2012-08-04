//////////////////////////////////////////////////////////////////////
// RVTrackerNodeMove.h - Implementation for the node movement tracker 

#include "bdefs.h"
#include "rvtrackernodemove.h"
#include "eventnames.h"

CRVTrackerNodeMove::CRVTrackerNodeMove(LPCTSTR pName, CRegionView *pView, int iFlags) :
	CRegionViewTracker(pName, pView),
	m_vStartVec(0.0f, 0.0f, 0.0f),
	m_vTotalMoveOffset(0.0f, 0.0f, 0.0f),
	m_vMoveSnapAxis(0.0f, 0.0f, 0.0f)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= FALSE;

	m_bLockAxis			= FALSE;
	m_pLockAxisTracker	= NULL;

	m_bClone			= (iFlags & FLAG_CLONE) != 0;
	m_pCloneTracker		= NULL;

	m_bSnap				= (iFlags & FLAG_SNAP) != 0;
	m_bHandle			= (iFlags & FLAG_HANDLE) != 0;
	m_bPerp				= (iFlags & FLAG_PERP) != 0;
}

CRVTrackerNodeMove::~CRVTrackerNodeMove()
{
	delete m_pLockAxisTracker;
	delete m_pCloneTracker;
}

void CRVTrackerNodeMove::FlushTracker()
{
	delete m_pLockAxisTracker;
	m_pLockAxisTracker = new CRegionViewTracker(UIE_NODE_MOVE_LOCK_AXIS, m_pView);
	SetupChildTracker(m_pLockAxisTracker);

	delete m_pCloneTracker;
	m_pCloneTracker = new CRegionViewTracker(UIE_NODE_MOVE_CLONE, m_pView);
	SetupChildTracker(m_pCloneTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerNodeMove::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}

// Watch for the shift key to be pressed
void CRVTrackerNodeMove::WatchEvent(const CUIEvent &cEvent)
{
	if (m_pLockAxisTracker)
	{
		m_pLockAxisTracker->ProcessEvent(cEvent);
		m_bLockAxis = m_pLockAxisTracker->GetActive();
	}
	if (m_pCloneTracker)
	{
		m_pCloneTracker->ProcessEvent(cEvent);
		m_bClone = m_pCloneTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerNodeMove::OnStart()
{
	// Only start if we're in brush or objectmode
	if ((m_pView->GetEditMode() != BRUSH_EDITMODE) && 
		(m_pView->GetEditMode() != OBJECT_EDITMODE))
		return FALSE;

	// Make sure something is selected
	if (!m_pView->GetRegion()->m_Selections.GetSize())
		return FALSE;

	// If in handle mode, make sure we're clicking on a move handle
	if (m_bHandle)
	{
		// Any handle at all?
		int iCurHandle = m_pView->GetCurrentMouseOverHandle();
		if (iCurHandle == -1)
			return FALSE;

		// A movement handle?
		CVector vDummy;
		if (m_pView->GetHandleInfo(iCurHandle, NULL, vDummy, vDummy))
			return FALSE;
	}

	// Make a clone if needed  
	if (m_bClone)
	{
		m_pView->GetRegionDoc()->Clone();
	}

	// Default to successful start...
	BOOL bStart = TRUE;
	
	CVector vClosest;

	if(m_bSnap && m_pView->GetClosestPoint(m_cCurPt, vClosest, TRUE ))
	{
		// Make the closest point our starting position...
		m_vStartVec = vClosest;

		// Snap to grid...
		CEditGrid *pGrid = &m_pView->EditGrid();
		m_vStartVec -= pGrid->Forward() * (pGrid->Forward().Dot(m_vStartVec - pGrid->Pos()));
	}
	else
	{
		if (!m_bSnap)
		{
			// Reset the move offset
			m_vTotalMoveOffset = CVector(0.0f, 0.0f, 0.0f);
			m_vMoveSnapAxis = CVector(1.0f, 1.0f, 1.0f);
		}

		// Find another point...
		bStart = m_pView->GetVertexFromPoint(m_cCurPt, m_vStartVec);
	}

	if( bStart )
		m_pView->GetRegionDoc()->SetupUndoForSelections();
	
	return bStart;
}

BOOL CRVTrackerNodeMove::OnUpdate(const CUIEvent &cEvent)
{
	// Only update on idle
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	LTVector newVert, moveOffset, translateAmount;
	DWORD i, j;
	CWorldNode *pNode;
	CEditBrush *pBrush;
	CEditPoly *pPoly;

	// If the mouse is outside of the view, then autoscroll the view
	DoAutoScroll();

	// Use the current mouse position and use the delta from the last to update the 
	// position of the node in 3d views...
	if( m_pView->GetVertexFromPoint(m_cCurPt, newVert))
	{
		moveOffset = newVert - m_vStartVec;

		// Snap the movement to the current move axis (may be all axis)
		moveOffset.x *= m_vMoveSnapAxis.x;
		moveOffset.y *= m_vMoveSnapAxis.y;
		moveOffset.z *= m_vMoveSnapAxis.z;

		// If the shift key is pressed, then snap the object to horizontal or vertical movement if
		// that hasn't been done already.		
		if (m_bLockAxis)
		{
			// Make sure that there isn't currently a move axis defined
			if (m_vMoveSnapAxis == CVector(1.0f, 1.0f, 1.0f))
			{
				// Create an absolute vector so that the magnitudes can be compared
				CVector vAbsolute(fabs(m_vTotalMoveOffset.x), fabs(m_vTotalMoveOffset.y), fabs(m_vTotalMoveOffset.z));

				// Check to see if we should snap to the x axis
				if (vAbsolute.x > vAbsolute.y && vAbsolute.x > vAbsolute.z)
				{
					// Cancel the movement along the y and z axis
					moveOffset.y=(-1)*m_vTotalMoveOffset.y;
					moveOffset.z=(-1)*m_vTotalMoveOffset.z;

					m_vMoveSnapAxis=CVector(1.0f, 0.0f, 0.0f);
				}

				// Check to see if we should snap to the y axis
				if (vAbsolute.y > vAbsolute.x && vAbsolute.y > vAbsolute.z)
				{
					// Cancel the movement along the y and z axis
					moveOffset.x=(-1)*m_vTotalMoveOffset.x;
					moveOffset.z=(-1)*m_vTotalMoveOffset.z;

					m_vMoveSnapAxis=CVector(0.0f, 1.0f, 0.0f);
				}

				// Check to see if we should snap to the z axis
				if (vAbsolute.z > vAbsolute.x && vAbsolute.z > vAbsolute.y)
				{
					// Cancel the movement along the y and z axis
					moveOffset.x=(-1)*m_vTotalMoveOffset.x;
					moveOffset.y=(-1)*m_vTotalMoveOffset.y;

					m_vMoveSnapAxis=CVector(0.0f, 0.0f, 1.0f);
				}
			}
		}
		else
		{
			m_vMoveSnapAxis=CVector(1.0f, 1.0f, 1.0f);
		}

		// Update the total move offset.  This is used to snap the nodes when the shift key is pressed.
		m_vTotalMoveOffset+=moveOffset;

		// Go through all the selected nodes and move them...
		for( i=0; i < m_pView->GetRegion()->m_Selections; i++ )
		{
			pNode = m_pView->GetRegion()->m_Selections[i];

			// Handle movement for Brush nodes...
			if(pNode->GetType() == Node_Brush)
			{
				// Get the brush pointer...
				pBrush = pNode->AsBrush();

				// Check for perpendicular movement...
				if(m_bPerp)
				{
					translateAmount = -(m_pView->EditGrid().Forward() * (CReal)(m_cCurPt.y - m_cLastPt.y));
				}
				// planar movement...
				else
				{
					translateAmount = moveOffset;
				}

				for( j=0; j < pBrush->m_Points; j++ )
					pBrush->m_Points[j] += translateAmount;

				// Update the texture space on the polies that are stuck.
				for(j=0; j < pBrush->m_Polies; j++)
				{
					pPoly = pBrush->m_Polies[j];
				
					for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
					{
						CTexturedPlane& Texture = pPoly->GetTexture(nCurrTex);

						pPoly->SetTextureSpace(nCurrTex, Texture.GetO() + translateAmount, Texture.GetP(), Texture.GetQ());
					}
				}

				m_pView->GetRegion()->UpdateBrush(pBrush);
			}
			// Handle movement for object nodes...
			else if((pNode->GetType() == Node_Object) || 
					(pNode->GetType() == Node_PrefabRef))
			{
				// Check if perpendicular movement...
				if(m_bPerp)
				{
					LTVector vCurPos = pNode->GetPos();
					vCurPos -= m_pView->EditGrid().Forward() * (CReal)(m_cCurPt.y - m_cLastPt.y);
					pNode->SetPos(vCurPos);
				}
				// planar movement...
				else
				{
					pNode->SetPos(pNode->GetPos() + moveOffset);
				}
			}
		}

		// Update starting vertex for next pass...
		m_vStartVec = newVert;
	}

	// If user wants all the views updated, then do it...
	if( GetApp()->m_bFullUpdate )
	{
		m_pView->GetDocument()->UpdateAllViews(m_pView);
		m_pView->DrawRect();
	}

	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerNodeMove::OnEnd()
{
	CEditRegion *pRegion = m_pView->GetRegion();

	// Recalculate plane equations for all planes...
	pRegion->UpdatePlanes();
	pRegion->CleanupGeometry( );

	// Resize the box encompassing all the selections...
	m_pView->GetRegionDoc()->UpdateSelectionBox();

	// Update properities dialog...
	m_pView->GetRegionDoc()->SetupPropertiesDlg( FALSE );

	return TRUE;
}


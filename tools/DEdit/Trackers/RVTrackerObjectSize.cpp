//////////////////////////////////////////////////////////////////////
// RVTrackerObjectSize.h - Implementation for the object sizing tracker 

#include "bdefs.h"
#include "rvtrackerobjectsize.h"

CRVTrackerObjectSize::CRVTrackerObjectSize(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = FALSE;
}

BOOL CRVTrackerObjectSize::OnStart()
{
	// Only start if we're in object mode
	if (m_pView->GetEditMode() != OBJECT_EDITMODE)
		return FALSE;

	// Don't start if they didn't hit a handle
	m_iCurHandle = m_pView->GetCurrentMouseOverHandle();
	if (m_iCurHandle == -1 )
		return FALSE;

	// Don't start if they didn't hit a sizing handle
	CVector vDummy;
	if (!m_pView->GetHandleInfo(m_iCurHandle, NULL, vDummy, m_vScaleOrigin))
		return FALSE;
					
	m_vStartVal = GetHandleCoordinate(m_iCurHandle);

	return TRUE;
}

BOOL CRVTrackerObjectSize::OnUpdate(const CUIEvent &cEvent)
{
	// Don't update the size if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	// Stay visible
	DoAutoScroll();

	CVector vNewVert;

	if (m_pView->GetVertexFromPoint(m_cCurPt, vNewVert))
		SetHandleCoordinate(m_iCurHandle, vNewVert);

	if (GetApp()->m_bFullUpdate)
	{
		m_pView->GetDocument()->UpdateAllViews(m_pView);
		// Update this view as well
		m_pView->DrawRect();
	}

	m_cLastPt = m_cCurPt;

	return TRUE;
}

BOOL CRVTrackerObjectSize::OnEnd()
{
	CVector		scaleAmt(1.0f, 1.0f, 1.0f);	
	DWORD		i, j;

	CVector		originVal = m_vScaleOrigin;
	CVector		startVal = m_vStartVal;
	CVector		endVal = GetHandleCoordinate(m_iCurHandle);

	CEditBrush	 *pBrush;
	CBaseEditObj *pObject;
	CVector		*pVec;
	CWorldNode	*pNode;

	CRegionDoc *pDoc = m_pView->GetRegionDoc();
	CEditRegion *pRegion = m_pView->GetRegion();


	pDoc->SetupUndoForSelections();

	// Get the amount to scale.
	if (startVal.x-originVal.x != 0.0f)
	{
		scaleAmt.x = (endVal.x - originVal.x) / (startVal.x - originVal.x);	
	}
	if (startVal.y-originVal.y != 0.0f)
	{
		scaleAmt.y = (endVal.y - originVal.y) / (startVal.y - originVal.y);	
	}
	if (startVal.z-originVal.z != 0.0f)
	{
		scaleAmt.z = (endVal.z - originVal.z) / (startVal.z - originVal.z);	
	}		

	// Don't allow scaling to a zero width
	if (!scaleAmt.x || !scaleAmt.y || !scaleAmt.z)
	{
		pDoc->UpdateSelectionBox();
		return TRUE;
	}

	// Determine if the polygon needs to be flipped
	BOOL bFlip=FALSE;
	if (scaleAmt.x < 0.0f)
	{	
		bFlip=!bFlip;
	}
	if (scaleAmt.y < 0.0f)
	{	
		bFlip=!bFlip;
	}
	if (scaleAmt.z < 0.0f)
	{	
		bFlip=!bFlip;
	}

	for( i=0; i < pRegion->m_Selections; i++ )
	{
		pNode = pRegion->m_Selections[i];

		if(pNode->GetType() == Node_Brush)
		{
			pBrush = pNode->AsBrush();

			if (bFlip)
			{
				pBrush->FlipNormals();
			}

			for( j=0; j < pBrush->m_Points; j++ )
			{
				pVec = &pBrush->m_Points[j];

				(*pVec) -= originVal;
				(*pVec).x *= scaleAmt.x;
				(*pVec).y *= scaleAmt.y;
				(*pVec).z *= scaleAmt.z;
				(*pVec) += originVal;
			}
		}
		else if((pNode->GetType() == Node_Object) || (pNode->GetType() == Node_PrefabRef))
		{
			LTVector vVec = pNode->GetPos();

			vVec -= originVal;
			vVec.x *= scaleAmt.x;
			vVec.y *= scaleAmt.y;
			vVec.z *= scaleAmt.z;
			vVec += originVal;
			pNode->SetPos(vVec);

			if (pNode->GetType() == Node_Object)
			{
				pObject = pNode->AsObject();
				if (pObject->GetNumDims() != 0) // has dims, so update the object
				{
					LTVector* dimV = pObject->GetDim(0);

					dimV->x *= scaleAmt.x;
					dimV->y *= scaleAmt.y;
					dimV->z *= scaleAmt.z;

					// update the brush properties as well

					char* tempchar;

					if (pObject->GetPropertyList() != NULL)
					{
						for (int j=0; j < pObject->GetPropertyList()->GetSize(); j++) 
						{
							if (strcmp("Dims", pObject->GetPropertyList()->GetAt(j)->GetName()) == 0) 
							{
								((CVectorProp*)pObject->GetPropertyList()->GetAt(j))->SetVector(*dimV);
								break;
							}
						}
					}
				}
			}
		}
	}

	// Recalculate plane equations for all planes...
	pRegion->UpdatePlanes();
	pRegion->CleanupGeometry( );

	pDoc->UpdateSelectionBox();

	// Update properties...
	pDoc->SetupPropertiesDlg( FALSE );

	//MessageBox(NULL, "Object resize completed", "Debug success", MB_OK);

	return TRUE;
}

void CRVTrackerObjectSize::OnCancel()
{
	m_pView->GetRegionDoc()->UpdateSelectionBox();
}

/************************************************************************/
// Returns a handle coordinate for a specific handle index
CVector CRVTrackerObjectSize::GetHandleCoordinate(int iHandle)
{	
	CVector vSelectionMax = m_pView->GetRegionDoc()->m_SelectionMax;
	CVector vSelectionMin = m_pView->GetRegionDoc()->m_SelectionMin;

	switch (iHandle)
	{
		case 0:		return CVector(vSelectionMax.x, 0, 0);		// X plane
		case 1:		return CVector(vSelectionMin.x, 0, 0);		// X plane
		case 2:		return CVector(0, vSelectionMax.y, 0);		// Y plane
		case 3:		return CVector(0, vSelectionMin.y, 0);		// Y plane
		case 4:		return CVector(0, 0, vSelectionMax.z);		// Z plane
		case 5:		return CVector(0, 0, vSelectionMin.z);		// Z plane

		// XY handles
		case 6:		return CVector(vSelectionMax.x, vSelectionMax.y, 0);
		case 7:		return CVector(vSelectionMin.x, vSelectionMax.y, 0);
		case 8:		return CVector(vSelectionMax.x, vSelectionMin.y, 0);
		case 9:		return CVector(vSelectionMin.x, vSelectionMin.y, 0);

		// XZ handles
		case 10:		return CVector(vSelectionMax.x, 0, vSelectionMax.z);
		case 11:		return CVector(vSelectionMin.x, 0, vSelectionMax.z);
		case 12:		return CVector(vSelectionMax.x, 0, vSelectionMin.z);
		case 13:		return CVector(vSelectionMin.x, 0, vSelectionMin.z);

		// YZ handles
		case 14:		return CVector(0, vSelectionMax.y, vSelectionMax.z);
		case 15:		return CVector(0, vSelectionMin.y, vSelectionMax.z);
		case 16:		return CVector(0, vSelectionMax.y, vSelectionMin.z);
		case 17:		return CVector(0, vSelectionMin.y, vSelectionMin.z);
		default:
			{
				ASSERT(FALSE);
				break;
			}
	}	
	
	ASSERT( FALSE );
	return CVector(0,0,0);
}

/************************************************************************/
// Sets a handle coordinate for a specific index
void CRVTrackerObjectSize::SetHandleCoordinate(int iHandle, CVector vCoord)
{
	CVector *pMax = &m_pView->GetRegionDoc()->m_SelectionMax;
	CVector *pMin = &m_pView->GetRegionDoc()->m_SelectionMin;

	switch (iHandle)
	{
		case 0: pMax->x=vCoord.x; break;	// X plane
		case 1: pMin->x=vCoord.x; break;	// X plane
		case 2: pMax->y=vCoord.y; break;	// Y plane
		case 3: pMin->y=vCoord.y; break;	// Y plane
		case 4: pMax->z=vCoord.z; break;	// Z plane
		case 5: pMin->z=vCoord.z; break;	// Z plane

		// XY handles
		case 6: pMax->x=vCoord.x; pMax->y=vCoord.y; break;
		case 7: pMin->x=vCoord.x; pMax->y=vCoord.y; break;
		case 8: pMax->x=vCoord.x; pMin->y=vCoord.y; break;
		case 9: pMin->x=vCoord.x; pMin->y=vCoord.y; break;
		
		// XZ handles
		case 10: pMax->x=vCoord.x; pMax->z=vCoord.z; break;
		case 11: pMin->x=vCoord.x; pMax->z=vCoord.z; break;
		case 12: pMax->x=vCoord.x; pMin->z=vCoord.z; break;
		case 13: pMin->x=vCoord.x; pMin->z=vCoord.z; break;

		// YZ handles
		case 14: pMax->y=vCoord.y; pMax->z=vCoord.z; break;
		case 15: pMin->y=vCoord.y; pMax->z=vCoord.z; break;
		case 16: pMax->y=vCoord.y; pMin->z=vCoord.z; break;
		case 17: pMin->y=vCoord.y; pMin->z=vCoord.z; break;				
		default:
			{
				ASSERT(FALSE);
				break;
			}
	}		
}


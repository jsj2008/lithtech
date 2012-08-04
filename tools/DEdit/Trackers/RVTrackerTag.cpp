//////////////////////////////////////////////////////////////////////
// RVTrackerTag.h - Implementation for the tagging tracker 

#include "bdefs.h"
#include "mainfrm.h"
#include "propertiesdlg.h"
#include "nodeview.h"
#include "rvtrackertag.h"
#include "optionsmisc.h"
#include "eventnames.h"

CRVTrackerTag::CRVTrackerTag(LPCTSTR pName, CRegionView *pView, int iFlags) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter		= FALSE;
	m_bAutoHide			= FALSE;

	m_bAdd				= FALSE;
	m_pAddTracker		= NULL;

	m_bInvert			= FALSE;
	m_pInvertTracker	= NULL;

	m_bObject	= !!(iFlags & FLAG_OBJECT);
	m_bBrush	= !!(iFlags & FLAG_BRUSH);
	m_bGeometry = !!(iFlags & FLAG_GEOMETRY);

	FlushTracker();
}

CRVTrackerTag::~CRVTrackerTag()
{
	delete m_pAddTracker;
	delete m_pInvertTracker;
}

void CRVTrackerTag::FlushTracker()
{
	delete m_pAddTracker;
	m_pAddTracker = new CRegionViewTracker(UIE_TAG_ADD, m_pView);
	SetupChildTracker(m_pAddTracker);

	delete m_pInvertTracker;
	m_pInvertTracker = new CRegionViewTracker(UIE_TAG_INVERT, m_pView);
	SetupChildTracker(m_pInvertTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerTag::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}


// Watch for the shift key to be pressed
void CRVTrackerTag::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pAddTracker)
	{
		m_pAddTracker->ProcessEvent(cEvent);
		m_bAdd = m_pAddTracker->GetActive();
	}
	if(m_pInvertTracker)
	{
		m_pInvertTracker->ProcessEvent(cEvent);
		m_bInvert = m_pInvertTracker->GetActive();
	}

	// Call the base class watch event
	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerTag::OnStart()
{
	// Only start if we're in the right mode for this tracker
	BOOL bModeOK = FALSE;
	int iEditMode = m_pView->GetEditMode();
	bModeOK |= (iEditMode == BRUSH_EDITMODE) && m_bBrush;
	bModeOK |= (iEditMode == OBJECT_EDITMODE) && m_bObject;
	bModeOK |= (iEditMode == GEOMETRY_EDITMODE) && m_bGeometry;
	if (!bModeOK)
		return FALSE;

	return TRUE;
}

BOOL CRVTrackerTag::OnUpdate(const CUIEvent &cEvent)
{
	// Only update on idle
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	DoAutoScroll();

	m_pView->m_TagDrawRect = m_cRect;
	m_pView->DrawRect(&m_cRect);

	m_cLastPt = m_cCurPt;

	return TRUE;
}

void CRVTrackerTag::SelectNode(CEditRegion *pRegion, CWorldNode *pNode, HTREEITEM hItem, ESelectMode eMode, BOOL bSelectInTree)
{
	//make sure that we aren't trying to select a frozen node
	if( pNode->IsFlagSet(NODEFLAG_FROZEN) )
	{
		return;				
	}

	// Only open the parent folder in the Node View if the options allow it
	if (!GetApp()->GetOptions().GetMiscOptions()->IsParentFolder())  bSelectInTree = false;

	switch (eMode)
	{
		case SELECT_OFF :
			// Unselect the node
			pRegion->UnselectNode(pNode);
			break;
		case SELECT_ON :
			// Select the node
			pRegion->SelectNode(pNode);
			// Select the node if desired
			if(bSelectInTree)
			{
				GetNodeView()->m_NodeViewTree.Select(hItem, TVGN_CARET);
			}
			break;
		case SELECT_INVERT :
			// Invert the selection
			SelectNode(pRegion, pNode, hItem, (pNode->IsFlagSet(NODEFLAG_SELECTED)) ? SELECT_OFF : SELECT_ON, bSelectInTree);
			break;
	}
}

// Note : To be perfectly honest, I got this code off of the internet and
//		am not 100% sure how it works...
static float GetRayLineDistance(CEditRay ray, CVector pt1, CVector pt2)
{
	CVector lineDirection(pt2 - pt1);
    CVector diff(ray.m_Pos - pt1);
    float A = ray.m_Dir.Dot(ray.m_Dir);
    float B = -ray.m_Dir.Dot(lineDirection);
    float C = lineDirection.Dot(lineDirection);
    float D = ray.m_Dir.Dot(diff);
    float E;  // -lineDirection.Dot(diff), defer until needed
    float F = diff.Dot(diff);
    float det = (float)fabs(A*C-B*B);  // A*C-B*B = |Cross(M0,M1)|^2 >= 0
	float s,t;

    if ( det >= 0.0001 )
    {
        // lines are not parallel
        E = -lineDirection.Dot(diff);
        float invDet = 1.0f/det;
        s = (B*E-C*D)*invDet;
        t = (B*D-A*E)*invDet;
        return (float)fabs(s*(A*s+B*t+2*D)+t*(B*s+C*t+2*E)+F);
    }
    else
    {
        // lines are parallel, select any closest pair of points
        s = -D/A;
        t = 0;
		return (float)fabs(D*s+F);
    }
}

static float GetPolyRayDistance(CPolyRef poly, CEditRay ray)
{
	CReal result = (CReal)MAX_CREAL, vertDist;
	DWORD i;
	for(i = 0; i < poly()->NumVerts(); i++)
	{
		// Calculate the distance from the ray to the current edge
		vertDist = GetRayLineDistance(ray, poly()->Pt(i), poly()->NextPt(i));
		if (vertDist < result)
			result = vertDist;
	}
	return result;
}

CPolyRef CRVTrackerTag::FindBestPoly(CEditRay ray)
{
	CPolyRef result, nextPoly;
	CReal rayDistance = 0.0f, bestDistance = (CReal)MAX_CREAL, thisDistance;

	// Shortcut out to the closest poly if it's a perspective view
	if (!m_pView->IsParallelViewType())
		return m_pView->CastRayAtPolies(ray, NULL, 0.0f);

	do
	{
		nextPoly = m_pView->CastRayAtPolies(ray, &thisDistance, rayDistance);
		if (nextPoly.IsValid())
		{
			// Save the ray distance for the next loop
			rayDistance = thisDistance;
			// Find out how far away this ray is from the poly's vertices
			thisDistance = GetPolyRayDistance(nextPoly, ray);
			// Save this one if it's closer..
			if (thisDistance < bestDistance)
			{
				bestDistance = thisDistance;
				result = nextPoly;
			}
		}
	} while (nextPoly.IsValid());
	return result;
}

BOOL CRVTrackerTag::OnEnd()
{
	int iEditMode = m_pView->GetEditMode();
	CEditBrush *pBrush;

	CEditRegion *pRegion = m_pView->GetRegion();

	ESelectMode eSelectMode;

	if (m_bInvert)
		eSelectMode = (m_bAdd) ? SELECT_OFF : SELECT_INVERT;
	else
		eSelectMode = SELECT_ON; 

	// Tagging in Geometry or Brush edit mode.
	if((iEditMode == GEOMETRY_EDITMODE) || (iEditMode == BRUSH_EDITMODE))
	{
		m_pView->TagPointsInRect(m_cRect);
		
		// If lParam == 1, then select all brushes with tagged points.
		if(iEditMode == BRUSH_EDITMODE)
		{
			CVertRefArray taggedVerts;
			DWORD			i;

			taggedVerts.GenCopyList(m_pView->TaggedVerts());

			if( taggedVerts )
			{
				// Check if this selection should clear old...
				if((GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION ) &&
					(!m_bAdd) && (!m_bInvert))
					pRegion->ClearSelections();

				for(i=0; i < taggedVerts; i++)
				{
					BOOL bLastItem = (i == taggedVerts - 1) ? TRUE : FALSE;

					pBrush = taggedVerts[i].m_pBrush;

					// If we're not toggling, it can be a lot faster by avoiding a search.
					if(eSelectMode == SELECT_ON || eSelectMode == SELECT_OFF)
					{
						if(!pBrush->IsFlagSet(NODEFLAG_SELECTED) || (eSelectMode == SELECT_OFF))
						{
							SelectNode(pRegion, pBrush->AsNode(), pBrush->GetItem(), eSelectMode, bLastItem);
						}
					}
					else
					{
						SelectNode(pRegion, pBrush->AsNode(), pBrush->GetItem(), eSelectMode, bLastItem);
						// Remove duplicate references to the same brush
						for (DWORD j = i + 1; j < taggedVerts; j++)
							if (taggedVerts[i].m_pBrush == taggedVerts[j].m_pBrush)
								taggedVerts.Remove(j--);
					}
				}
			}
			else 
			{
				// Find the closest brush
				CEditRay	ray;
				CPolyRef	polyRef;

				ray = m_pView->ViewDef()->MakeRayFromScreenPoint(m_cStartPt);
				polyRef = FindBestPoly(ray);
				if (polyRef.IsValid())
				{
					CEditBrush *pBrush = polyRef.m_pBrush;

					// Check if this selection should clear old...
					if((GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION ) &&
						(!m_bAdd) && (!m_bInvert))
						pRegion->ClearSelections();

					// Set the selection state
					SelectNode(pRegion, pBrush->AsNode(), pBrush->GetItem(), eSelectMode, TRUE);
				}
			}

			m_pView->GetRegionDoc()->NotifySelectionChange();
			m_pView->TaggedVerts().GenRemoveAll(); // Untag the vertices!
		}
		else
		{
			if ((!m_pView->TaggedVerts().GenGetSize()) && (m_pView->IPoly().IsValid()))
			{
				CPolyRefArray *pTaggedPolies = &(m_pView->TaggedPolies());

				DWORD i;
				BOOL bFound = FALSE;

				// First remove this polygon from the selection list
				for (i = 1; i < pTaggedPolies->GetSize(); i++)
				{
					if (pTaggedPolies->GetAt(i) == m_pView->IPoly())
					{
						pTaggedPolies->Remove(i);
						bFound = TRUE;
						break;
					}
				}

				if (!bFound)
					pTaggedPolies->Add(m_pView->IPoly());
			}
		}
	}
	// Tagging in object mode.
	else if (iEditMode == OBJECT_EDITMODE)
	{
		CMoArray<CWorldNode*> objects;
		DWORD i;

		m_pView->GetObjectsInRect(m_cRect, objects, TRUE);

		if (!objects.GetSize())
		{
			CWorldNode *obj = m_pView->GetClosestObject( m_cCurPt, FALSE, NULL, TRUE );
			if (obj)
				objects.Add(obj);
		}

		if (objects.GetSize())
		{
			// Check if this selection should clear old...
			if((GetMainFrame()->GetNodeSelectionMode() != MULTI_SELECTION ) &&
				(!m_bAdd) && (!m_bInvert))
				pRegion->ClearSelections();

			for( i = 0; i < objects.GetSize( ); i++ )
			{
				BOOL bLastItem = (i == objects.GetSize() - 1) ? TRUE : FALSE;
				SelectNode(pRegion, objects[i], objects[i]->GetItem(), eSelectMode, bLastItem);
			}

			m_pView->GetRegionDoc()->NotifySelectionChange();
		}
	}
	
	m_pView->m_TagDrawRect.SetRect( -1, -1, -1, -1 );

	m_pView->GetDocument()->UpdateAllViews(m_pView);
	m_pView->DrawRect(&m_cRect);

	return TRUE;
}

void CRVTrackerTag::OnCancel()
{
	m_pView->m_TagDrawRect.SetRect( -1, -1, -1, -1 );

	m_pView->GetDocument()->UpdateAllViews(m_pView);
	m_pView->DrawRect(&m_cRect);
}
//////////////////////////////////////////////////////////////////////
// RVTrackerTextMove.h - Implementation for the texture movement tracker 

#include "bdefs.h"
#include "rvtrackertextmove.h"
#include "eventnames.h"

#define MAX_TEXTURE_SIZE		512

CRVTrackerTextureMove::CRVTrackerTextureMove(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView),
	m_bSaveUndo(TRUE),
	m_bRotate(FALSE),
	m_bScale(FALSE),
	m_bStep(FALSE),
	m_pRotateTracker(NULL),
	m_pScaleTracker(NULL),
	m_pStepTracker(NULL)
{
	m_bAutoCenter	= FALSE;
	m_bAutoHide		= TRUE;

	//setup the new events
	FlushTracker();
}

CRVTrackerTextureMove::~CRVTrackerTextureMove()
{
	delete m_pRotateTracker;
	delete m_pScaleTracker;
	delete m_pStepTracker;
}

void CRVTrackerTextureMove::WatchEvent(const CUIEvent &cEvent)
{
	if(m_pRotateTracker)
	{
		m_pRotateTracker->ProcessEvent(cEvent);
		m_bRotate = m_pRotateTracker->GetActive();
	}
	if(m_pScaleTracker)
	{
		m_pScaleTracker->ProcessEvent(cEvent);
		m_bScale = m_pScaleTracker->GetActive();
	}
	if(m_pStepTracker)
	{
		m_pStepTracker->ProcessEvent(cEvent);
		m_bStep = m_pStepTracker->GetActive();
	}

	CRegionViewTracker::WatchEvent(cEvent);
}

BOOL CRVTrackerTextureMove::OnStart()
{
	// Only start if we're in geometry mode
	if (m_pView->GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	m_bSaveUndo = TRUE;

	//clear out any old polygons
	m_Textures.RemoveAll();

	//clear out the old nodes
	m_UndoNodes.RemoveAll();


	if(m_pView->GetEditMode() == GEOMETRY_EDITMODE)
	{
		if (!m_pView->IPoly().IsValid())
			return FALSE;

		//get the selected polygons
		CMoArray<CEditPoly*> SelectedPolies;
		SelectedPolies.SetSize(0);
		SelectedPolies.SetCacheSize(16);

		m_pView->GetSelectedPolies(SelectedPolies);

		//add this onto the list of selected polygons, also add them to the list of items that
		//need to be undone
		uint32 nCurrPoly;
		for(nCurrPoly = 0; nCurrPoly < SelectedPolies.GetSize(); nCurrPoly++)
		{
			m_Textures.Add(&SelectedPolies[nCurrPoly]->GetTexture(GetCurrTexture()));
			m_UndoNodes.Add((CWorldNode*)SelectedPolies[nCurrPoly]->m_pBrush);
			m_TextureNormals.Add(SelectedPolies[nCurrPoly]->GetTextureNormal());
		}
	}
	
	m_OriginalO.SetSize(m_Textures.GetSize());
	m_OriginalP.SetSize(m_Textures.GetSize());
	m_OriginalQ.SetSize(m_Textures.GetSize());

	for(uint32 nTex = 0; nTex < m_Textures.GetSize(); nTex++)
	{
		LTVector vO, vP, vQ;
		m_Textures[nTex]->GetTextureSpace(vO, vP, vQ);

		//save the O, P and Q
		m_OriginalO[nTex] = vO;
		m_OriginalP[nTex] = vP;
		m_OriginalQ[nTex] = vQ;
	}
	
	m_iTotalRotate = 0;
	m_iTotalScaleX = 0;
	m_iTotalScaleY = 0;
	m_iTotalOfsX = 0;
	m_iTotalOfsY = 0;

	return TRUE;
}

BOOL CRVTrackerTextureMove::OnUpdate(const CUIEvent &cEvent)
{
	// Only process on idle events
	if (cEvent.GetType() != UIEVENT_NONE)
		return TRUE;

	// Don't update the texture if it hasn't moved
	if (m_cCurPt == m_cLastPt)
		return TRUE;

	int32 iOfsX = m_cCurPt.x - m_cLastPt.x;
	int32 iOfsY = m_cCurPt.y - m_cLastPt.y;
	CenterCursor();

	if (m_bRotate)
	{
		m_iTotalRotate += iOfsX;
	}
	else if(m_bScale)
	{
		m_iTotalScaleX += iOfsX;
		m_iTotalScaleY += iOfsY;

		//the scales need to be clamped, so that they do not flip the P/Q axis (this
		//results in it shrinking after growing)
		m_iTotalScaleX = LTMAX(-64, m_iTotalScaleX);
		m_iTotalScaleY = LTMAX(-64, m_iTotalScaleY);
	}
	else
	{
		m_iTotalOfsX += iOfsX;
		m_iTotalOfsY += iOfsY;
	}

	//see if we need to set up the undo
	if(m_bSaveUndo)
	{
		for(uint32 nCurrUndo = 0; nCurrUndo < m_UndoNodes.GetSize(); nCurrUndo++)
		{
			m_pView->GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, m_UndoNodes[nCurrUndo]), TRUE);
		}
		m_bSaveUndo = FALSE;
	}

	for(uint32 nCurrPoly = 0; nCurrPoly < m_Textures.GetSize(); nCurrPoly++)
	{
		CTexturedPlane* pTexture = m_Textures[nCurrPoly];

		LTVector vNormal = m_TextureNormals[nCurrPoly];

		CVector cVectorO = pTexture->GetO();
		CVector cVectorP = pTexture->GetP();
		CVector cVectorQ = pTexture->GetQ();

		int32 iStepRotate	= (m_bStep) ? (m_iTotalRotate / 15) * 15 : m_iTotalRotate;
		int32 iStepScaleX	= (m_bStep) ? (m_iTotalScaleX / 16) * 16 : m_iTotalScaleX;
		int32 iStepScaleY	= (m_bStep) ? (m_iTotalScaleY / 16) * 16 : m_iTotalScaleY;
		int32 iStepOfsX		= (m_bStep) ? (m_iTotalOfsX / 16) * 16 : m_iTotalOfsX;
		int32 iStepOfsY		= (m_bStep) ? (m_iTotalOfsY / 16) * 16 : m_iTotalOfsY;

		CMatrix matRot;
		matRot.SetupRot( vNormal, (float)iStepRotate * MATH_PI / 180.0f);

		float fMag = m_OriginalP[nCurrPoly].Mag() * (((float)iStepScaleX) / 64.0f + 1.0f);
		cVectorP = m_OriginalP[nCurrPoly];
		cVectorP.Norm();
		cVectorP = matRot * cVectorP * fMag;
		
		fMag = m_OriginalQ[nCurrPoly].Mag() * (((float)iStepScaleY) / 64.0f + 1.0f);
		cVectorQ = m_OriginalQ[nCurrPoly];
		cVectorQ.Norm();
		cVectorQ = matRot * cVectorQ * fMag;
		

		cVectorO = m_OriginalO[nCurrPoly];
		
		if(iStepOfsX > 0)
			cVectorO += (m_OriginalP[nCurrPoly] * ((float)(iStepOfsX % (2 * MAX_TEXTURE_SIZE)) * 0.5f));
		else
			cVectorO += (m_OriginalP[nCurrPoly] * ((float)-(-iStepOfsX % (2 * MAX_TEXTURE_SIZE)) * 0.5f));

		if(iStepOfsY > 0)
			cVectorO += (m_OriginalQ[nCurrPoly] * ((float)(iStepOfsY % (2 * MAX_TEXTURE_SIZE)) * 0.5f));
		else
			cVectorO += (m_OriginalQ[nCurrPoly] * ((float)-(-iStepOfsY % (2 * MAX_TEXTURE_SIZE)) * 0.5f));		

		pTexture->SetTextureSpace(vNormal, cVectorO, cVectorP, cVectorQ);
	}

	m_pView->Invalidate(FALSE);

	return TRUE;
}

BOOL CRVTrackerTextureMove::OnEnd()
{
	return TRUE;
}

void CRVTrackerTextureMove::FlushTracker()
{
	delete m_pRotateTracker;
	m_pRotateTracker = new CRegionViewTracker(UIE_TEXTURE_MOVE_ROTATE, m_pView);
	SetupChildTracker(m_pRotateTracker);

	delete m_pScaleTracker;
	m_pScaleTracker = new CRegionViewTracker(UIE_TEXTURE_MOVE_SCALE, m_pView);
	SetupChildTracker(m_pScaleTracker);

	delete m_pStepTracker;
	m_pStepTracker = new CRegionViewTracker(UIE_TEXTURE_MOVE_STEP, m_pView);
	SetupChildTracker(m_pStepTracker);

	CRegionViewTracker::FlushTracker();
}

void CRVTrackerTextureMove::SetupChildTracker(CRegionViewTracker* pTracker)
{
	if(pTracker)
	{
		pTracker->SetAutoCapture(FALSE);
		pTracker->SetAutoHide(FALSE);
		pTracker->SetAutoCenter(FALSE);
	}
}

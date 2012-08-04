#if !defined(AFX_RENDERWND_H__07339643_2849_11D1_9462_0020AFF7CDC1__INCLUDED_)
#define AFX_RENDERWND_H__07339643_2849_11D1_9462_0020AFF7CDC1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RenderWnd.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CRenderWnd window
#include "ltaModel.h"

#include "bdefs.h"
#include "model.h"
#include "gl_modelrender.h"
#include "animtracker.h"



class CLocator
{
	public:
			
				CLocator();

		void	UpdateLocation();
		void	Center();

		float			m_nDistance;
		float			m_nXZAngle;
		float			m_nYAngle;
		CVector			m_Offset;
		CVector			m_Location;
		CVector			m_Orientation, m_Up, m_Right;
};


class CCamera
{
public:
	CCamera();
	void Update();
	void Reset();
	void LookAtPoint( const LTVector& lookAt );

	float m_fXZAngle;
	float m_fYAngle;
	LTVector m_Offset;

	LTVector m_Position;
	LTVector m_LookAt;
	LTVector m_Direction;
	LTVector m_Right;
	LTVector m_Up;
};


class CModelEditDlg;
class CSetFOVDlg;   // feild of view dialog

// ------------------------------------------------------------------------
// CRenderWnd
// the render window.
// ------------------------------------------------------------------------
class CRenderWnd : public CWnd
{
// Construction
public:
	CRenderWnd();

// Attributes
public:

// Operations
public:

	// Initialize the animation pointers.  These POINTERS are stored in RenderWnd.
	void			InitAnims(LTAnimTracker *pTrackers[NUM_ANIM_INFOS]);

	void			Draw();
	void			SetLOD(DWORD iLOD)		{m_DrawStruct.m_iLOD = iLOD;}
	void			SetLOD( float flod )    {m_DrawStruct.m_CurrentLODDist = flod ; }
	Model*			GetModel()				{return m_DrawStruct.GetModel();}

	BOOL			SetupTransformMaker(TransformMaker *pMaker);

	void			ResetLocator();
	void			ResetLights();
	GLM_CONTEXT		GetContext() {return m_hContext;}
	
	CLocator		m_LightLocators[MAX_GLM_LIGHTS];
	CCamera			m_Camera;

	void		    OpenFOVDlg();
	
	void			SetFOV( int val );
	void			ReleaseAllTextures();
	BOOL			SetTexture( TextureData *pTexture, DWORD nTextures );
	void			SetBackgroundColor( COLORREF  ms_color );

	void			AllocSelections( uint32 pieces,uint32 nodes ){
		m_SelectedPieces.SetSizeInit2(pieces, 0);
		m_SelectedNodes.SetSizeInit2(nodes, 0);
	}

	// Helps movement with small models.
	float			m_Scale;

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRenderWnd();

	int GetSelectedPiecesSize() { return m_SelectedPieces.GetSize() ; }
	int GetSelectedNodesSize() { return m_SelectedNodes.GetSize() ; }
	CMoByteArray	m_SelectedPieces;
	CMoByteArray	m_SelectedNodes;

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenderWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	GLM_CONTEXT		m_hContext;
	CSetFOVDlg     *m_pSetFOVDlg ;  // modeless dialog for dynamically setting the field of view on renderwnd.

public:

	AnimTracker		*m_pTrackers[NUM_ANIM_INFOS];
	
	CModelEditDlg	*m_pModelEditDlg;
	DrawStruct		m_DrawStruct;
	
	BOOL			m_bCameraFollow;


	BOOL			m_bTracking;
	CPoint			m_ptTracking;
	CPoint			m_ptTrackingScreen;

	// variables used for calculating internal radius by looping through all animation frames
	bool			m_bCalcRadius;
	float			m_fCurRadius;
	bool			m_bCalcAndDraw;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERWND_H__07339643_2849_11D1_9462_0020AFF7CDC1__INCLUDED_)
